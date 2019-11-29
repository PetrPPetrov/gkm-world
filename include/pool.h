// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cassert>
#include <memory>
#include <vector>
#include <array>
#include <boost/core/noncopyable.hpp>
#include "log.h"

namespace Memory
{
    template<class ElementType, size_t MaxElementCount, size_t ElementSize = sizeof(ElementType)>
    class Pool : private boost::noncopyable
    {
        static_assert(ElementSize > 0, "element size is zero");
        static_assert(MaxElementCount > 0, "max element count is zero");

        const static size_t ELEMENT_SIZE = ElementSize;
        const static size_t MAX_CAPACITY = MaxElementCount;
        struct ElementStorage
        {
            bool allocated = false;
            ElementStorage* next_free = nullptr;
            std::array<unsigned char, ELEMENT_SIZE> buffer;
        };
        typedef std::vector<ElementStorage> storage_t;
        storage_t storage;
        ElementStorage* free_element = nullptr;

    public:
        Pool()
        {
            storage.resize(MAX_CAPACITY);
            for (size_t i = 0; i < MAX_CAPACITY - 1; ++i)
            {
                storage[i].next_free = &storage[i + 1];
            }
            free_element = storage.data();
        }
        ElementType* allocate()
        {
            if (free_element)
            {
                ElementStorage* result = free_element;
                result->allocated = true;
                free_element = result->next_free;
                return reinterpret_cast<ElementType*>(result->buffer.data());
            }
            else
            {
                LOG_WARNING << "not enough buffer, allocation by malloc" << std::endl;
                ElementType* result = reinterpret_cast<ElementType*>(malloc(ELEMENT_SIZE));
                return result;
            }
        }
        void deallocate(ElementType* element)
        {
            const size_t base = reinterpret_cast<size_t>(storage.data());
            const size_t element_address = reinterpret_cast<size_t>(element);
            const size_t index = (element_address - base) / sizeof(ElementStorage);
            if (index < MAX_CAPACITY)
            {
                if (storage[index].allocated)
                {
                    storage[index].allocated = false;
                    storage[index].next_free = free_element;
                    free_element = &storage[index];
                }
            }
            else
            {
                free(element);
            }
        }
        void deallocate(size_t index)
        {
            assert(index < MAX_CAPACITY);
            if (storage[index].allocated)
            {
                storage[index].allocated = false;
                storage[index].next_free = free_element;
                free_element = &storage[index];
            }
        }
    };
}
