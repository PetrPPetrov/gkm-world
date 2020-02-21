// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cassert>
#include <list>

template<typename ValueType>
struct ChainBlock
{
    ValueType* pointer = nullptr;
    ChainBlock<ValueType>* previous = nullptr;
    ChainBlock<ValueType>* next = nullptr;

    ChainBlock(ValueType* pointer_) : pointer(pointer_)
    {
    }
};

template<class ValueType>
void removeBlock(ChainBlock<ValueType>* block, ChainBlock<ValueType>*& head)
{
    assert(block);
    ChainBlockType* previous = block->previous;
    ChainBlockType* next = block->next;
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

template<class ValueType>
void pushFrontBlock(ChainBlock<ValueType>* block, ChainBlock<ValueType>*& head)
{
    block->next = head;
    if (head)
    {
        head->previous = block;
    }
    head = block;
}
