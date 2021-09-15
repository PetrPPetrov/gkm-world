// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <atomic>
#include <array>
#include "gkm_world/gkm_world.h"

namespace Memory {
    namespace Details {
        template<class ElementType>
        struct ElementStorage {
            std::atomic<bool> allocated = false;
            IndexType next_available_index = 0;
            std::atomic<IndexType> chain_previous_index = INVALID_INDEX;
            std::atomic<IndexType> chain_next_index = INVALID_INDEX;

            std::array<std::uint8_t, sizeof(ElementType)> storage;
        };

        template<class ElementType, std::uint8_t PageBitCount, std::uint8_t NestedPageBitCount, std::uint8_t PageBitOffset>
        class FastIndexPage;

        template<class ElementType, std::uint8_t PageBitCount, std::uint8_t NestedPageBitCount>
        class FastIndexPage<ElementType, PageBitCount, NestedPageBitCount, 0> {
            constexpr static std::uint8_t PAGE_BIT_COUNT = PageBitCount;
            constexpr static IndexType PAGE_SIZE = (1 << PAGE_BIT_COUNT);

            std::array<ElementStorage<ElementType>, PAGE_SIZE> elements;

            constexpr static IndexType getMask() {
                return ((1 << PAGE_BIT_COUNT) - 1);
            }
            static IndexType getElementIndex(IndexType index) {
                return index & getMask();
            }

        public:
            typedef ElementStorage<ElementType> ElementTypeStorage;

            ElementTypeStorage* element(IndexType index) const {
                const IndexType element_index = getElementIndex(index);
                return const_cast<ElementTypeStorage*>(&elements[element_index]);
            }
            ElementType* fast_find(IndexType index) const {
                const IndexType element_index = getElementIndex(index);
                const ElementTypeStorage& element = elements[element_index];
                return const_cast<ElementType*>(reinterpret_cast<const ElementType*>(element.storage.data()));
            }
            ElementType* find(IndexType index) const {
                const IndexType element_index = getElementIndex(index);
                const ElementTypeStorage& element = elements[element_index];
                if (element.allocated) {
                    return const_cast<ElementType*>(reinterpret_cast<const ElementType*>(element.storage.data()));
                }
                return nullptr;
            }
            bool pageAllocated(IndexType index) const {
                return true;
            }
            ElementType* allocateBlock(IndexType index) {
                const IndexType element_index = getElementIndex(index);
                ElementTypeStorage& element = elements[element_index];
                element.allocated = true;
                return reinterpret_cast<ElementType*>(element.storage.data());
            }
            void deallocateBlock(IndexType index) {
                const IndexType element_index = getElementIndex(index);
                ElementTypeStorage& element = elements[element_index];
                element.allocated = false;
            }
        };

        template<class ElementType, std::uint8_t PageBitCount, std::uint8_t NestedPageBitCount, std::uint8_t PageBitOffset>
        class FastIndexPage {
            constexpr static std::uint8_t ALL_BIT_COUNT = sizeof(IndexType) * 8;
            constexpr static std::uint8_t PAGE_BIT_COUNT = PageBitCount;
            constexpr static IndexType PAGE_SIZE = (1 << PAGE_BIT_COUNT);
            constexpr static std::uint8_t NESTED_PAGE_BIT_COUNT = NestedPageBitCount;
            constexpr static std::uint8_t PAGE_BIT_OFFSET = PageBitOffset;

            static_assert(PAGE_BIT_OFFSET >= NESTED_PAGE_BIT_COUNT);
            typedef FastIndexPage<ElementType, NESTED_PAGE_BIT_COUNT, NESTED_PAGE_BIT_COUNT, PAGE_BIT_OFFSET - NESTED_PAGE_BIT_COUNT> PageType;
            std::array<std::atomic<PageType*>, PAGE_SIZE> pages;

            constexpr static IndexType getMask() {
                if constexpr (PAGE_BIT_COUNT + PAGE_BIT_OFFSET == ALL_BIT_COUNT) {
                    return std::numeric_limits<IndexType>::max();
                } else {
                    return ((1 << (PAGE_BIT_COUNT + PAGE_BIT_OFFSET)) - 1);
                }
            }
            static IndexType getPageIndex(IndexType index) {
                return (index & getMask()) >> PAGE_BIT_OFFSET;
            }

        public:
            typedef ElementStorage<ElementType> ElementTypeStorage;

            ~FastIndexPage() {
                for (IndexType page_index = 0; page_index < PAGE_SIZE; ++page_index) {
                    PageType* page = pages[page_index];
                    pages[page_index] = nullptr;
                    if (page) {
                        delete page;
                    }
                }
            }

            ElementTypeStorage* element(IndexType index) const {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page) {
                    return page->element(index);
                }
                return nullptr;
            }
            ElementType* fast_find(IndexType index) const {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page) {
                    return page->fast_find(index);
                }
                return nullptr;
            }
            ElementType* find(IndexType index) const {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page) {
                    return page->find(index);
                }
                return nullptr;
            }
            bool pageAllocated(IndexType index) const {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page == nullptr) {
                    return false;
                }
                return page->pageAllocated(index);
            }
            ElementType* allocateBlock(IndexType index) {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page == nullptr) {
                    page = new PageType;
                    pages[page_index] = page;
                }
                return page->allocateBlock(index);
            }
            void deallocateBlock(IndexType index) {
                const IndexType page_index = getPageIndex(index);
                PageType* page = pages[page_index];
                if (page) {
                    page->deallocateBlock(index);
                }
            }
        };
    }

    template<class ElementType, std::uint8_t PageBitCount>
    class FastIndex {
        constexpr static std::uint8_t ALL_BIT_COUNT = sizeof(IndexType) * 8;
        constexpr static std::uint8_t NESTED_PAGE_BIT_COUNT = PageBitCount;
        constexpr static std::uint8_t REMAINDER = ALL_BIT_COUNT % PageBitCount;
        constexpr static std::uint8_t PAGE_BIT_COUNT = (REMAINDER == 0) ? PageBitCount : REMAINDER;

        static_assert(ALL_BIT_COUNT >= PAGE_BIT_COUNT);

        Details::FastIndexPage<ElementType, PAGE_BIT_COUNT, NESTED_PAGE_BIT_COUNT, ALL_BIT_COUNT - PAGE_BIT_COUNT> storage;
        IndexType max_allocated_index = 0;
        IndexType next_available_index = 0;
        std::atomic<IndexType> chain_head_index = INVALID_INDEX;
        std::atomic<IndexType> chain_tail_index = INVALID_INDEX;

    protected:
        typedef Details::ElementStorage<ElementType> ElementTypeStorage;

        ElementTypeStorage* element(IndexType index) const {
            return storage.element(index);
        }
        ElementType* fast_find(IndexType index) const {
            return storage.fast_find(index);
        }
        ElementType* find(IndexType index) const {
            return storage.find(index);
        }
        bool pageAllocated(IndexType index) const {
            return storage.pageAllocated(index);
        }
        ElementType* allocateBlock(IndexType index) {
            max_allocated_index = std::max(max_allocated_index, index);
            return storage.allocateBlock(index);
        }
        void deallocateBlock(IndexType index) {
            storage.deallocateBlock(index);
        }
        IndexType getMaxAllocatedIndex() const {
            return max_allocated_index;
        }
        IndexType allocateIndex() {
            if (pageAllocated(next_available_index)) {
                ElementTypeStorage* info = element(next_available_index);
                IndexType result_index = next_available_index;
                if (info->next_available_index) {
                    next_available_index = info->next_available_index;
                } else {
                    next_available_index++;
                }
                return result_index;
            } else {
                return next_available_index++;
            }
        }
        void deallocateIndex(IndexType index) {
            assert(pageAllocated(index));
            ElementTypeStorage* info = element(index);
            if (index < next_available_index) {
                info->next_available_index = next_available_index;
                next_available_index = index;
            } else if (index > next_available_index) {
                assert(pageAllocated(next_available_index));
                ElementTypeStorage* next_available_info = element(next_available_index);
                info->next_available_index = next_available_info->next_available_index;
                next_available_info->next_available_index = index;
            }
        }
        void chainPushFront(IndexType index) {
            assert(pageAllocated(index));
            ElementTypeStorage* info = element(index);
            info->chain_next_index = chain_head_index.load();
            info->chain_previous_index = INVALID_INDEX;
            if (chain_head_index != INVALID_INDEX) {
                ElementTypeStorage* head_info = element(chain_head_index);
                head_info->chain_previous_index = index;
            }
            chain_head_index = index;
            if (chain_tail_index == INVALID_INDEX) {
                chain_tail_index = index;
            }
        }
        void chainPushBack(IndexType index) {
            assert(pageAllocated(index));
            ElementTypeStorage* info = element(index);
            info->chain_next_index = INVALID_INDEX;
            info->chain_previous_index = chain_tail_index.load();
            if (chain_tail_index != INVALID_INDEX) {
                ElementTypeStorage* tail_info = element(chain_tail_index);
                tail_info->chain_next_index = index;
            }
            chain_tail_index = index;
            if (chain_head_index == INVALID_INDEX) {
                chain_head_index = index;
            }
        }
        void chainRemove(IndexType index) {
            assert(pageAllocated(index));
            ElementTypeStorage* info = element(index);
            IndexType previous_index = info->chain_previous_index;
            IndexType next_index = info->chain_next_index;
            if (previous_index != INVALID_INDEX) {
                ElementTypeStorage* previous = element(previous_index);
                previous->chain_next_index = next_index;
            } else {
                chain_head_index = next_index;
            }
            if (next_index != INVALID_INDEX) {
                ElementTypeStorage* next = element(next_index);
                next->chain_previous_index = previous_index;
            } else {
                chain_tail_index = previous_index;
            }
            info->chain_previous_index = INVALID_INDEX;
            info->chain_next_index = INVALID_INDEX;
        }
        IndexType getChainHeadIndex() const {
            return chain_head_index;
        }
        IndexType getChainNextIndex(IndexType index) const {
            assert(pageAllocated(index));
            ElementTypeStorage* info = element(index);
            return info->chain_next_index;
        }
    };

    template<class ElementType, std::uint8_t PageBitCount>
    class FastIndexMap : protected FastIndex<ElementType, PageBitCount> {
        typedef FastIndex<ElementType, PageBitCount> Base;

    public:
        ElementType* allocateBlock(IndexType index) {
            return Base::allocateBlock(index);
        }
        void deallocateBlock(IndexType index) {
            Base::deallocateBlock(index);
        }
        ElementType* find(IndexType index) const {
            return Base::find(index);
        }
    };

    template<class ElementType, std::uint8_t PageBitCount>
    class FastIndexChain : protected FastIndex<ElementType, PageBitCount> {
        typedef FastIndex<ElementType, PageBitCount> Base;

    public:
        ElementType* chainPushFront(IndexType index) {
            auto result = allocateBlock(index);
            Base::chainPushFront(index);
            return result;
        }
        ElementType* chainPushBack(IndexType index) {
            auto result = allocateBlock(index);
            Base::chainPushBack(index);
            return result;
        }
        void chainRemove(IndexType index) {
            Base::chainRemove(index);
            deallocateBlock(index);
        }
        IndexType getChainHeadIndex() const {
            return Base::getChainHeadIndex();
        }
        IndexType getChainNextIndex(IndexType index) const {
            return Base::getChainNextIndex(index);
        }
        ElementType* get(IndexType index) const {
            return Base::fast_find(index);
        }
        ElementType* find(IndexType index) const {
            return Base::find(index);
        }
    };

    template<class ElementType, std::uint8_t PageBitCount>
    class FastIndexRegistry : protected FastIndex<ElementType, PageBitCount> {
        typedef FastIndex<ElementType, PageBitCount> Base;

    public:
        ElementType* allocate(IndexType& result_index) {
            result_index = allocateIndex();
            return allocateBlock(result_index);
        }
        void deallocate(IndexType index) {
            deallocateBlock(index);
            deallocateIndex(index);
        }
        ElementType* find(IndexType index) const {
            return Base::find(index);
        }
    };

    template<class ElementType, std::uint8_t PageBitCount>
    class FastIndexRegistryChain : protected FastIndex<ElementType, PageBitCount> {
        typedef FastIndex<ElementType, PageBitCount> Base;

    public:
        ElementType* chainPushFront(IndexType& result_index) {
            result_index = allocateIndex();
            auto result = allocateBlock(result_index);
            Base::chainPushFront(result_index);
            return result;
        }
        ElementType* chainPushBack(IndexType& result_index) {
            result_index = allocateIndex();
            auto result = allocateBlock(result_index);
            Base::chainPushBack(result_index);
            return result;
        }
        void chainRemove(IndexType index) {
            Base::chainRemove(index);
            deallocateBlock(index);
            deallocateIndex(index);
        }
        IndexType getChainHeadIndex() const {
            return Base::getChainHeadIndex();
        }
        IndexType getChainNextIndex(IndexType index) const {
            return Base::getChainNextIndex(index);
        }
        ElementType* get(IndexType index) const {
            return Base::fast_find(index);
        }
        ElementType* find(IndexType index) const {
            return Base::find(index);
        }
    };
}
