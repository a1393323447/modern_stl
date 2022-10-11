//
// Created by Shiroan on 2022/9/25.
//

#ifndef MODERN_STL_TRACKINGALLOCATOR_H
#define MODERN_STL_TRACKINGALLOCATOR_H

#include <iostream>
#include <mstl/mstl.h>
#include <map>

namespace mstl {

    template <memory::concepts::Allocator A=memory::allocator::Allocator>
    class TrackingAllocator {
        A alloc;

        static usize memory_allocated_cumulative;
        static usize memory_deallocated_cumulative;
        static std::map<void*, usize> memory_map;

    public:
        TrackingAllocator() : alloc() {}
        explicit TrackingAllocator(const A& alloc) : alloc(alloc) {}
        explicit TrackingAllocator(A&& alloc) : alloc(alloc) {}

        void* allocate(const memory::Layout& layout, usize length) noexcept { // NOLINT(readability-convert-member-functions-to-static)
            memory_allocated_cumulative += layout.size * length;
            void* ptr = alloc.allocate(layout, length);
            memory_map[ptr] = layout.size * length;
            return ptr;
        }

        void deallocate(void* ptr, const memory::Layout& layout, usize length) noexcept{ // NOLINT(readability-convert-member-functions-to-static)
            if (ptr == nullptr) {
                return;
            }
            memory_deallocated_cumulative += layout.size * length;
            auto iter = memory_map.find(ptr);
            if (iter != memory_map.end()) {
                memory_map.erase(iter);
            }
            alloc.deallocate(ptr, layout, length);
        }

        template<class T>
        T* allocate(usize len) {
            return alloc.template allocate<T>(len);
        }

        template<class T>
        void deallocate(T* ptr, usize len) {
            alloc.template deallocate(ptr, len);
        }

        static usize get_memory_allocated_cumulative() {
            return memory_allocated_cumulative;
        }

        static usize get_memory_deallocated_cumulative() {
            return memory_deallocated_cumulative;
        }

        static usize get_beholding_memory() {
            return memory_allocated_cumulative - memory_deallocated_cumulative;
        }

        static const std::map<void *, usize> &getMemoryMap() {
            return memory_map;
        }

        static void dump() {
            std::cout << "Allocated Memory:\t" << memory_allocated_cumulative << std::endl
                      << "Deallocated Memory:\t" << memory_deallocated_cumulative << std::endl
                      << "Beholding Memory:\t" << get_beholding_memory() << std::endl;
            if (!memory_map.empty()) {
                std::cout << "\n" << std::endl;
                for (auto &p : memory_map) {
                    std::cout << p.first << " : " << p.second << std::endl;
                }
            }
        }
    };

    template<memory::concepts::Allocator A>
    usize TrackingAllocator<A>::memory_allocated_cumulative = 0;

    template<memory::concepts::Allocator A>
    usize TrackingAllocator<A>::memory_deallocated_cumulative = 0;

    template<memory::concepts::Allocator A>
    std::map<void*, usize> TrackingAllocator<A>::memory_map = {};
} // mstl

#endif //MODERN_STL_TRACKINGALLOCATOR_H
