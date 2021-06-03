// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <limits>
#include <memory>
#include <atomic>
#include "pool.h"

namespace Memory
{
    const std::uint32_t INVALID_INDEX = static_cast<std::uint32_t>(std::numeric_limits<std::uint32_t>::max());

    template<class ElementType>
    class FastIndex
    {
        const static std::uint32_t PAGE_SIZE = static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) + 1;

        struct ElementStorage
        {
            std::atomic<bool> allocated = false;
            std::uint32_t next_available_index = 0;
            std::atomic<std::uint32_t> chain_previous_index = INVALID_INDEX;
            std::atomic<std::uint32_t> chain_next_index = INVALID_INDEX;

            std::array<unsigned char, sizeof(ElementType)> storage;
        };
        struct Page
        {
            std::array<ElementStorage, PAGE_SIZE> elements;
        };

        std::array<std::atomic<Page*>, PAGE_SIZE> pages;
        std::uint32_t max_allocated_index = 0;
        std::uint32_t next_available_index = 0;
        std::atomic<std::uint32_t> chain_head_index = INVALID_INDEX;

    protected:
        ElementStorage* element(std::uint32_t index) const
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            Page* page = pages[page_index];
            if (page)
            {
                std::uint32_t index_in_page = index % PAGE_SIZE;
                return &page->elements[index_in_page];
            }
            else
            {
                return nullptr;
            }
        }
        ElementType* fast_find(std::uint32_t index) const
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            Page* page = pages[page_index];
            if (page)
            {
                std::uint32_t index_in_page = index % PAGE_SIZE;
                ElementStorage& element = page->elements[index_in_page];
                return reinterpret_cast<ElementType*>(element.storage.data());
            }
            else
            {
                return nullptr;
            }
        }
        ElementType* find(std::uint32_t index) const
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            Page* page = pages[page_index];
            if (page)
            {
                std::uint32_t index_in_page = index % PAGE_SIZE;
                ElementStorage& element = page->elements[index_in_page];
                if (element.allocated)
                {
                    return reinterpret_cast<ElementType*>(element.storage.data());
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }
        bool pageAllocated(std::uint32_t index) const
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            return pages[page_index] ? true : false;
        }
        ElementType* allocateBlock(std::uint32_t index)
        {
            max_allocated_index = std::max(max_allocated_index, index);
            std::uint32_t page_index = index / PAGE_SIZE;
            std::uint32_t index_in_page = index % PAGE_SIZE;
            Page* page = pages[page_index];
            if (page)
            {
                ElementStorage& element = page->elements[index_in_page];
                element.allocated = true;
                return reinterpret_cast<ElementType*>(element.storage.data());
            }
            else
            {
                Page* new_page = new Page;
                pages[page_index] = new_page;
                ElementStorage& new_element = new_page->elements[index_in_page];
                new_element.allocated = true;
                return reinterpret_cast<ElementType*>(new_element.storage.data());
            }
        }
        void deallocateBlock(std::uint32_t index)
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            Page* page = pages[page_index];
            if (page)
            {
                std::uint32_t index_in_page = index % PAGE_SIZE;
                ElementStorage& element = page->elements[index_in_page];
                element.allocated = false;
            }
        }
        std::uint32_t getMaxAllocatedIndex() const
        {
            return max_allocated_index;
        }
        std::uint32_t allocateIndex()
        {
            if (pageAllocated(next_available_index))
            {
                ElementStorage* info = element(next_available_index);
                std::uint32_t result_index = next_available_index;
                if (info->next_available_index)
                {
                    next_available_index = info->next_available_index;
                }
                else
                {
                    next_available_index++;
                }
                return result_index;
            }
            else
            {
                return next_available_index++;
            }
        }
        void deallocateIndex(std::uint32_t index)
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            if (index < next_available_index)
            {
                info->next_available_index = next_available_index;
                next_available_index = index;
            }
            else if (index > next_available_index)
            {
                assert(pageAllocated(next_available_index));
                ElementStorage* next_available_info = element(next_available_index);
                info->next_available_index = next_available_info->next_available_index;
                next_available_info->next_available_index = index;
            }
        }
        void chainPushFront(std::uint32_t index)
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            info->chain_next_index = chain_head_index;
            info->chain_previous_index = INVALID_INDEX;
            if (chain_head_index != INVALID_INDEX)
            {
                ElementStorage* head_info = element(chain_head_index);
                head_info->chain_previous_index = index;
            }
            chain_head_index = index;
        }
        void chainRemove(std::uint32_t index)
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            std::uint32_t previous_index = info->chain_previous_index;
            std::uint32_t next_index = info->chain_next_index;
            if (previous_index != INVALID_INDEX)
            {
                ElementStorage* previous = element(previous_index);
                previous->chain_next_index = next_index;
            }
            else
            {
                chain_head_index = next_index;
            }
            if (next_index != INVALID_INDEX)
            {
                ElementStorage* next = element(next_index);
                next->chain_previous_index = previous_index;
            }
        }
        std::uint32_t getChainHeadIndex() const
        {
            return chain_head_index;
        }
        std::uint32_t getChainNextIndex(std::uint32_t index) const
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            return info->chain_next_index;
        }
        FastIndex()
        {
        }
        ~FastIndex()
        {
            for (size_t i = 0; i < PAGE_SIZE; ++i)
            {
                Page* page = pages[i];
                if (page)
                {
                    delete page;
                }
            }
        }
    };

    template<class ElementType>
    class FastIndexMap : protected FastIndex<ElementType>
    {
        typedef FastIndex<ElementType> Base;

    public:
        ElementType* allocateBlock(std::uint32_t index)
        {
            return Base::allocateBlock(index);
        }
        void deallocateBlock(std::uint32_t index)
        {
            Base::deallocateBlock(index);
        }
        ElementType* find(std::uint32_t index) const
        {
            return Base::find(index);
        }
    };

    template<class ElementType>
    class FastIndexRegistry : protected FastIndex<ElementType>
    {
        typedef FastIndex<ElementType> Base;

    public:
        ElementType* allocate(std::uint32_t& result_index)
        {
            result_index = allocateIndex();
            return allocateBlock(result_index);
        }
        void deallocate(std::uint32_t index)
        {
            deallocateBlock(index);
            deallocateIndex(index);
        }
        ElementType* find(std::uint32_t index) const
        {
            return Base::find(index);
        }
    };

    template<class ElementType>
    class FastIndexChain : protected FastIndex<ElementType>
    {
        typedef FastIndex<ElementType> Base;

    public:
        ElementType* chainPushFront(std::uint32_t& result_index)
        {
            result_index = allocateIndex();
            Base::chainPushFront(result_index);
            return allocateBlock(result_index);
        }
        void chainRemove(std::uint32_t index)
        {
            deallocateBlock(index);
            Base::chainRemove(index);
            deallocateIndex(index);
        }
        std::uint32_t getChainHeadIndex() const
        {
            return Base::getChainHeadIndex();
        }
        std::uint32_t getChainNextIndex(std::uint32_t index) const
        {
            return Base::getChainNextIndex(index);
        }
        ElementType* get(std::uint32_t index) const
        {
            return Base::fast_find(index);
        }
        ElementType* find(std::uint32_t index) const
        {
            return Base::find(index);
        }
    };
}
