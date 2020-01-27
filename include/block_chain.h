// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cassert>

template<class BlockType>
inline void removeBlock(BlockType* block, BlockType*& head)
{
    assert(block);
    BlockType* previous = block->previous;
    BlockType* next = block->next;
    if (previous)
    {
        previous->next = next;
    }
    else
    {
        head = next;
    }
    if (next)
    {
        next->previous = previous;
    }
}

template<class BlockType>
inline void pushFrontBlock(BlockType* block, BlockType*& head)
{
    block->next = head;
    if (head)
    {
        head->previous = block;
    }
    head = block;
}
