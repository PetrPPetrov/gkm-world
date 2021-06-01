// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cassert>
#include <list>

template<typename ValueType>
struct BlockChain
{
    ValueType value;
    BlockChain<ValueType>* previous = nullptr;
    BlockChain<ValueType>* next = nullptr;

    BlockChain(ValueType value_) : value(value_)
    {
    }
    BlockChain() = default;
    BlockChain(const BlockChain&) = delete;
    BlockChain& operator=(const BlockChain&) = delete;
};

template<class ValueType>
void removeBlock(BlockChain<ValueType>* block, BlockChain<ValueType>*& head)
{
    assert(block);
    BlockChain<ValueType>* previous = block->previous;
    BlockChain<ValueType>* next = block->next;
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
void pushFrontBlock(BlockChain<ValueType>* block, BlockChain<ValueType>*& head)
{
    block->next = head;
    if (head)
    {
        head->previous = block;
    }
    head = block;
}
