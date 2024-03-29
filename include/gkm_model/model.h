// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <array>
#include <list>
#include <vector>
#include <memory>
#include <stdexcept>

namespace Serialization {
    template<typename Buffer>
    static inline size_t bytesAvailable(Buffer& buffer, size_t index) {
        const size_t buffer_size = buffer.size();
        if (index <= buffer_size) {
            return buffer_size - index;
        } else {
            return 0;
        }
    }

    static inline std::uint8_t readByte(const std::vector<std::uint8_t>& buffer, size_t& index) {
        if (bytesAvailable(buffer, index) >= 1) {
            return buffer[index++];
        } else {
            throw std::runtime_error("unexpected end of data");
        }
    }

    static inline std::uint16_t readWord(const std::vector<std::uint8_t>& buffer, size_t& index) {
        std::uint16_t lo_byte = readByte(buffer, index);
        std::uint16_t hi_byte = readByte(buffer, index);
        std::uint16_t result = (hi_byte << 8) + lo_byte;
        return result;
    }

    static inline std::uint32_t readDoubleWord(const std::vector<std::uint8_t>& buffer, size_t& index) {
        std::uint32_t byte0 = readByte(buffer, index);
        std::uint32_t byte1 = readByte(buffer, index);
        std::uint32_t byte2 = readByte(buffer, index);
        std::uint32_t byte3 = readByte(buffer, index);
        std::uint32_t result = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
        return result;
    }

    static inline float readFloat(const std::vector<std::uint8_t>& buffer, size_t& index) {
        union {
            float floating_number;
            std::uint32_t raw_data;
        } exchange;
        exchange.raw_data = readDoubleWord(buffer, index);
        return exchange.floating_number;
    }

    template<typename Buffer>
    static inline void saveByte(Buffer& buffer, size_t& index, std::uint8_t byte) {
        if (bytesAvailable(buffer, index) >= 1) {
            buffer[index++] = byte;
        } else {
            throw std::runtime_error("unexpected end of space");
        }
    }

    template<typename Buffer>
    static inline void saveWord(Buffer& buffer, size_t& index, std::uint16_t word) {
        const std::uint8_t lo_byte = word & 0x00FF;
        const std::uint8_t hi_byte = (word & 0xFF00) >> 8;
        saveByte(buffer, index, lo_byte);
        saveByte(buffer, index, hi_byte);
    }

    template<typename Buffer>
    static inline void saveDoubleWord(Buffer& buffer, size_t& index, std::uint32_t double_word) {
        const std::uint8_t byte0 = double_word & 0x000000FF;
        const std::uint8_t byte1 = (double_word & 0x0000FF00) >> 8;
        const std::uint8_t byte3 = (double_word & 0x00FF0000) >> 16;
        const std::uint8_t byte4 = (double_word & 0xFF000000) >> 24;
        saveByte(buffer, index, byte0);
        saveByte(buffer, index, byte1);
        saveByte(buffer, index, byte3);
        saveByte(buffer, index, byte4);
    }

    template<typename Buffer>
    static inline void saveFloat(Buffer& buffer, size_t& index, float floating_number) {
        union {
            float floating_number;
            std::uint32_t raw_data;
        } exchange;
        exchange.floating_number = floating_number;
        saveDoubleWord(buffer, index, exchange.raw_data);
    }

    struct DummyVector {
        std::uint8_t dummy = 0;

        std::uint8_t& operator[](size_t index) {
            return dummy;
        }
        size_t size() const {
            return std::numeric_limits<size_t>::max();
        }
    };
}

namespace GkmModelRev0 {
    const std::int16_t TEX_COORD_MULTIPLIER = 4096;
    struct Vertex {
        float x = 0;
        float y = 0;
        float z = 0;
        std::int16_t u = 0;
        std::int16_t v = 0;
    };

    struct Mesh {
        typedef std::shared_ptr<Mesh> Ptr;

        std::uint8_t relative_texture_id = 0;
        std::vector<Vertex> vertices;
    };

    struct Resource {
        typedef std::shared_ptr<Resource> Ptr;

        const std::uint8_t revision = 0;
        std::list<Mesh::Ptr> meshes;
    };

    struct Model {
        typedef std::shared_ptr<Model> Ptr;

        const std::uint8_t revision = 0;
        std::uint16_t resource_id;
        const static std::size_t TEXTURE_COUNT = 16;
        std::array<std::uint16_t, TEXTURE_COUNT> texture_ids = { 0 };
    };

    static inline Mesh::Ptr loadMesh(const std::vector<std::uint8_t>& buffer, size_t& index) {
        using namespace Serialization;

        Mesh::Ptr result_mesh = std::make_shared<Mesh>();
        result_mesh->relative_texture_id = readByte(buffer, index);
        const std::uint16_t vertex_count = readWord(buffer, index);
        result_mesh->vertices.reserve(vertex_count);
        for (std::uint16_t i = 0; i < vertex_count; ++i) {
            Vertex new_vertex;
            new_vertex.x = readFloat(buffer, index);
            new_vertex.y = readFloat(buffer, index);
            new_vertex.z = readFloat(buffer, index);
            new_vertex.u = static_cast<std::int16_t>(readWord(buffer, index));
            new_vertex.v = static_cast<std::int16_t>(readWord(buffer, index));
            result_mesh->vertices.push_back(new_vertex);
        }
        return result_mesh;
    }

    static inline Resource::Ptr loadResource(const std::vector<std::uint8_t>& buffer, size_t& index) {
        using namespace Serialization;

        if (readByte(buffer, index) != 'G') {
            throw std::runtime_error("wrong signature (GKR)");
        }
        if (readByte(buffer, index) != 'K') {
            throw std::runtime_error("wrong signature (GKR)");
        }
        if (readByte(buffer, index) != 'R') {
            throw std::runtime_error("wrong signature (GKR)");
        }
        const std::uint16_t revision = readByte(buffer, index);
        if (revision != 0) {
            throw std::runtime_error("wrong revision (0)");
        }
        Resource::Ptr result_resource = std::make_shared<Resource>();
        const std::uint8_t mesh_count = readByte(buffer, index);
        for (std::uint8_t i = 0; i < mesh_count; ++i) {
            Mesh::Ptr new_mesh = loadMesh(buffer, index);
            result_resource->meshes.push_back(new_mesh);
        }
        return result_resource;
    }

    static inline Model::Ptr loadModel(const std::vector<std::uint8_t>& buffer, size_t& index) {
        using namespace Serialization;

        if (readByte(buffer, index) != 'G') {
            throw std::runtime_error("wrong signature (GKM)");
        }
        if (readByte(buffer, index) != 'K') {
            throw std::runtime_error("wrong signature (GKM)");
        }
        if (readByte(buffer, index) != 'M') {
            throw std::runtime_error("wrong signature (GKM)");
        }
        const std::uint16_t revision = readByte(buffer, index);
        if (revision != 0) {
            throw std::runtime_error("wrong revision (0)");
        }
        Model::Ptr result_model = std::make_shared<Model>();
        result_model->resource_id = readWord(buffer, index);
        for (std::uint8_t i = 0; i < result_model->texture_ids.size(); ++i) {
            result_model->texture_ids[i] = readWord(buffer, index);
        }
        return result_model;
    }

    template<typename Buffer>
    void saveMesh(const Mesh::Ptr& mesh, Buffer& buffer, size_t& index) {
        using namespace Serialization;

        saveByte(buffer, index, mesh->relative_texture_id);
        const std::uint16_t vertex_count = static_cast<std::uint16_t>(mesh->vertices.size());
        saveWord(buffer, index, vertex_count);
        for (std::uint16_t i = 0; i < vertex_count; ++i) {
            saveFloat(buffer, index, mesh->vertices[i].x);
            saveFloat(buffer, index, mesh->vertices[i].y);
            saveFloat(buffer, index, mesh->vertices[i].z);
            saveWord(buffer, index, static_cast<std::uint16_t>(mesh->vertices[i].u));
            saveWord(buffer, index, static_cast<std::uint16_t>(mesh->vertices[i].v));
        }
    }

    template<typename Buffer>
    void saveResource(const Resource::Ptr& resource, Buffer& buffer, size_t& index) {
        using namespace Serialization;

        saveByte(buffer, index, 'G');
        saveByte(buffer, index, 'K');
        saveByte(buffer, index, 'R');
        saveByte(buffer, index, resource->revision);
        const std::uint8_t mesh_count = static_cast<std::uint8_t>(resource->meshes.size());
        saveByte(buffer, index, mesh_count);
        auto it = resource->meshes.begin();
        for (std::uint8_t i = 0; i < mesh_count; ++i) {
            saveMesh(*it++, buffer, index);
        }
    }

    // We use C++ 11 move semantic feature to avoid copying std::vector
    static inline std::vector<std::uint8_t> saveResource(const Resource::Ptr& resource) {
        using namespace Serialization;

        DummyVector dummy;
        size_t required_size = 0;
        saveResource(resource, dummy, required_size);
        std::vector<std::uint8_t> result_buffer;
        result_buffer.resize(required_size, 0);
        size_t index = 0;
        saveResource(resource, result_buffer, index);
        return result_buffer;
    }

    // We use C++ 11 move semantic feature to avoid copying std::vector
    static inline std::vector<std::uint8_t> saveModel(const Model::Ptr& model) {
        using namespace Serialization;

        std::vector<std::uint8_t> buffer(sizeof(Model) + 3);
        size_t index = 0;

        saveByte(buffer, index, 'G');
        saveByte(buffer, index, 'K');
        saveByte(buffer, index, 'M');
        saveByte(buffer, index, model->revision);
        saveWord(buffer, index, model->resource_id);
        for (std::uint8_t i = 0; i < model->texture_ids.size(); ++i) {
            saveWord(buffer, index, model->texture_ids[i]);
        }
        return buffer;
    }
}
