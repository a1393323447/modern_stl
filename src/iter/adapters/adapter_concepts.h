//
// Created by 朕与将军解战袍 on 2022/9/17.
//

#ifndef __MODERN_STL_ADAPTER_CONCEPTS_H__
#define __MODERN_STL_ADAPTER_CONCEPTS_H__

#include <iter/iter_concepts.h>

namespace mstl::iter::adapter {

    // TODO: move to a relevant place
    enum class ControlFlow {
        Continue,
        Break,
    };

    enum class InfluenceType {
        NoEffect,
        Influence
    };

    template<typename R>
    struct ProcessResult {
        ControlFlow controlFlow;
        R result;
    };

    template<typename A>
    concept Adapter = requires {
        /// Adapter is a iterator
        requires Iterator<A>;
        /// Adapter should have a underlying iterator
        typename A::UnderlyingIter;
        requires Iterator<typename A::UnderlyingIter>;
        /// And it should have a getter
        requires requires(A adapter) {
            /// [NOTE]: It should have a move semantics and
            /// this function should mark with `constexpr`
            { adapter.get_underlying_iter() } -> std::same_as<typename A::UnderlyingIter>;
        };
        /// Adapter should have a lambda function member
        typename A::Lambda;
        /// The lambda function should take a item of the underlying iterator by reference or by value
        /// So need to indicate it
        typename A::LambdaTakeItemType;
        requires requires(A adapter) {
            /// And the lambda function member should have a getter
            /// [NOTE]: It should have a move semantics and
            /// this function should mark with `constexpr`
            { adapter.get_lambda() } -> std::same_as<typename A::Lambda>;
        };
        /// Adapter should have a static process function
        /// Process function should take the lambda member
        /// and the a element in the underlying iterator as its args.
        /// And the invoke result of process function should be a ProcessResult(ControlFlow, Return).
        /// Adapter should knows about the raw process result type (Return) -----------------/
        typename A::ProcessResultType;
        requires requires(
            typename A::UnderlyingIter::Item item,
            const typename A::Lambda& lambda // To avoid unnecessary copy, it should take a const reference
        ) {
            { A::process(item, lambda) } -> std::same_as<
                ProcessResult<typename A::ProcessResultType>
            >;
        };
        /// Depending on different adapters, the lambda return value may influence the control flow.
        /// So the adapter should have a static const member to indicate that.
        requires std::same_as<decltype(A::thisInfluenceType), const InfluenceType>;
    };
}

#endif //__MODERN_STL_ADAPTER_CONCEPTS_H__
