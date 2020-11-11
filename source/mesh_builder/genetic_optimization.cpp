// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <unordered_set>
#include "genetic_optimization.h"

static inline void toPolygon(const TriangleTexture::Ptr& triangle_texture, NfpPolygon& polygon)
{
    std::vector<NfpPoint> points;
    points.reserve(3);
    for (unsigned i = 0; i < 3; ++i)
    {
        const int x = static_cast<int>(SCALE * triangle_texture->texture_coordinates[i].x() * triangle_texture->getWidth());
        const int y = static_cast<int>(SCALE * triangle_texture->texture_coordinates[i].y() * triangle_texture->getHeight());
        points.push_back(NfpPoint(x, y));
    }
    boost::polygon::set_points(polygon, points.begin(), points.end());
}

static inline void toPolygonSet(const TriangleTexture::Ptr& triangle_texture, NfpPolygonSet& polygons)
{
    polygons.clear();
    std::vector<NfpPoint> points;
    points.reserve(3);
    for (unsigned i = 0; i < 3; ++i)
    {
        const int x = static_cast<int>(SCALE * triangle_texture->texture_coordinates[i].x() * triangle_texture->getWidth());
        const int y = static_cast<int>(SCALE * triangle_texture->texture_coordinates[i].y() * triangle_texture->getHeight());
        points.push_back(NfpPoint(x, y));
    }
    NfpPolygon polygon;
    boost::polygon::set_points(polygon, points.begin(), points.end());
    using namespace boost::polygon::operators;
    polygons += polygon;
}

static inline void convolveTwoSegments(std::vector<NfpPoint>& figure, const NfpEdge& a, const NfpEdge& b)
{
    using namespace boost::polygon;
    figure.clear();
    figure.reserve(4);
    figure.push_back(NfpPoint(a.first));
    figure.push_back(NfpPoint(a.first));
    figure.push_back(NfpPoint(a.second));
    figure.push_back(NfpPoint(a.second));
    convolve(figure[0], b.second);
    convolve(figure[1], b.first);
    convolve(figure[2], b.first);
    convolve(figure[3], b.second);
}

template <typename Iterator1, typename Iterator2>
static inline void convolveTwoPointSequences(NfpPolygonSet& result, Iterator1 ab, Iterator1 ae, Iterator2 bb, Iterator2 be)
{
    if (ab == ae || bb == be)
        return;

    NfpPoint first_a = *ab;
    NfpPoint prev_a = *ab;
    std::vector<NfpPoint> vec;
    NfpPolygon poly;
    ++ab;
    for (; ab != ae; ++ab)
    {
        NfpPoint first_b = *bb;
        NfpPoint prev_b = *bb;
        Iterator2 tmpb = bb;
        ++tmpb;
        for (; tmpb != be; ++tmpb)
        {
            convolveTwoSegments(vec, std::make_pair(prev_b, *tmpb), std::make_pair(prev_a, *ab));
            set_points(poly, vec.begin(), vec.end());
            result.insert(poly);
            prev_b = *tmpb;
        }
        prev_a = *ab;
    }
}

template <typename Iterator>
static inline void convolvePointSequenceWithPolygon(NfpPolygonSet& result, Iterator b, Iterator e, const NfpPolygon& polygon)
{
    using namespace boost::polygon;
    convolveTwoPointSequences(result, b, e, begin_points(polygon), end_points(polygon));
    for (polygon_with_holes_traits<NfpPolygon>::iterator_holes_type itrh = begin_holes(polygon);
        itrh != end_holes(polygon); ++itrh)
    {
        convolveTwoPointSequences(result, b, e, begin_points(*itrh), end_points(*itrh));
    }
}

template <typename Iterator>
static inline void convolvePointSequenceWithPolygons(NfpPolygonSet& result, Iterator b, Iterator e, const std::vector<NfpPolygon>& polygons)
{
    using namespace boost::polygon;
    for (size_t i = 0; i < polygons.size(); ++i)
    {
        convolveTwoPointSequences(result, b, e, begin_points(polygons[i]), end_points(polygons[i]));
        for (polygon_with_holes_traits<NfpPolygon>::iterator_holes_type itrh = begin_holes(polygons[i]);
            itrh != end_holes(polygons[i]); ++itrh)
        {
            convolveTwoPointSequences(result, b, e, begin_points(*itrh), end_points(*itrh));
        }
    }
}

static inline void convolveTwoPolygons(NfpPolygonSet& result, const NfpPolygon& a, NfpPolygon b)
{
    using namespace boost::polygon;
    result.clear();

    scale(b, -1.0); // To detect NFP we need negative coordinates of polygon b

    convolvePointSequenceWithPolygon(result, begin_points(a), end_points(a), b);
    for (polygon_with_holes_traits<NfpPolygon>::iterator_holes_type itrh = begin_holes(a);
        itrh != end_holes(a); ++itrh)
    {
        convolvePointSequenceWithPolygon(result, begin_points(*itrh),
            end_points(*itrh), b);
    }

    NfpPolygon tmp_poly = a;
    result.insert(convolve(tmp_poly, *(begin_points(b))));
    tmp_poly = b;
    result.insert(convolve(tmp_poly, *(begin_points(a))));
}

static inline void convolveTwoPolygonSets(NfpPolygonSet& result, const NfpPolygonSet& a, const NfpPolygonSet& b)
{
    using namespace boost::polygon;
    result.clear();
    std::vector<NfpPolygon> a_polygons;
    std::vector<NfpPolygon> b_polygons;
    a.get(a_polygons);
    b.get(b_polygons);

    for (size_t i = 0; i < b_polygons.size(); ++i)
    {
        scale(b_polygons[i], -1.0); // To detect NFP we need negative coordinates of polygon b
    }

    for (size_t ai = 0; ai < a_polygons.size(); ++ai)
    {
        convolvePointSequenceWithPolygons(result, begin_points(a_polygons[ai]),
            end_points(a_polygons[ai]), b_polygons);
        for (polygon_with_holes_traits<NfpPolygon>::iterator_holes_type itrh = begin_holes(a_polygons[ai]);
            itrh != end_holes(a_polygons[ai]); ++itrh)
        {
            convolvePointSequenceWithPolygons(result, begin_points(*itrh),
                end_points(*itrh), b_polygons);
        }
        for (size_t bi = 0; bi < b_polygons.size(); ++bi)
        {
            NfpPolygon tmp_poly = a_polygons[ai];
            result.insert(convolve(tmp_poly, *(begin_points(b_polygons[bi]))));
            tmp_poly = b_polygons[bi];
            result.insert(convolve(tmp_poly, *(begin_points(a_polygons[ai]))));
        }
    }
}

static inline const NfpPolygonSet& cachedOuterNfp(const NfpPolygonSetPtr& a, const NfpPolygonSetPtr& b, int effective_protection_offset)
{
    struct OuterNfp
    {
        NfpPolygonSetPtr result;
        int effective_protection_offset = 0.0;
    };
    static std::map<std::pair<NfpPolygonSet*, NfpPolygonSet*>, OuterNfp> outer_nfp_cache;

    auto key = std::make_pair(a.get(), b.get());
    auto fit = outer_nfp_cache.find(key);
    if (fit == outer_nfp_cache.end())
    {
        auto outer_nfp = std::make_shared<NfpPolygonSet>();

        if (effective_protection_offset > 0)
        {
            NfpPolygonSet a_for_bloating = *a;
            a_for_bloating.bloat(effective_protection_offset);
            convolveTwoPolygonSets(*outer_nfp, a_for_bloating, *b);
        }
        else
        {
            convolveTwoPolygonSets(*outer_nfp, *a, *b);
        }

        OuterNfp outer_nfp_info;
        outer_nfp_info.result = outer_nfp;
        outer_nfp_info.effective_protection_offset = effective_protection_offset;

        fit = outer_nfp_cache.emplace(key, outer_nfp_info).first;
    }
    if (fit->second.effective_protection_offset != effective_protection_offset)
    {
        assert(false);
        throw std::runtime_error("different effective protection offset!");
    }

    return *fit->second.result;
}

Pair GeneticOptimization::getRandomPair() const
{
    const size_t max_random = POPULATION_SIZE * POPULATION_SIZE;
    std::vector<Individual::Ptr> result;
    result.reserve(2);
    while (result.size() < 2)
    {
        size_t index = 0;
        for (auto individual : population)
        {
            if (result.empty() || *result.begin() != individual)
            {
                if (uniform(engine) % max_random < 2 * (POPULATION_SIZE - index))
                {
                    result.push_back(individual);
                    if (result.size() >= 2)
                    {
                        goto finish; // Yep! I'm using goto statement!
                    }
                }
            }
            ++index;
        }
    }
finish:
    return std::make_pair(result[0], result[1]);
}

Pair GeneticOptimization::mate(const Pair& pair) const
{
    const Individual::Ptr& male = pair.first;
    const Individual::Ptr& female = pair.first;

    const size_t genes_count = male->genotype.size();
    const size_t genes_cross_area_size = genes_count * 80 / 10; // 80%
    const size_t genes_cross_area_base = genes_count * 10 / 10; // 10%
    size_t cross_point = (uniform(engine) % genes_cross_area_size + genes_cross_area_base) / 10;

    auto male_based_child = std::make_shared<Individual>();
    auto female_based_child = std::make_shared<Individual>();
    male_based_child->genotype.reserve(male->genotype.size());
    female_based_child->genotype.reserve(female->genotype.size());
    std::unordered_set<size_t> male_based_triangles;
    std::unordered_set<size_t> female_based_triangles;

    for (size_t i = 0; i < cross_point; ++i)
    {
        const Gene& male_gene = male->genotype[i];
        male_based_child->genotype.push_back(male_gene);
        male_based_triangles.insert(male_gene.triangle_texture_index);

        const Gene& female_gene = female->genotype[i];
        female_based_child->genotype.push_back(female_gene);
        female_based_triangles.insert(female_gene.triangle_texture_index);
    }

    for (size_t i = 0; i < female->genotype.size(); ++i)
    {
        const Gene& female_gene = female->genotype[i];
        if (male_based_triangles.find(female_gene.triangle_texture_index) == male_based_triangles.end())
        {
            male_based_child->genotype.push_back(female_gene);
            male_based_triangles.insert(female_gene.triangle_texture_index);
        }
    }
    for (size_t i = 0; i < male->genotype.size(); ++i)
    {
        const Gene& male_gene = male->genotype[i];
        if (female_based_triangles.find(male_gene.triangle_texture_index) == female_based_triangles.end())
        {
            female_based_child->genotype.push_back(male_gene);
            female_based_triangles.insert(male_gene.triangle_texture_index);
        }
    }

    return std::make_pair(female_based_child, male_based_child);
}

void GeneticOptimization::mutate(const Individual::Ptr& individual) const
{
    for (size_t i = 0; i < individual->genotype.size(); ++i)
    {
        if (uniform(engine) % 100 < MUTATION_RATE)
        {
            Gene& gene = individual->genotype[i];
            gene.rotation_index = uniform(engine) % gene.max_rotation_index;
        }
        if (uniform(engine) % 100 < MUTATION_RATE)
        {
            size_t j = i + 1;
            if (j >= individual->genotype.size())
            {
                j = 0;
            }
            std::swap(individual->genotype[i], individual->genotype[j]);
        }
    }
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
