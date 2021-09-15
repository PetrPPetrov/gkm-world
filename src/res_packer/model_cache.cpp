// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <tiny_obj_loader.h>
#include "gkm_world/fnv_hash.h"
#include "model_cache.h"

static inline std::uint64_t calcHash(const std::string& file_name) {
    std::ifstream input_file(file_name, std::ifstream::binary);
    if (input_file) {
        input_file.seekg(0, input_file.end);
        std::size_t length = input_file.tellg();
        input_file.seekg(0, input_file.beg);

        std::vector<std::uint8_t> buffer(length);
        input_file.read(reinterpret_cast<char*>(&buffer[0]), length);
        input_file.close();

        FnvHash fnv_hash;
        fnv_hash.Update(&buffer[0], length);
        return fnv_hash.getHash();
    } else {
        throw std::runtime_error(file_name + " could not be opened");
    }
}

static inline std::uint64_t calcHash(const std::vector<std::uint8_t>& data) {
    FnvHash fnv_hash;
    fnv_hash.Update(&data[0], data.size());
    return fnv_hash.getHash();
}

static inline void save(const std::string& file_name, const std::vector<std::uint8_t>& data) {
    std::ofstream output_file(file_name, std::ofstream::binary);
    if (output_file) {
        output_file.write(reinterpret_cast<const char*>(&data[0]), data.size());
        output_file.close();
    } else {
        throw std::runtime_error(file_name + " could not be opened");
    }
}

ResourceModelInfo ModelCache::loadResource(const std::string& file_name) const {
    GkmModelRev0::Model::Ptr result_model = std::make_shared<GkmModelRev0::Model>();
    GkmModelRev0::Resource::Ptr result_resource = std::make_shared<GkmModelRev0::Resource>();
    tinyobj::ObjReader obj_reader;
    if (obj_reader.ParseFromFile(file_name)) {
        auto& materials = obj_reader.GetMaterials();
        auto& vertices = obj_reader.GetAttrib().vertices;
        auto& tex_coords = obj_reader.GetAttrib().texcoords;
        auto& shapes = obj_reader.GetShapes();
        std::unordered_map<int, size_t> material_to_vertex_count;
        for (auto& shape : shapes) {
            auto& mesh = shape.mesh;
            const size_t num_faces = mesh.num_face_vertices.size();
            for (size_t face_index = 0; face_index < num_faces; ++face_index) {
                const unsigned char num_face_vertices = mesh.num_face_vertices[face_index];
                std::size_t face_vertex_count = 0;
                if (num_face_vertices >= 3) {
                    const size_t num_triangles = static_cast<size_t>(num_face_vertices) - 2;
                    face_vertex_count += num_triangles * 3;
                }
                const int material_index = mesh.material_ids[face_index];
                auto find_material_it = material_to_vertex_count.find(material_index);
                if (find_material_it == material_to_vertex_count.end()) {
                    find_material_it = material_to_vertex_count.emplace(material_index, 0).first;
                    if (material_to_vertex_count.size() > result_model->texture_ids.size()) {
                        throw std::runtime_error("max number of materials was achieved");
                    }
                }
                find_material_it->second += face_vertex_count;
            }
        }
        std::unordered_map<int, GkmModelRev0::Mesh::Ptr> material_to_mesh;
        for (auto& shape : shapes) {
            auto& mesh = shape.mesh;
            auto& indices = mesh.indices;
            size_t cur_index = 0;
            const size_t num_faces = mesh.num_face_vertices.size();
            for (size_t face_index = 0; face_index < num_faces; ++face_index) {
                const int material_index = mesh.material_ids[face_index];
                const unsigned char num_face_vertices = mesh.num_face_vertices[face_index];
                auto find_material_it = material_to_mesh.find(material_index);
                if (find_material_it == material_to_mesh.end()) {
                    GkmModelRev0::Mesh::Ptr new_mesh = std::make_shared<GkmModelRev0::Mesh>();
                    new_mesh->vertices.reserve(material_to_vertex_count[material_index]);
                    new_mesh->relative_texture_id = static_cast<std::uint8_t>(material_to_mesh.size());
                    find_material_it = material_to_mesh.emplace(material_index, new_mesh).first;
                    if (material_to_mesh.size() > result_model->texture_ids.size()) {
                        throw std::runtime_error("max number of materials was achieved");
                    }
                    result_model->texture_ids[new_mesh->relative_texture_id] = texture_cache->getTextureId(materials.at(material_index).diffuse_texname);
                }
                GkmModelRev0::Mesh::Ptr& new_mesh = find_material_it->second;
                for (unsigned char i = 2; i < num_face_vertices; ++i) {
                    auto base_vertex = indices[cur_index];
                    auto next_vertex = indices[cur_index + i - 1];
                    auto last_vertex = indices[cur_index + i];
                    GkmModelRev0::Vertex v0, v1, v2;
                    v0.x = vertices.at(static_cast<size_t>(base_vertex.vertex_index) * 3 + 0);
                    v0.y = vertices.at(static_cast<size_t>(base_vertex.vertex_index) * 3 + 1);
                    v0.z = vertices.at(static_cast<size_t>(base_vertex.vertex_index) * 3 + 2);
                    v0.u = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(base_vertex.texcoord_index) * 2 + 0) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    v0.v = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(base_vertex.texcoord_index) * 2 + 1) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    v1.x = vertices.at(static_cast<size_t>(next_vertex.vertex_index) * 3 + 0);
                    v1.y = vertices.at(static_cast<size_t>(next_vertex.vertex_index) * 3 + 1);
                    v1.z = vertices.at(static_cast<size_t>(next_vertex.vertex_index) * 3 + 2);
                    v1.u = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(next_vertex.texcoord_index) * 2 + 0) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    v1.v = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(next_vertex.texcoord_index) * 2 + 1) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    v2.x = vertices.at(static_cast<size_t>(last_vertex.vertex_index) * 3 + 0);
                    v2.y = vertices.at(static_cast<size_t>(last_vertex.vertex_index) * 3 + 1);
                    v2.z = vertices.at(static_cast<size_t>(last_vertex.vertex_index) * 3 + 2);
                    v2.u = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(last_vertex.texcoord_index) * 2 + 0) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    v2.v = static_cast<std::int16_t>(tex_coords.at(static_cast<size_t>(last_vertex.texcoord_index) * 2 + 1) * GkmModelRev0::TEX_COORD_MULTIPLIER);
                    new_mesh->vertices.push_back(v0);
                    new_mesh->vertices.push_back(v1);
                    new_mesh->vertices.push_back(v2);
                }
                cur_index += num_face_vertices;
            }
        }
        const size_t material_count = material_to_mesh.size();
        for (size_t material_index = 0; material_index < material_count; ++material_index) {
            result_resource->meshes.push_back(material_to_mesh[static_cast<int>(material_index)]);
        }
    }

    ResourceModelInfo result;
    result.resource = result_resource;
    result.model = result_model;
    return result;
}

void ModelCache::addResource(const ResourceInfo::Ptr& resource_info) {
    if (resource_info->cached) {
        if (resource_info->resource_id == 0) {
            files_to_remove_from_resource_cache.emplace(resource_info->file_path);
            return;
        }
        if (id_to_resource_info.find(resource_info->resource_id) != id_to_resource_info.end()) {
            throw std::runtime_error(std::to_string(resource_info->resource_id) + " resource id is always present");
        }

        if (hash_to_resource_info.find(resource_info->resource_hash) != hash_to_resource_info.end()) {
            // Has a duplicate in resource cache, we will remove it later
            files_to_remove_from_resource_cache.emplace(resource_info->file_path);
            return;
        }

        id_to_resource_info.emplace(resource_info->resource_id, resource_info);
        hash_to_resource_info.emplace(resource_info->resource_hash, resource_info);
    } else {
        if (file_path_to_resource_info.find(resource_info->file_path) != file_path_to_resource_info.end()) {
            return; // File is already added
        }

        auto find_it_by_hash = hash_to_resource_info.find(resource_info->resource_hash);
        if (find_it_by_hash != hash_to_resource_info.end()) {
            // File is already added
            file_path_to_resource_info.emplace(resource_info->file_path, find_it_by_hash->second);
            return;
        }

        resource_info->resource_id = getNextResourceId();

        file_path_to_resource_info.emplace(resource_info->file_path, resource_info);
        id_to_resource_info.emplace(resource_info->resource_id, resource_info);
        hash_to_resource_info.emplace(resource_info->resource_hash, resource_info);
    }
}

std::uint16_t ModelCache::getNextResourceId() {
    while (id_to_resource_info.find(next_resource_id) != id_to_resource_info.end()) {
        ++next_resource_id;
        if (next_resource_id == 0) {
            throw std::runtime_error("resource id overflow");
        }
    }
    return next_resource_id++;
}

void ModelCache::addModel(const ModelInfo::Ptr& model_info) {
    if (model_info->cached) {
        if (model_info->model_id == 0) {
            files_to_remove_from_model_cache.emplace(model_info->file_path);
            return;
        }
        if (id_to_model_info.find(model_info->model_id) != id_to_model_info.end()) {
            throw std::runtime_error(std::to_string(model_info->model_id) + " model id is always present");
        }

        if (hash_to_model_info.find(model_info->model_hash) != hash_to_model_info.end()) {
            // Has a duplicate in model cache, we will remove it later
            files_to_remove_from_model_cache.emplace(model_info->file_path);
            return;
        }

        id_to_model_info.emplace(model_info->model_id, model_info);
        hash_to_model_info.emplace(model_info->model_hash, model_info);
    } else {
        if (file_path_to_model_info.find(model_info->file_path) != file_path_to_model_info.end()) {
            return; // File is already added
        }

        auto find_it_by_hash = hash_to_model_info.find(model_info->model_hash);
        if (find_it_by_hash != hash_to_model_info.end()) {
            // File is already added
            file_path_to_model_info.emplace(model_info->file_path, find_it_by_hash->second);
            return;
        }

        model_info->model_id = getNextModelId();

        file_path_to_model_info.emplace(model_info->file_path, model_info);
        id_to_model_info.emplace(model_info->model_id, model_info);
        hash_to_model_info.emplace(model_info->model_hash, model_info);
    }
}

std::uint16_t ModelCache::getNextModelId() {
    while (id_to_model_info.find(next_model_id) != id_to_model_info.end()) {
        ++next_model_id;
        if (next_model_id == 0) {
            throw std::runtime_error("model id overflow");
        }
    }
    return next_model_id++;
}

void ModelCache::loadCachedResources() {
    boost::filesystem::path resource_cache_path(resource_cache_dir);
    if (!boost::filesystem::exists(resource_cache_path)) {
        boost::filesystem::create_directories(resource_cache_path);
    }
    if (boost::filesystem::is_regular_file(resource_cache_path)) {
        throw std::runtime_error("resource output directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(resource_cache_path); it != end_it; ++it) {
        if (it->path().extension() == ".gkr") {
            try {
                int resource_id = std::stoi(it->path().stem().string());
                ResourceInfo::Ptr resource_info = std::make_shared<ResourceInfo>();
                resource_info->resource_id = static_cast<std::uint16_t>(resource_id);
                resource_info->file_path = it->path().string();
                resource_info->resource_hash = calcHash(resource_info->file_path);
                addResource(resource_info);
            } catch (std::invalid_argument const&) {
            } catch (std::out_of_range const&) {
            }
        }
    }
}

void ModelCache::loadCachedModels() {
    boost::filesystem::path model_cache_path(model_cache_dir);
    if (!boost::filesystem::exists(model_cache_path)) {
        boost::filesystem::create_directories(model_cache_path);
    }
    if (boost::filesystem::is_regular_file(model_cache_path)) {
        throw std::runtime_error("model output directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(model_cache_path); it != end_it; ++it) {
        if (it->path().extension() == ".gkm") {
            try {
                int model_id = std::stoi(it->path().stem().string());
                ModelInfo::Ptr model_info = std::make_shared<ModelInfo>();
                model_info->model_id = static_cast<std::uint16_t>(model_id);
                model_info->file_path = it->path().string();
                model_info->model_hash = calcHash(model_info->file_path);
                addModel(model_info);
            } catch (std::invalid_argument const&) {
            } catch (std::out_of_range const&) {
            }
        }
    }
}

ModelCache::ModelCache(TextureCache::Ptr texture_cache_, const std::string& resource_cache_dir_, const std::string& model_cache_dir_)
    : texture_cache(texture_cache_), resource_cache_dir(resource_cache_dir_), model_cache_dir(model_cache_dir_) {
    loadCachedResources();
    loadCachedModels();
}

void ModelCache::loadNewModels(const std::string& input_model_dir) {
    boost::filesystem::path input_model_path(input_model_dir);
    if (!boost::filesystem::exists(input_model_path)) {
        throw std::runtime_error("input model directory is not exist");
    }
    if (boost::filesystem::is_regular_file(input_model_path)) {
        throw std::runtime_error("input model directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(input_model_path); it != end_it; ++it) {
        if (it->path().extension() == ".obj") {
            ResourceInfo::Ptr new_resource_info = std::make_shared<ResourceInfo>();
            new_resource_info->cached = false;
            new_resource_info->file_path = it->path().string();
            auto new_resources = loadResource(new_resource_info->file_path);
            new_resource_info->resource_data = GkmModelRev0::saveResource(new_resources.resource);
            new_resource_info->resource_hash = calcHash(new_resource_info->resource_data);
            addResource(new_resource_info);
            ModelInfo::Ptr new_model_info = std::make_shared<ModelInfo>();
            new_model_info->cached = false;
            new_model_info->file_path = it->path().string();
            new_resources.model->resource_id = new_resource_info->resource_id;
            new_model_info->model_data = GkmModelRev0::saveModel(new_resources.model);
            new_model_info->model_hash = calcHash(new_model_info->model_data);
            addModel(new_model_info);
        }
    }
}

void ModelCache::updateCache() {
    for (auto file_to_remove : files_to_remove_from_resource_cache) {
        std::cout << "removing " << file_to_remove << " as duplicated" << std::endl;
        boost::filesystem::remove(file_to_remove);
    }
    for (auto resource_info : file_path_to_resource_info) {
        if (!resource_info.second->cached) {
            std::stringstream new_file_name_in_cache;
            new_file_name_in_cache << resource_cache_dir << "/" << std::setw(8) << std::setfill('0') << resource_info.second->resource_id << ".gkr";
            std::cout << "copying " << resource_info.second->file_path << " => " << new_file_name_in_cache.str() << std::endl;
            save(new_file_name_in_cache.str(), resource_info.second->resource_data);
        }
    }
    for (auto file_to_remove : files_to_remove_from_model_cache) {
        std::cout << "removing " << file_to_remove << " as duplicated" << std::endl;
        boost::filesystem::remove(file_to_remove);
    }
    for (auto model_info : file_path_to_model_info) {
        if (!model_info.second->cached) {
            std::stringstream new_file_name_in_cache;
            new_file_name_in_cache << model_cache_dir << "/" << std::setw(8) << std::setfill('0') << model_info.second->model_id << ".gkm";
            std::cout << "copying " << model_info.second->file_path << " => " << new_file_name_in_cache.str() << std::endl;
            save(new_file_name_in_cache.str(), model_info.second->model_data);
        }
    }
}
