// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <vector>
#include <random>
#include <boost/polygon/polygon.hpp>
#include "texture_atlas.h"

typedef boost::polygon::point_data<int> NfpPoint;
typedef boost::polygon::rectangle_data<int> NfpRectange;
typedef boost::polygon::polygon_set_data<int> NfpPolygonSet;
typedef boost::polygon::polygon_with_holes_data<int> NfpPolygon;
typedef std::pair<NfpPoint, NfpPoint> NfpEdge;

struct Gene
{
    size_t triangle_texture_index;
    size_t rotation_index;
};

struct Individual
{
    typedef std::shared_ptr<Individual> Ptr;

    std::vector<Gene> genotype;
    size_t penalty = 0;
};
typedef std::pair<Individual::Ptr, Individual::Ptr> Pair;

class GeneticOptimization
{
    std::list<Individual::Ptr> population;
    mutable std::mt19937 engine;
    mutable std::uniform_int_distribution<size_t> uniform;

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
