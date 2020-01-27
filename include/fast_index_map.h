// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <limits>
#include <memory>
#include "pool.h"

namespace Memory
{
    template<class ElementType>
    class FastIndexMap
    {
        const static uint32_t PAGE_SIZE = static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) + 1;

        struct ElementStorage
        {
            bool allocated = false;
            std::uint32_t next_available_id = 0;
            std::array<unsigned char, sizeof(ElementType)> storage;

            ElementStorage()
            {
                storage.fill(0);
            }
        };
        struct Page
        {
            std::array<ElementStorage, PAGE_SIZE> elements;
        };

        std::array<Page*, PAGE_SIZE> pages;
        std::uint32_t next_available_id = 0;
        std::uint32_t max_allocated_id = 0;

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
        bool pageAllocated(std::uint32_t index) const
        {
            std::uint32_t page_index = index / PAGE_SIZE;
            return pages[page_index] ? true : false;
        }
    public:
        FastIndexMap()
        {
            pages.fill(nullptr);
        }
        ~FastIndexMap()
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
        ElementType* allocate(std::uint32_t index)
        {
            max_allocated_id = std::max(max_allocated_id, index);
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
        void deallocate(std::uint32_t index)
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
        std::uint32_t allocateIndex()
        {
            if (pageAllocated(next_available_id))
            {
                ElementStorage* info = element(next_available_id);
                assert(!find(next_available_id));
                std::uint32_t result_id = next_available_id;
                if (info->next_available_id)
                {
                    next_available_id = info->next_available_id;
                }
                else
                {
                    next_available_id++;
                }
                return result_id;
            }
            else
            {
                return next_available_id++;
            }
        }
        void deallocateIndex(std::uint32_t id)
        {
            assert(pageAllocated(id));
            ElementStorage* info = element(id);
            if (id < next_available_id)
            {
                info->next_available_id = next_available_id;
                next_available_id = id;
            }
            else
            {
                assert(pageAllocated(next_available_id));
                ElementStorage* next_available_id_info = element(next_available_id);
                info->next_available_id = next_available_id_info->next_available_id;
                next_available_id_info->next_available_id = id;
            }
        }
        std::uint32_t getMaxAllocatedIndex() const
        {
            return max_allocated_id;
        }
    };
}
