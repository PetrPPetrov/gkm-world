// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>

// Fowler / Noll / Vo (FNV) Hash
struct FnvHash
{
    void Update(const std::uint8_t* data, std::size_t size)
    {
        for (std::size_t i = 0; i < size; ++i)
        {
            hash = hash ^ (data[i]);         // xor the low 8 bits
            hash = hash * g_fnv_multiple; // multiply by the magic number
        }
    }
    std::uint64_t getHash() const
    {
        return hash;
    }
private:
    // Magic numbers are from http://www.isthe.com/chongo/tech/comp/fnv/
    static const std::uint64_t g_initial_fnv = 2166136261U;
    static const std::uint64_t g_fnv_multiple = 16777619;
    std::uint64_t hash = g_initial_fnv;
};
