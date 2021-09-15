// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include "gkm_world/logger.h"
#include "gkm_world/fast_index.h"

constexpr std::size_t ARRAY_SIZE = 50000000;
constexpr std::size_t INIT_COUNT = ARRAY_SIZE / 2;
constexpr std::size_t REALLOC_COUNT = ARRAY_SIZE * 5;

struct A {
    unsigned int id = 0;
    double data[16] = { 0 };
};

static std::uint32_t random;

void my_rand_reset() {
    random = 57568;
}

std::uint32_t my_rand() {
    random = random * 5457 + 46451;
    return random;
}

void test_vector_raw_pointer() {
    std::vector<A*> map_data(ARRAY_SIZE);
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        if (!map_data[index]) {
            map_data[index] = new A;
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            if (map_data[x]) {
                delete map_data[x];
                map_data[x] = nullptr;
            }
        } else {
            if (!map_data[x]) {
                map_data[x] = new A;
            }
        }
    }
}

void test_vector_smart_pointer() {
    std::vector<std::unique_ptr<A>> map_data(ARRAY_SIZE);
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        if (!map_data[index]) {
            map_data[index] = std::make_unique<A>();
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            if (map_data[x]) {
                map_data[x].reset(nullptr);
            }
        } else {
            if (!map_data[x]) {
                map_data[x] = std::make_unique<A>();
            }
        }
    }
}

void test_vector_value() {
    struct B {
        bool is_allocated = false;
        A data;
    };
    std::vector<B> map_data(ARRAY_SIZE);
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        if (!map_data[index].is_allocated) {
            map_data[index].is_allocated = true;
            map_data[index].data = A();
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            if (map_data[x].is_allocated) {
                map_data[x].is_allocated = false;
                map_data[x].data.~A();
            }
        } else {
            if (!map_data[x].is_allocated) {
                map_data[x].is_allocated = true;
                map_data[x].data = A();
            }
        }
    }
}

void test_map_raw_pointer() {
    std::map<std::uint32_t, A*> map_data;
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        auto it = map_data.find(index);
        if (it == map_data.end()) {
            map_data.emplace(index, new A);
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            auto it = map_data.find(x);
            if (it != map_data.end()) {
                delete it->second;
                map_data.erase(it);
            }
        } else {
            auto it = map_data.find(x);
            if (it == map_data.end()) {
                map_data.emplace(x, new A);
            }
        }
    }
}

void test_map_smart_pointer() {
    std::map<std::uint32_t, std::unique_ptr<A>> map_data;
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        auto it = map_data.find(index);
        if (it == map_data.end()) {
            map_data.emplace(index, std::make_unique<A>());
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            auto it = map_data.find(x);
            if (it != map_data.end()) {
                map_data.erase(it);
            }
        } else {
            auto it = map_data.find(x);
            if (it == map_data.end()) {
                map_data.emplace(x, std::make_unique<A>());
            }
        }
    }
}

void test_unordered_map_raw_pointer() {
    std::unordered_map<std::uint32_t, A*> map_data;
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        auto it = map_data.find(index);
        if (it == map_data.end()) {
            map_data.emplace(index, new A);
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            auto it = map_data.find(x);
            if (it != map_data.end()) {
                delete it->second;
                map_data.erase(it);
            }
        } else {
            auto it = map_data.find(x);
            if (it == map_data.end()) {
                map_data.emplace(x, new A);
            }
        }
    }
}

void test_unordered_map_smart_pointer() {
    std::unordered_map<std::uint32_t, std::unique_ptr<A>> map_data;
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        auto it = map_data.find(index);
        if (it == map_data.end()) {
            map_data.emplace(index, std::make_unique<A>());
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            auto it = map_data.find(x);
            if (it != map_data.end()) {
                map_data.erase(it);
            }
        } else {
            auto it = map_data.find(x);
            if (it == map_data.end()) {
                map_data.emplace(x, std::make_unique<A>());
            }
        }
    }
}

void test_fast_index_map() {
    auto map_data = std::make_unique<Memory::FastIndexMap<A, 16>>();
    for (std::size_t i = 0; i < INIT_COUNT; ++i) {
        std::uint32_t index = my_rand() % ARRAY_SIZE;
        auto it = map_data->find(index);
        if (!it) {
            A* new_element = map_data->allocateBlock(index);
            new(new_element) A();
        } else {
            index = 0;
        }
    }
    for (std::size_t i = 0; i < REALLOC_COUNT; ++i) {
        std::uint32_t x = my_rand() % ARRAY_SIZE;
        if (my_rand() % 2) {
            A* it = map_data->find(x);
            if (it) {
                it->~A();
                map_data->deallocateBlock(x);
            }
        } else {
            A* it = map_data->find(x);
            if (!it) {
                A* new_element = map_data->allocateBlock(x);
                new(new_element) A();
            }
        }
    }
}

template<class Operation>
void test(const std::string description, Operation&& operation) {
    my_rand_reset();
    auto start = std::chrono::high_resolution_clock::now();
    operation();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end - start;
    std::cout << description << ": duration " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " (milliseconds)\n";
}

int main(int argc, char* argv[]) {
    test("std::vector<A*>", test_vector_raw_pointer);
    test("std::vector<std::uniquie_ptr<A>>", test_vector_smart_pointer);
    test("std::vector<A>", test_vector_value);
    test("std::map<std::uint32_t, A*>", test_map_raw_pointer);
    test("std::map<std::uint32_t, std::uniquie_ptr<A>>", test_map_smart_pointer);
    test("std::unordered_map<std::uint32_t, A*>", test_unordered_map_raw_pointer);
    test("std::unordered_map<std::uint32_t, std::uniquie_ptr<A>>", test_unordered_map_smart_pointer);
    test("Memory::FastIndex16BitPage", test_fast_index_map);

    return EXIT_SUCCESS;
}
