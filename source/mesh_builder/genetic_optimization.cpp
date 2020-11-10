// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "genetic_optimization.h"

Pair GeneticOptimization::getRandomPair() const
{
    return std::make_pair(nullptr, nullptr);
}

Pair GeneticOptimization::mate(const Pair& pair) const
{
    return std::make_pair(nullptr, nullptr);
}

void GeneticOptimization::mutate(const Individual::Ptr& individual) const
{
}

GeneticOptimization::GeneticOptimization(const std::vector<TriangleTexture::Ptr>& triangle_textures, const Mesh::Ptr& mesh) : engine(std::random_device()())
{
}

void GeneticOptimization::calculatePenalty(const Individual::Ptr& individual) const
{
}

void GeneticOptimization::calculatePenalties() const
{
}

void GeneticOptimization::sort()
{
}

void GeneticOptimization::nextGeneration()
{
}

Individual::Ptr GeneticOptimization::getBest() const
{
    return nullptr;
}
