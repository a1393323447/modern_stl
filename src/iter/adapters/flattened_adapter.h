//
// Created by 朕与将军解战袍 on 2022/9/17.
//

#ifndef __MODERN_STL_FLATTENED_ADAPTER_H__
#define __MODERN_STL_FLATTENED_ADAPTER_H__

#include <iter/adapters/adapter_concepts.h>
#include <intrinsics.h>

namespace mstl::iter::adapter {
    namespace _private {
        template<typename P>
        concept StackPipe = requires {
            typename P::Lambda;
            typename P::InputItem;
            typename P::ThisPipeReturnType; // must be an Option
            requires requires(
                    P pipe,
                    typename P::InputItem item
            ) {
                { pipe(item) } -> std::same_as<typename P::ThisPipeReturnType>;
            };
        };

        /// StackPipe is a series of Pipe which wrap a lambda function, and is a functor
        /// Input -> Pipe[lambda 1] -> Pipe[lambda 2] -> Pipe[lambda 3] -> ... -> Output
        ///                                  |
        ///                                  v
        ///                    May return early(see ControlFlow)

        // TODO: Implement move for Pipe

        template<typename... Args>
        class Pipe {
            template<typename... T>
            struct Protected {
                static const bool value = false;
            };
            static_assert(
                Protected<Args...>::value,
                "Unexpected Pipe Args ! This is a bug\n"
            );
        };

        template<adapter::Adapter A, StackPipe P>
        class Pipe<A, P> {
        public:
            using InputItem = typename A::LambdaTakeItemType;
            using Lambda    = typename A::Lambda;
            using NextPipeReturnType = typename P::ThisPipeReturnType; // is must be an Option
            using ThisPipeReturnType = NextPipeReturnType;

            Pipe(Lambda l, P p): lambda(std::move(l)), pipe(p) { }

            MSTL_INLINE
            ThisPipeReturnType operator()(InputItem item) {
                using ProcessResult = adapter::ProcessResult<typename A::ProcessResultType>;
                ProcessResult this_result = A::process(
                    std::move(item),
                    lambda
                );

                if constexpr (A::thisInfluenceType == adapter::InfluenceType::Influence) {
                    using adapter::ControlFlow;
                    switch (this_result.controlFlow) {
                    case ControlFlow::Break:
                        return NextPipeReturnType::none();
                    case ControlFlow::Continue:
                        return pipe(std::move(this_result.result)); // next pipe
                    }
                } else {
                    return pipe(std::move(this_result.result));
                }

                // unreachable
                return NextPipeReturnType::none();
            }

        private:
            Lambda lambda;
            P      pipe;
        };

        template<adapter::Adapter A>
        class Pipe<A> {
        public:
            using InputItem = typename A::LambdaTakeItemType;
            using Lambda    = typename A::Lambda;
            using ThisPipeReturnType = Option<typename A::ProcessResultType>;

            Pipe(Lambda l): lambda(std::move(l)) { }

            MSTL_INLINE
            ThisPipeReturnType operator()(InputItem item) {
                using ProcessResult = adapter::ProcessResult<typename A::ProcessResultType>;
                ProcessResult this_result = A::process(
                    std::move(item),
                    lambda
                );
                if constexpr (A::thisInfluenceType == adapter::InfluenceType::Influence) {
                    using adapter::ControlFlow;
                    switch (this_result.controlFlow) {
                    case ControlFlow::Break:
                        return Option<typename A::ProcessResultType>::none();
                    case ControlFlow::Continue:
                        return Option<typename A::ProcessResultType>::some(
                            std::move(this_result.result)
                        );
                    }
                } else {
                    return Option<typename A::ProcessResultType>::some(
                        std::move(this_result.result)
                    );
                }
                // unreachable
                return Option<typename A::ProcessResultType>::none();
            }

        private:
            Lambda lambda;
        };
    }

    template<Iterator Iter, _private::StackPipe Pipe>
    requires (!adapter::Adapter<Iter>) // Iter should not be an Adapter
    class FlattenedAdapter {
    public:
        // TODO: implement Iter, IntoIter, Adapter
        using Item = typename Pipe::ThisPipeReturnType::WrapType;
        using Lambda = Pipe; // FIXME: This should be a wrapper of pipe
        using LambdaTakeItemType = typename Pipe::InputItem; // FIXME
        using ProcessResultType = typename Pipe::ThisPipeReturnType; // FIXME

        FlattenedAdapter(Iter it, Pipe p):
        iter(std::move(it)), pipe(std::move(p))
        { }

        // TODO: faster for_each

        MSTL_INLINE
        typename Pipe::ThisPipeReturnType
        next() {
            auto next = iter.next();
            if (next.is_some()) {
                auto control = pipe(next.unwrap_uncheck());
                while (control.is_none()) {
                    next = iter.next();
                    if (next.is_none()) {
                        return Pipe::ThisPipeReturnType::none();
                    }
                    control = pipe(next.unwrap_uncheck());
                }
                return control;
            } else {
                return Pipe::ThisPipeReturnType::none();
            }
        }

    private:
        Iter iter;
        Pipe pipe;
    };

    /// A helper function to flatten stacked adapters

    template<Iterator A, _private::StackPipe P>
    decltype(auto) flatten(A adapter, P pipe) {
        using UnderIt = typename A::UnderlyingIter;
        using Lambda  = typename A::Lambda;
        using CurrentPipe = _private::Pipe<A, P>;

        UnderIt iter = adapter.get_underlying_iter();
        Lambda lambda = adapter.get_lambda();
        CurrentPipe current_pipe{
                std::move(lambda),
                std::move(pipe)
        };

        if constexpr (Adapter<UnderIt>) {
            return flatten(
                std::move(iter),
                std::move(current_pipe)
            );
        } else {
            return FlattenedAdapter<UnderIt, CurrentPipe>{
                    std::move(iter),
                    std::move(current_pipe)
            };
        }
    }

    template<Adapter A>
    decltype(auto) flatten(A adapter) {
        using UnderIt = typename A::UnderlyingIter;
        using Lambda  = typename A::Lambda;
        using CurrentPipe = _private::Pipe<A>;

        UnderIt iter = adapter.get_underlying_iter();
        Lambda lambda = adapter.get_lambda();
        CurrentPipe current_pipe{ std::move(lambda) };

        if constexpr (Adapter<UnderIt>) {
            return flatten(
                std::move(iter),
                std::move(current_pipe)
            );
        } else {
            return FlattenedAdapter<UnderIt, CurrentPipe>{
                std::move(iter),
                std::move(current_pipe)
            };
        }
    }
}

#endif //__MODERN_STL_FLATTENED_ADAPTER_H__
