// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include "pool.h"
#include "log.h"
#include "fast_index.h"

extern Log::Logger* g_logger = nullptr;

struct A
{
    unsigned int id = 0;
    double data[16] = { 0 };
};

static std::uint32_t random;

void my_rand_reset()
{
    random = 57568;
}

std::uint32_t my_rand()
{
    random = random * 5457 + 46451;
    return random;
}

typedef Memory::FastIndexMap<A> FastIndexMap;
FastIndexMap map_data6;
void main6()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        auto it = map_data6.find(index);
        if (!it)
        {
            A* new_element = map_data6.allocateBlock(index);
            new(new_element) A();
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            A* it = map_data6.find(x);
            if (it)
            {
                it->~A();
                map_data6.deallocateBlock(x);
            }
        }
        else
        {
            A* it = map_data6.find(x);
            if (!it)
            {
                A* new_element = map_data6.allocateBlock(x);
                new(new_element) A();
            }
        }
    }
}

typedef std::unordered_map<std::uint32_t, A*> UnorderedMap;
UnorderedMap map_data5;
void main5()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        UnorderedMap::iterator it = map_data5.find(index);
        if (it == map_data5.end())
        {
            map_data5.insert(UnorderedMap::value_type(index, new A));
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            UnorderedMap::iterator it = map_data5.find(x);
            if (it != map_data5.end())
            {
                delete it->second;
                map_data5.erase(it);
            }
        }
        else
        {
            UnorderedMap::iterator it = map_data5.find(x);
            if (it == map_data5.end())
            {
                map_data5.insert(UnorderedMap::value_type(x, new A));
            }
        }
    }
}

typedef std::vector<A*> Vector;
Vector map_data1(1000000);
void main1()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        if (!map_data1[index])
        {
            map_data1[index] = new A;
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            if (map_data1[x])
            {
                delete map_data1[x];
                map_data1[x] = nullptr;
            }
        }
        else
        {
            if (!map_data1[x])
            {
                map_data1[x] = new A;
            }
        }
    }
}

typedef std::map<std::uint32_t, A*> Map;
Map map_data2;
void main2()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        Map::iterator it = map_data2.find(index);
        if (it == map_data2.end())
        {
            map_data2.insert(Map::value_type(index, new A));
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            Map::iterator it = map_data2.find(x);
            if (it != map_data2.end())
            {
                delete it->second;
                map_data2.erase(it);
            }
        }
        else
        {
            Map::iterator it = map_data2.find(x);
            if (it == map_data2.end())
            {
                map_data2.insert(Map::value_type(x, new A));
            }
        }
    }
}

Memory::Pool<A, 1000000> vector_pool;
Vector map_data3(1000000);
void main3()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        if (!map_data3[index])
        {
            map_data3[index] = vector_pool.allocate();
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            if (map_data3[x])
            {
                vector_pool.deallocate(map_data3[x]);
                map_data3[x] = nullptr;
            }
        }
        else
        {
            if (!map_data3[x])
            {
                map_data3[x] = vector_pool.allocate();
            }
        }
    }
}


Memory::Pool<A, 1000000> map_pool;
Map map_data4;
void main4()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        Map::iterator it = map_data4.find(index);
        if (it == map_data4.end())
        {
            map_data4.insert(Map::value_type(index, map_pool.allocate()));
        }
        else
        {
            index = 0;
        }
    }
    for (size_t i = 0; i < 4000000; ++i)
    {
        std::uint32_t x = my_rand() % 1000000;
        if (my_rand() % 2)
        {
            Map::iterator it = map_data4.find(x);
            if (it != map_data4.end())
            {
                map_pool.deallocate(it->second);
                map_data4.erase(it);
            }
        }
        else
        {
            Map::iterator it = map_data4.find(x);
            if (it == map_data4.end())
            {
                map_data4.insert(Map::value_type(x, map_pool.allocate()));
            }
        }
    }
}

void validate(const Vector& vec1, const Vector& vec2)
{
    bool validation_error = false;
    for (size_t i = 0; i < 1000000; ++i)
    {
        if (vec1[i] && !vec2[i] || !vec1[i] && vec2[i])
        {
            validation_error = true;
            break;
        }
    }

    if (!validation_error)
    {
        std::cout << "validation OK" << std::endl;
    }
    else
    {
        std::cout << "validation failed" << std::endl;
    }
}

void validate(const Map& map1, const Map& map2)
{
    bool validation_error = false;
    Map::const_iterator it1 = map1.begin();
    Map::const_iterator it2 = map2.begin();
    while (it1 != map1.end())
    {
        if (it1->first != it2->first)
        {
            validation_error = true;
            break;
        }
        ++it1;
        ++it2;
    }
    it1 = map1.begin();
    it2 = map2.begin();
    while (it2 != map2.end())
    {
        if (it1->first != it2->first)
        {
            validation_error = true;
            break;
        }
        ++it1;
        ++it2;
    }

    if (!validation_error)
    {
        std::cout << "validation OK" << std::endl;
    }
    else
    {
        std::cout << "validation failed" << std::endl;
    }
}

void validate(const Vector& vec, const Map& map)
{
    bool validation_error = false;
    Map::const_iterator it = map.begin();
    for (size_t i = 0; i < 1000000; ++i)
    {
        if (vec[i])
        {
            if (it->first != i)
            {
                validation_error = true;
                break;
            }
            ++it;
        }
    }
    it = map.begin();
    while (it != map.end())
    {
        if (!vec[it->first])
        {
            validation_error = true;
            break;
        }
        ++it;
    }

    if (!validation_error)
    {
        std::cout << "validation OK" << std::endl;
    }
    else
    {
        std::cout << "validation failed" << std::endl;
    }
}

void validate(const Map& map1, const UnorderedMap& map2)
{
    bool validation_error = false;
    Map::const_iterator it1 = map1.begin();
    UnorderedMap::const_iterator it2 = map2.begin();
    while (it1 != map1.end())
    {
        if (it1->first != it2->first)
        {
            validation_error = true;
            break;
        }
        ++it1;
        ++it2;
    }
    it1 = map1.begin();
    it2 = map2.begin();
    while (it2 != map2.end())
    {
        if (it1->first != it2->first)
        {
            validation_error = true;
            break;
        }
        ++it1;
        ++it2;
    }

    if (!validation_error)
    {
        std::cout << "validation OK" << std::endl;
    }
    else
    {
        std::cout << "validation failed" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    {
        my_rand_reset();
        auto start = std::chrono::system_clock::now();
        main1();
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << elapsed.count() << '\n';
    }

    {
        my_rand_reset();
        auto start = std::chrono::system_clock::now();
        main2();
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << elapsed.count() << '\n';
    }

    {
        my_rand_reset();
        auto start = std::chrono::system_clock::now();
        main3();
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << elapsed.count() << '\n';
    }

    {
        my_rand_reset();
        auto start = std::chrono::system_clock::now();
        main4();
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << elapsed.count() << '\n';
    }

    {
        my_rand_reset();
        auto start = std::chrono::system_clock::now();
        main5();
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << elapsed.count() << '\n';
    }

    //{
    //    my_rand_reset();
    //    auto start = std::chrono::system_clock::now();
    //    main6();
    //    auto end = std::chrono::system_clock::now();
    //    auto elapsed = end - start;
    //    std::cout << elapsed.count() << '\n';
    //}

    validate(map_data1, map_data2);
    validate(map_data1, map_data3);
    validate(map_data3, map_data2);
    validate(map_data1, map_data4);
    validate(map_data3, map_data4);
    validate(map_data2, map_data5);

    return EXIT_SUCCESS;
}
