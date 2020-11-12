// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <array>
#include <vector>
#include <random>
#include <boost/polygon/polygon.hpp>
#include "texture_atlas.h"

typedef boost::polygon::point_data<int> NfpPoint;
typedef boost::polygon::rectangle_data<int> NfpRectange;
typedef boost::polygon::polygon_set_data<int> NfpPolygonSet;
typedef boost::polygon::polygon_data<int> NfpPolygon;
typedef std::pair<NfpPoint, NfpPoint> NfpEdge;

typedef std::shared_ptr<NfpPolygonSet> NfpPolygonSetPtr;

constexpr static double PROTECTION_OFFSET = 8.0;
constexpr static double SCALE = 256.0;
constexpr static size_t ROTATION_COUNT = 8;
constexpr static size_t POPULATION_SIZE = 128;
constexpr static size_t MUTATION_RATE = 10;

struct TriangleTextureVariation
{
    typedef std::shared_ptr<TriangleTextureVariation> Ptr;

    NfpPolygonSet polygon;
    double rotation_angle;
};

struct TriangleTextureInformation
{
    typedef std::shared_ptr<TriangleTextureInformation> Ptr;

    std::vector<TriangleTextureVariation::Ptr> variations;
};

struct Gene
{
    size_t triangle_texture_index;
    size_t rotation_index;
    NfpPoint placement;
    bool placed = false;
};

struct Individual
{
    typedef std::shared_ptr<Individual> Ptr;

    std::vector<Gene> genotype;
    size_t penalty = 0;
};
typedef std::array<Individual::Ptr, 2> Pair;

struct OuterNfp
{
    NfpPolygonSetPtr result;
    int effective_protection_offset = 0;
};

class GeneticOptimization
{
    std::vector<TriangleTextureInformation::Ptr> triangle_texture_information;
    std::list<Individual::Ptr> population;
    std::map<std::pair<NfpPolygonSet*, NfpPolygonSet*>, OuterNfp> outer_nfp_cache;

    mutable std::mt19937 engine;
    mutable std::uniform_int_distribution<size_t> uniform;

    const NfpPolygonSet& cachedOuterNfp(const NfpPolygonSetPtr& a, const NfpPolygonSetPtr& b, int effective_protection_offset);
    Pair getRandomPair() const;
    Pair mate(const Pair& pair) const;
    void mutate(const Individual::Ptr& individual) const;

public:
    typedef std::shared_ptr<GeneticOptimization> Ptr;

    GeneticOptimization(const std::vector<TriangleTexture::Ptr>& triangle_textures, const Mesh::Ptr& mesh);
    void calculatePenalty(const Individual::Ptr& individual) const;
    void calculatePenalties() const;
    void sort();
    void nextGeneration();
    Individual::Ptr getBest() const;
};
