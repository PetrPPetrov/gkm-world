// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include "pool.h"
#include "fast_index_map.h"

extern std::ofstream* g_log_file = nullptr;

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

typedef Memory::FastIndexMap<A> fast_index_map_t;
fast_index_map_t map_data6;
void main6()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        auto it = map_data6.find(index);
        if (!it)
        {
            A* new_element = map_data6.allocate(index);
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
                map_data6.deallocate(x);
            }
        }
        else
        {
            A* it = map_data6.find(x);
            if (!it)
            {
                A* new_element = map_data6.allocate(x);
                new(new_element) A();
            }
        }
    }
}

typedef std::unordered_map<std::uint32_t, A*> unordered_map_t;
unordered_map_t map_data5;
void main5()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        unordered_map_t::iterator it = map_data5.find(index);
        if (it == map_data5.end())
        {
            map_data5.insert(unordered_map_t::value_type(index, new A));
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
            unordered_map_t::iterator it = map_data5.find(x);
            if (it != map_data5.end())
            {
                delete it->second;
                map_data5.erase(it);
            }
        }
        else
        {
            unordered_map_t::iterator it = map_data5.find(x);
            if (it == map_data5.end())
            {
                map_data5.insert(unordered_map_t::value_type(x, new A));
            }
        }
    }
}

typedef std::vector<A*> vector_t;
vector_t map_data1(1000000);
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

typedef std::map<std::uint32_t, A*> map_t;
map_t map_data2;
void main2()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        map_t::iterator it = map_data2.find(index);
        if (it == map_data2.end())
        {
            map_data2.insert(map_t::value_type(index, new A));
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
            map_t::iterator it = map_data2.find(x);
            if (it != map_data2.end())
            {
                delete it->second;
                map_data2.erase(it);
            }
        }
        else
        {
            map_t::iterator it = map_data2.find(x);
            if (it == map_data2.end())
            {
                map_data2.insert(map_t::value_type(x, new A));
            }
        }
    }
}

Memory::Pool<A, 1000000> vector_pool;
vector_t map_data3(1000000);
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
map_t map_data4;
void main4()
{
    for (size_t i = 0; i < 500000; ++i)
    {
        std::uint32_t index = my_rand() % 1000000;
        map_t::iterator it = map_data4.find(index);
        if (it == map_data4.end())
        {
            map_data4.insert(map_t::value_type(index, map_pool.allocate()));
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
            map_t::iterator it = map_data4.find(x);
            if (it != map_data4.end())
            {
                map_pool.deallocate(it->second);
                map_data4.erase(it);
            }
        }
        else
        {
            map_t::iterator it = map_data4.find(x);
            if (it == map_data4.end())
            {
                map_data4.insert(map_t::value_type(x, map_pool.allocate()));
            }
        }
    }
}

void validate(const vector_t& vec1, const vector_t& vec2)
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

void validate(const map_t& map1, const map_t& map2)
{
    bool validation_error = false;
    map_t::const_iterator it1 = map1.begin();
    map_t::const_iterator it2 = map2.begin();
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

void validate(const vector_t& vec, const map_t& map)
{
    bool validation_error = false;
    map_t::const_iterator it = map.begin();
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

void validate(const map_t& map1, const unordered_map_t& map2)
{
    bool validation_error = false;
    map_t::const_iterator it1 = map1.begin();
    unordered_map_t::const_iterator it2 = map2.begin();
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
