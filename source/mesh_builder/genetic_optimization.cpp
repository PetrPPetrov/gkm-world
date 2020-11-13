// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <unordered_set>
#include "global_parameters.h"
#include "genetic_optimization.h"

template<typename ProcessPointFunc>
static inline void iterateAllPoints(const NfpPolygon& polygon, ProcessPointFunc& process_point)
{
    using namespace boost::polygon;
    for (auto& point_it = begin_points(polygon); point_it != end_points(polygon); ++point_it)
    {
        process_point(*point_it);
    }
    for (polygon_with_holes_traits<NfpPolygon>::iterator_holes_type it = begin_holes(polygon);
        it != end_holes(polygon); ++it)
    {
        for (auto& point_it = begin_points(*it); point_it != end_points(*it); ++point_it)
        {
            process_point(*point_it);
        }
    }
}

template<typename ProcessPointFunc>
static inline void iterateAllPoints(const NfpPolygonSet& polygon_set, ProcessPointFunc& process_point)
{
    std::vector<NfpPolygon> polygons;
    polygon_set.get(polygons);
    for (auto& polygon : polygons)
    {
        iterateAllPoints(polygon, process_point);
    }
}

static inline void toPolygonSet(const TriangleTexture::Ptr& triangle_texture, double rotation_angle, NfpPolygonSet& polygons)
{
    polygons.clear();
    std::vector<NfpPoint> points;
    points.reserve(3);
    for (unsigned i = 0; i < 3; ++i)
    {
        const Eigen::Vector2d tex_coord = Eigen::Rotation2Dd(rotation_angle) * triangle_texture->texture_coordinates[i];
        const int x = static_cast<int>(SCALE * tex_coord.x());
        const int y = static_cast<int>(SCALE * tex_coord.y());
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

const NfpPolygonSet& GeneticOptimization::cachedOuterNfp(const NfpPolygonSetPtr& a, const NfpPolygonSetPtr& b, int effective_protection_offset)
{
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
    Pair result = { nullptr };
    unsigned result_index = 0;
    while (result.size() < 2)
    {
        size_t index = 0;
        for (auto individual : population)
        {
            if (result.empty() || *result.begin() != individual)
            {
                if (uniform(engine) % max_random < 2 * (POPULATION_SIZE - index))
                {
                    result[result_index++] = individual;
                    if (result_index >= 2)
                    {
                        return result;
                    }
                }
            }
            ++index;
        }
    }
    return result;
}

Pair GeneticOptimization::mate(const Pair& pair) const
{
    const Individual::Ptr& male = pair[0];
    const Individual::Ptr& female = pair[1];

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

    return { female_based_child, male_based_child };
}

void GeneticOptimization::mutate(const Individual::Ptr& individual) const
{
    for (size_t i = 0; i < individual->genotype.size(); ++i)
    {
        if (uniform(engine) % 100 < MUTATION_RATE)
        {
            Gene& gene = individual->genotype[i];
            gene.rotation_index = uniform(engine) % ROTATION_COUNT;
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
    triangle_texture_information.reserve(triangle_textures.size());

    const double rotation_step = 360.0 / ROTATION_COUNT * GRAD_TO_RAD;
    for (size_t i = 0; i < triangle_textures.size(); ++i)
    {
        auto new_information = std::make_shared<TriangleTextureInformation>();
        triangle_texture_information.push_back(new_information);
        new_information->variations.reserve(ROTATION_COUNT);

        for (size_t j = 0; j < ROTATION_COUNT; ++j)
        {
            auto new_variation = std::make_shared<TriangleTextureVariation>();
            new_information->variations.push_back(new_variation);
            new_variation->rotation_angle = rotation_step * j;
            new_variation->polygon = std::make_shared<NfpPolygonSet>();
            toPolygonSet(triangle_textures[i], new_variation->rotation_angle, *new_variation->polygon);
        }
    }

    std::vector<TriangleTexture::Ptr> sorted_triangle_textures = triangle_textures; // Perform copy
    std::sort(sorted_triangle_textures.begin(), sorted_triangle_textures.end(), [](const TriangleTexture::Ptr& a, const TriangleTexture::Ptr& b)
        {
            return a->area > b->area;
        });

    auto adam = std::make_shared<Individual>();
    adam->genotype.reserve(sorted_triangle_textures.size());
    for (auto& triangle_info : sorted_triangle_textures)
    {
        Gene new_gene;
        new_gene.triangle_texture_index = triangle_info->getTriangleIndex();
        new_gene.rotation_index = uniform(engine) % ROTATION_COUNT;
        adam->genotype.push_back(new_gene);
    }
    population.push_back(adam);

    while (population.size() < POPULATION_SIZE)
    {
        auto eva = std::make_shared<Individual>();
        *eva = *adam;
        mutate(eva);
        population.push_back(eva);
    }
}

void GeneticOptimization::calculatePenalty(const Individual::Ptr& individual)
{
    if (individual->penalty)
    {
        // If this individual already contains some calculated
        // penalty then use the calculated penalty.
        // This happens for best individual who goes
        // to the next generation without any mutations.
        return;
    }

    using namespace boost::polygon;
    using namespace boost::polygon::operators;

    bool first_iteration = true;
    NfpPolygonSet accumulated_geometry;
    for (size_t gene_index = 0; gene_index < individual->genotype.size(); ++gene_index)
    {
        Gene& gene = individual->genotype[gene_index];
        gene.placed = false;
        const NfpPolygonSetPtr& cur_gene_geometry = triangle_texture_information[gene.triangle_texture_index]->variations[gene.rotation_index]->polygon;
        std::vector<NfpPolygon> cur_gene_polygons;
        cur_gene_geometry->get(cur_gene_polygons);

        if (first_iteration)
        {
            // Place the first triangle texture at (0,0) position
            gene.placement = NfpPoint(0, 0);
            gene.placed = true;
            first_iteration = false;
            accumulated_geometry += *cur_gene_geometry;
            continue;
        }

        NfpPolygonSet result_nfp;
        for (size_t prev_gene_index = 0; prev_gene_index < gene_index; ++prev_gene_index)
        {
            const Gene& prev_gene = individual->genotype[prev_gene_index];
            if (prev_gene.placed)
            {
                const NfpPolygonSetPtr& prev_gene_geometry = triangle_texture_information[prev_gene.triangle_texture_index]->variations[prev_gene.rotation_index]->polygon;
                const NfpPolygonSet& outer_nfp = cachedOuterNfp(prev_gene_geometry, cur_gene_geometry, EFFECTIVE_PROTECTION_OFFSET);

                std::vector<NfpPolygon> outer_nfp_polygons;
                outer_nfp.get(outer_nfp_polygons);
                for (auto& outer_nfp_polygon : outer_nfp_polygons)
                {
                    // Move outer NFP according the current triangle texture placement
                    move(outer_nfp_polygon, HORIZONTAL, x(prev_gene.placement));
                    move(outer_nfp_polygon, VERTICAL, y(prev_gene.placement));
                    result_nfp += outer_nfp_polygon; // Union of all part NFP
                }
            }
        }

        size_t min_local_penalty = 0;
        iterateAllPoints(result_nfp,
            [&min_local_penalty, &gene, &accumulated_geometry, &cur_gene_polygons](const NfpPoint& point)
            {
                NfpPolygonSet new_geometry = accumulated_geometry;
                for (auto& cur_gene_polygon : cur_gene_polygons)
                {
                    NfpPolygon new_cur_gene_polygon = cur_gene_polygon;
                    move(new_cur_gene_polygon, HORIZONTAL, x(point));
                    move(new_cur_gene_polygon, VERTICAL, y(point));
                    new_geometry += new_cur_gene_polygon;
                }
                NfpRectange bounding_box;
                extents(bounding_box, new_geometry);
                const size_t cur_local_penalty = static_cast<size_t>(area(bounding_box));

                if (!gene.placed || cur_local_penalty < min_local_penalty)
                {
                    gene.placement = point;
                    gene.placed = true;
                    min_local_penalty = cur_local_penalty;
                }
            }
        );

        assert(gene.placed);
        for (auto& cur_gene_polygon : cur_gene_polygons)
        {
            move(cur_gene_polygon, HORIZONTAL, x(gene.placement));
            move(cur_gene_polygon, VERTICAL, y(gene.placement));
            accumulated_geometry += cur_gene_polygon;
        }

        individual->penalty = min_local_penalty;
    }
}

void GeneticOptimization::calculatePenalties()
{
    for (auto& individual : population)
    {
        calculatePenalty(individual);
    }
}

void GeneticOptimization::sort()
{
    population.sort([](const Individual::Ptr& a, const Individual::Ptr& b)
        {
            return a->penalty < b->penalty;
        });
}

void GeneticOptimization::nextGeneration()
{
    std::list<Individual::Ptr> next_population;
    next_population.push_back(getBest());
    while (next_population.size() < POPULATION_SIZE)
    {
        auto pair = getRandomPair();
        Pair children = mate(pair);
        for (auto& child : children)
        {
            if (next_population.size() < POPULATION_SIZE)
            {
                mutate(child);
                next_population.push_back(child);
            }
            else
            {
                break;
            }
        }
    }
    population = next_population;
}

Individual::Ptr GeneticOptimization::getBest() const
{
    return *population.begin();
}
