// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <atomic>
#include <array>
#include "gkm_world/gkm_world.h"

namespace Memory
{
    template<class ElementType>
    class FastIndex16BitPage
    {
        constexpr std::uint8_t PAGE_BIT_COUNT = 16;
        constexpr IndexType PAGE_SIZE = (1 << PAGE_BIT_COUNT);

        static inline IndexType getPageIndex(IndexType index)
        {
            return index >> PAGE_BIT_COUNT;
        }
        static inline IndexType getElementIndex(IndexType index)
        {
            return index & ((1 << PAGE_BIT_COUNT) - 1);
        }

        struct ElementStorage
        {
            std::atomic<bool> allocated = false;
            IndexType next_available_index = 0;
            std::atomic<IndexType> chain_previous_index = INVALID_INDEX;
            std::atomic<IndexType> chain_next_index = INVALID_INDEX;

            std::array<std::uint8_t, sizeof(ElementType)> storage;
        };
        struct Page
        {
            std::array<ElementStorage, PAGE_SIZE> elements;
        };

        std::array<std::atomic<Page*>, PAGE_SIZE> pages;
        IndexType max_allocated_index = 0;
        IndexType next_available_index = 0;
        std::atomic<IndexType> chain_head_index = INVALID_INDEX;

    protected:
        ElementStorage* element(IndexType index) const
        {
            const IndexType page_index = getPageIndex(index);
            Page* page = pages[page_index];
            if (page)
            {
                const IndexType element_index = getElementIndex(index);
                return &page->elements[element_index];
            }
            return nullptr;
        }
        ElementType* fast_find(IndexType index) const
        {
            const IndexType page_index = getPageIndex(index);
            Page* page = pages[page_index];
            if (page)
            {
                const IndexType element_index = getElementIndex(index);
                ElementStorage& element = page->elements[element_index];
                return reinterpret_cast<ElementType*>(element.storage.data());
            }
            return nullptr;
        }
        ElementType* find(IndexType index) const
        {
            const IndexType page_index = getPageIndex(index);
            Page* page = pages[page_index];
            if (page)
            {
                const IndexType element_index = getElementIndex(index);
                ElementStorage& element = page->elements[element_index];
                if (element.allocated)
                {
                    return reinterpret_cast<ElementType*>(element.storage.data());
                }
            }
            return nullptr;
        }
        bool pageAllocated(IndexType index) const
        {
            const IndexType page_index = getPageIndex(index);
            return pages[page_index] ? true : false;
        }
        ElementType* allocateBlock(IndexType index)
        {
            max_allocated_index = std::max(max_allocated_index, index);
            const IndexType page_index = getPageIndex(index);
            Page* page = pages[page_index];
            if (page == nullptr)
            {
                page = new Page;
                pages[page_index] = page;
            }
            const IndexType element_index = getOffsetIndex(index);
            ElementStorage& element = page->elements[element_index];
            element.allocated = true;
            return reinterpret_cast<ElementType*>(element.storage.data());
        }
        void deallocateBlock(IndexType index)
        {
            const IndexType page_index = getPageIndex(index);
            Page* page = pages[page_index];
            if (page)
            {
                const IndexType element_index = getElementIndex(index);
                ElementStorage& element = page->elements[element_index];
                element.allocated = false;
            }
        }
        IndexType getMaxAllocatedIndex() const
        {
            return max_allocated_index;
        }
        IndexType allocateIndex()
        {
            if (pageAllocated(next_available_index))
            {
                ElementStorage* info = element(next_available_index);
                IndexType result_index = next_available_index;
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
        void deallocateIndex(IndexType index)
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
        void chainPushFront(IndexType index)
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
        void chainRemove(IndexType index)
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            IndexType previous_index = info->chain_previous_index;
            IndexType next_index = info->chain_next_index;
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
        IndexType getChainHeadIndex() const
        {
            return chain_head_index;
        }
        IndexType getChainNextIndex(IndexType index) const
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            return info->chain_next_index;
        }
        FastIndex16BitPage()
        {
        }
        ~FastIndex16BitPage()
        {
            for (IndexType page_index = 0; page_index < PAGE_SIZE; ++page_index)
            {
                Page* page = pages[page_index];
                pages[page_index] = nullptr;
                if (page)
                {
                    delete page;
                }
            }
        }
    };

    template<class ElementType>
    class FastIndexMap : protected FastIndex16BitPage<ElementType>
    {
        typedef FastIndex16BitPage<ElementType> Base;

    public:
        ElementType* allocateBlock(IndexType index)
        {
            return Base::allocateBlock(index);
        }
        void deallocateBlock(IndexType index)
        {
            Base::deallocateBlock(index);
        }
        ElementType* find(IndexType index) const
        {
            return Base::find(index);
        }
    };

    template<class ElementType>
    class FastIndexRegistry : protected FastIndex16BitPage<ElementType>
    {
        typedef FastIndex16BitPage<ElementType> Base;

    public:
        ElementType* allocate(IndexType& result_index)
        {
            result_index = allocateIndex();
            return allocateBlock(result_index);
        }
        void deallocate(IndexType index)
        {
            deallocateBlock(index);
            deallocateIndex(index);
        }
        ElementType* find(IndexType index) const
        {
            return Base::find(index);
        }
    };

    template<class ElementType>
    class FastIndex8BitPage
    {
        constexpr std::uint8_t PAGE_BIT_COUNT = 8;
        const static IndexType PAGE_SIZE = (1 << PAGE_BIT_COUNT);

        static inline IndexType getElementIndex(IndexType index)
        {
            return index & ((1 << PAGE_BIT_COUNT) - 1);
        }
        static inline IndexType getPageIndex(IndexType index)
        {
            return (index & ((1 << PAGE_BIT_COUNT * 2) - 1)) >> PAGE_BIT_COUNT;
        }
        static inline IndexType getChapterIndex(IndexType index)
        {
            return (index & ((1 << PAGE_BIT_COUNT * 3) - 1)) >> (PAGE_BIT_COUNT * 2);
        }
        static inline IndexType getVolumeIndex(IndexType index)
        {
            return index >> (PAGE_BIT_COUNT * 3);
        }

        struct ElementStorage
        {
            std::atomic<bool> allocated = false;
            IndexType next_available_index = 0;
            std::atomic<IndexType> chain_previous_index = INVALID_INDEX;
            std::atomic<IndexType> chain_next_index = INVALID_INDEX;

            std::array<std::uint8_t, sizeof(ElementType)> storage;
        };
        struct Page
        {
            std::array<ElementStorage, PAGE_SIZE> elements;
        };
        struct Chapter
        {
            std::array<std::atomic<Page*>, PAGE_SIZE> pages;
        };
        struct Volume
        {
            std::array<std::atomic<Chapter*>, PAGE_SIZE> chapters;
        };

        std::array<std::atomic<Volume*>, PAGE_SIZE> volumes;
        IndexType max_allocated_index = 0;
        IndexType next_available_index = 0;
        std::atomic<IndexType> chain_head_index = INVALID_INDEX;

    protected:
        ElementStorage* element(IndexType index) const
        {
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume)
            {
                const IndexType chapter_index = getChapterIndex(index);
                Chapter* chapter = volume->chapters[chapter_index];
                if (chapter)
                {
                    const IndexType page_index = getPageIndex(index);
                    Page* page = chapter->pages[page_index];
                    if (page)
                    {
                        const IndexType element_index = getElementIndex(index);
                        return &page->elements[element_index];
                    }
                }
            }
            return nullptr;
        }
        ElementType* fast_find(IndexType index) const
        {
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume)
            {
                const IndexType chapter_index = getChapterIndex(index);
                Chapter* chapter = volume->chapters[chapter_index];
                if (chapter)
                {
                    const IndexType page_index = getPageIndex(index);
                    Page* page = chapter->pages[page_index];
                    if (page)
                    {
                        const IndexType element_index = getElementIndex(index);
                        ElementStorage& element = page->elements[element_index];
                        return reinterpret_cast<ElementType*>(element.storage.data());
                    }
                }
            }
            return nullptr;
        }
        ElementType* find(IndexType index) const
        {
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume)
            {
                const IndexType chapter_index = getChapterIndex(index);
                Chapter* chapter = volume->chapters[chapter_index];
                if (chapter)
                {
                    const IndexType page_index = getPageIndex(index);
                    Page* page = chapter->pages[page_index];
                    if (page)
                    {
                        const IndexType element_index = getElementIndex(index);
                        ElementStorage& element = page->elements[element_index];
                        if (element.allocated)
                        {
                            return reinterpret_cast<ElementType*>(element.storage.data());
                        }
                    }
                }
            }
            return nullptr;
        }
        bool pageAllocated(IndexType index) const
        {
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume)
            {
                const IndexType chapter_index = getChapterIndex(index);
                Chapter* chapter = volume->chapters[chapter_index];
                if (chapter)
                {
                    const IndexType page_index = getPageIndex(index);
                    return chapter->pages[page_index] ? true : false;
                }
            }
            return false;
        }
        ElementType* allocateBlock(IndexType index)
        {
            max_allocated_index = std::max(max_allocated_index, index);
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume == nullptr)
            {
                volume = new Volume;
                volumes[volume_index] = volume;
            }
            const IndexType chapter_index = getVolumeIndex(index);
            Chapter* chapter = volume->chapters[chapter_index];
            if (chapter == nullptr)
            {
                chapter = new Chapter;
                volume->chapters[chapter_index] = chapter;
            }
            const IndexType page_index = getPageIndex(index);
            Page* page = chapter->pages[page_index];
            if (page == nullptr)
            {
                page = new Page;
                chapter->pages[page_index] = page;
            }
            const IndexType element_index = getElementIndex(index);
            ElementStorage& element = page->elements[element_index];
            element.allocated = true;
            return reinterpret_cast<ElementType*>(element.storage.data());
        }
        void deallocateBlock(IndexType index)
        {
            const IndexType volume_index = getVolumeIndex(index);
            Volume* volume = volumes[volume_index];
            if (volume)
            {
                const IndexType chapter_index = getChapterIndex(index);
                Chapter* chapter = volume->chapters[chapter_index];
                if (chapter)
                {
                    const IndexType page_index = getPageIndex(index);
                    Page* page = chapter->pages[page_index];
                    if (page)
                    {
                        const IndexType element_index = getElementIndex(index);
                        ElementStorage& element = page->elements[element_index];
                        element.allocated = false;
                    }
                }
            }
        }
        IndexType getMaxAllocatedIndex() const
        {
            return max_allocated_index;
        }
        IndexType allocateIndex()
        {
            if (pageAllocated(next_available_index))
            {
                ElementStorage* info = element(next_available_index);
                IndexType result_index = next_available_index;
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
        void deallocateIndex(IndexType index)
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
        void chainPushFront(IndexType index)
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
        void chainRemove(IndexType index)
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            IndexType previous_index = info->chain_previous_index;
            IndexType next_index = info->chain_next_index;
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
        IndexType getChainHeadIndex() const
        {
            return chain_head_index;
        }
        IndexType getChainNextIndex(IndexType index) const
        {
            assert(pageAllocated(index));
            ElementStorage* info = element(index);
            return info->chain_next_index;
        }
        FastIndex8BitPage()
        {
        }
        ~FastIndex8BitPage()
        {
            for (IndexType volume_index = 0; volume_index < PAGE_SIZE; ++volume_index)
            {
                Volume* volume = volumes[volume_index];
                volumes[volume_index] = nullptr;
                if (volume)
                {
                    for (IndexType chapter_index = 0; chapter_index < PAGE_SIZE; ++chapter_index)
                    {
                        Chapter* chapter = volume->chapters[chapter_index];
                        volume->chapters[chapter_index] = nullptr;
                        if (chapter)
                        {
                            for (IndexType page_index = 0; page_index < PAGE_SIZE; ++page_index)
                            {
                                Page* page = chapter->pages[page_index];
                                chapter->pages[page_index] = nullptr;
                                if (page)
                                {
                                    delete page;
                                }
                            }
                            delete chapter;
                        }
                    }
                    delete volume;
                }
            }
        }
    };

    template<class ElementType>
    class FastIndexChain : protected FastIndex8BitPage<ElementType>
    {
        typedef FastIndex8BitPage<ElementType> Base;

    public:
        ElementType* chainPushFront(IndexType& result_index)
        {
            result_index = allocateIndex();
            Base::chainPushFront(result_index);
            return allocateBlock(result_index);
        }
        void chainRemove(IndexType index)
        {
            deallocateBlock(index);
            Base::chainRemove(index);
            deallocateIndex(index);
        }
        IndexType getChainHeadIndex() const
        {
            return Base::getChainHeadIndex();
        }
        IndexType getChainNextIndex(IndexType index) const
        {
            return Base::getChainNextIndex(index);
        }
        ElementType* get(IndexType index) const
        {
            return Base::fast_find(index);
        }
        ElementType* find(IndexType index) const
        {
            return Base::find(index);
        }
    };
}
