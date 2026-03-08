#include "engine/simulation/simulation.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "engine/rendering/particle.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static SphSimulation make_sim() {
    SphSimulation sim;
    sim.simulation_data.BOX_START = {-5.f, -5.f, -5.f};
    sim.simulation_data.BOX_END = {5.f, 5.f, 5.f};
    sim.simulation_data.H_SMOOTHING = 1.0f;
    sim.simulation_data.DT = 0.01f;
    sim.simulation_data.GRAVITY = 9.8f;
    sim.simulation_data.GAS_CONSTANT = 1.0f;
    sim.simulation_data.REST_DENSITY = 1000.f;
    sim.simulation_data.VISCOSITY = 0.1f;
    sim.simulation_data.TENSION_COEFICIENT = 0.01f;
    sim.simulation_data.ENERGY_LOSS = 0.8f;
    sim.simulation_data.FRICTION = 0.9f;
    return sim;
}

class SphSimulationTestBase : public ::testing::Test {
   protected:
    SphSimulation sim = make_sim();
    static constexpr float R = 0.1f;

    void apply_bb(std::vector<Particle> &p, float r, float el, float fr) {
        sim.apply_bounding_box_physics(p, r, el, fr);
    }
    uint32_t expand_bits(uint32_t x) { return sim.expand_bits(x); }
    uint32_t morton_code(uint32_t x, uint32_t y, uint32_t z) { return sim.morton_code(x, y, z); }
    void z_insertion_sort(std::vector<std::pair<uint32_t, size_t>> &v) { sim.z_insertion_sort(v); }
    void z_sort_particles(std::vector<Particle> &p) { sim.z_sort_particles(p); }
    void get_neighbors(const Particle &p, std::vector<size_t> &r) { sim.get_neighbors(p, r); }

    void grid_clear() { sim.particle_grid.clear(); }
    void grid_insert(const std::tuple<int, int, int> &key, size_t idx) { sim.particle_grid[key].push_back(idx); }
    void neighbors_cache_resize(size_t n) { sim.neighbors_cache.resize(n); }
    size_t neighbors_cache_size() { return sim.neighbors_cache.size(); }
};

// ─────────────────────────────────────────────────────────────────────────────
// Bounding-box physics
// ─────────────────────────────────────────────────────────────────────────────

class BoundingBoxTest : public SphSimulationTestBase {};

TEST_F(BoundingBoxTest, FloorBounceReflectsYVelocity) {
    Particle p;
    p.pos = {0.f, -4.95f, 0.f};
    p.velocity = {0.f, -2.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_GE(particles[0].pos.y, sim.simulation_data.BOX_START.y + R);
    EXPECT_GT(particles[0].velocity.y, 0.f) << "Y velocity should flip after floor bounce";
}

TEST_F(BoundingBoxTest, FloorBouncePositionClampedExactly) {
    Particle p;
    p.pos = {0.f, -10.f, 0.f};  // well outside the box
    p.velocity = {0.f, -5.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_NEAR(particles[0].pos.y, sim.simulation_data.BOX_START.y + R, 1e-6f);
}

TEST_F(BoundingBoxTest, FloorBounceAppliesEnergyLossToYVelocity) {
    Particle p;
    p.pos = {0.f, -4.95f, 0.f};
    p.velocity = {0.f, -3.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    // After bounce: vy = -(-3) * ENERGY_LOSS = +2.4
    float expected_vy = 3.f * sim.simulation_data.ENERGY_LOSS;
    EXPECT_NEAR(particles[0].velocity.y, expected_vy, 1e-5f);
}

TEST_F(BoundingBoxTest, FloorBounceAppliesFrictionToX) {
    Particle p;
    p.pos = {0.f, -4.95f, 0.f};
    p.velocity = {4.f, -1.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    float expected_vx = 4.f * sim.simulation_data.FRICTION;
    EXPECT_NEAR(particles[0].velocity.x, expected_vx, 1e-5f);
}

TEST_F(BoundingBoxTest, FloorBounceShouldApplyFrictionToZ) {
    Particle p;
    p.pos = {0.f, -4.95f, 0.f};
    p.velocity = {0.f, -1.f, 3.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    float expected_vz = 3.f * sim.simulation_data.FRICTION;
    EXPECT_NEAR(particles[0].velocity.z, expected_vz, 1e-5f);
}

TEST_F(BoundingBoxTest, LeftWallBounce) {
    Particle p;
    p.pos = {-4.95f, 0.f, 0.f};
    p.velocity = {-3.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_GE(particles[0].pos.x, sim.simulation_data.BOX_START.x + R);
    EXPECT_GT(particles[0].velocity.x, 0.f);
}

TEST_F(BoundingBoxTest, LeftWallBounceAppliesEnergyLoss) {
    Particle p;
    p.pos = {-4.95f, 0.f, 0.f};
    p.velocity = {-4.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    float expected_vx = 4.f * sim.simulation_data.ENERGY_LOSS;
    EXPECT_NEAR(particles[0].velocity.x, expected_vx, 1e-5f);
}

TEST_F(BoundingBoxTest, RightWallBounce) {
    Particle p;
    p.pos = {4.95f, 0.f, 0.f};
    p.velocity = {3.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_LE(particles[0].pos.x, sim.simulation_data.BOX_END.x - R);
    EXPECT_LT(particles[0].velocity.x, 0.f);
}

TEST_F(BoundingBoxTest, RightWallBounceAppliesEnergyLoss) {
    Particle p;
    p.pos = {4.95f, 0.f, 0.f};
    p.velocity = {5.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    float expected_speed = 5.f * sim.simulation_data.ENERGY_LOSS;
    EXPECT_NEAR(std::abs(particles[0].velocity.x), expected_speed, 1e-5f);
}

TEST_F(BoundingBoxTest, CeilingBounce) {
    Particle p;
    p.pos = {0.f, 4.95f, 0.f};
    p.velocity = {0.f, 3.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_LE(particles[0].pos.y, sim.simulation_data.BOX_END.y - R);
    EXPECT_LT(particles[0].velocity.y, 0.f);
}

TEST_F(BoundingBoxTest, CeilingBounceAppliesEnergyLoss) {
    Particle p;
    p.pos = {0.f, 4.95f, 0.f};
    p.velocity = {0.f, 6.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    float expected_speed = 6.f * sim.simulation_data.ENERGY_LOSS;
    EXPECT_NEAR(std::abs(particles[0].velocity.y), expected_speed, 1e-5f);
}

TEST_F(BoundingBoxTest, FrontWallBounce) {
    Particle p;
    p.pos = {0.f, 0.f, -4.95f};
    p.velocity = {0.f, 0.f, -2.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_GE(particles[0].pos.z, sim.simulation_data.BOX_START.z + R);
    EXPECT_GT(particles[0].velocity.z, 0.f);
}

TEST_F(BoundingBoxTest, BackWallBounce) {
    Particle p;
    p.pos = {0.f, 0.f, 4.95f};
    p.velocity = {0.f, 0.f, 2.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_LE(particles[0].pos.z, sim.simulation_data.BOX_END.z - R);
    EXPECT_LT(particles[0].velocity.z, 0.f);
}

TEST_F(BoundingBoxTest, EnergyLossOneIsElastic) {
    Particle p;
    p.pos = {4.95f, 0.f, 0.f};
    p.velocity = {3.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, 1.0f, 1.0f);

    EXPECT_NEAR(std::abs(particles[0].velocity.x), 3.f, 1e-5f);
}

TEST_F(BoundingBoxTest, EnergyLossZeroKillsNormalVelocity) {
    Particle p;
    p.pos = {4.95f, 0.f, 0.f};
    p.velocity = {3.f, 0.f, 0.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, 0.0f, 1.0f);

    EXPECT_NEAR(particles[0].velocity.x, 0.f, 1e-5f);
}

TEST_F(BoundingBoxTest, ParticleInsideBoxUnaffected) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};
    p.velocity = {1.f, 1.f, 1.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_NEAR(particles[0].pos.x, 0.f, 1e-6f);
    EXPECT_NEAR(particles[0].pos.y, 0.f, 1e-6f);
    EXPECT_NEAR(particles[0].pos.z, 0.f, 1e-6f);
    EXPECT_NEAR(particles[0].velocity.x, 1.f, 1e-6f);
    EXPECT_NEAR(particles[0].velocity.y, 1.f, 1e-6f);
    EXPECT_NEAR(particles[0].velocity.z, 1.f, 1e-6f);
}

TEST_F(BoundingBoxTest, MultipleParticlesHandledIndependently) {
    Particle a, b;
    a.pos = {4.95f, 0.f, 0.f};
    a.velocity = {3.f, 0.f, 0.f};
    b.pos = {0.f, -4.95f, 0.f};
    b.velocity = {0.f, -2.f, 0.f};

    std::vector<Particle> particles{a, b};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_LE(particles[0].pos.x, sim.simulation_data.BOX_END.x - R);
    EXPECT_LT(particles[0].velocity.x, 0.f);

    EXPECT_GE(particles[1].pos.y, sim.simulation_data.BOX_START.y + R);
    EXPECT_GT(particles[1].velocity.y, 0.f);
}

TEST_F(BoundingBoxTest, EmptyParticleVectorDoesNotCrash) {
    std::vector<Particle> particles;
    EXPECT_NO_THROW(apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION));
}

TEST_F(BoundingBoxTest, CornerBounceShouldClampAllAxes) {
    Particle p;
    p.pos = {4.95f, -4.95f, 4.95f};
    p.velocity = {2.f, -2.f, 2.f};

    std::vector<Particle> particles{p};
    apply_bb(particles, R, sim.simulation_data.ENERGY_LOSS, sim.simulation_data.FRICTION);

    EXPECT_LE(particles[0].pos.x, sim.simulation_data.BOX_END.x - R);
    EXPECT_GE(particles[0].pos.y, sim.simulation_data.BOX_START.y + R);
    EXPECT_LE(particles[0].pos.z, sim.simulation_data.BOX_END.z - R);
    EXPECT_LT(particles[0].velocity.x, 0.f);
    EXPECT_GT(particles[0].velocity.y, 0.f);
    EXPECT_LT(particles[0].velocity.z, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Morton code / bit-expansion
// ─────────────────────────────────────────────────────────────────────────────

class MortonTest : public SphSimulationTestBase {};

TEST_F(MortonTest, ExpandBitsZeroIsZero) { EXPECT_EQ(expand_bits(0u), 0u); }

TEST_F(MortonTest, ExpandBitsOneIsOne) { EXPECT_EQ(expand_bits(1u), 1u); }

TEST_F(MortonTest, ExpandBitsTwoPlacesBitAtPosition3) {
    // 2 = 0b10. The algorithm spreads bits so each input bit i goes to output bit 3*i.
    // bit 1 of input -> bit 3 of output = 0b1000 = 8
    EXPECT_EQ(expand_bits(2u), 8u);
}

TEST_F(MortonTest, ExpandBitsThreeExpandsBothLowBits) {
    // 3 = 0b11: bit0 -> pos0 (=1), bit1 -> pos3 (=8) => 9
    EXPECT_EQ(expand_bits(3u), 9u);
}

TEST_F(MortonTest, ExpandBitsIsIdempotentOnZeroAndOne) {
    // Sanity: applying expand twice should give a different (larger spread) result
    // than the original, confirming bits are actually being moved.
    EXPECT_NE(expand_bits(2u), 2u);
    EXPECT_NE(expand_bits(3u), 3u);
}

TEST_F(MortonTest, MortonCodeOriginIsZero) { EXPECT_EQ(morton_code(0, 0, 0), 0u); }

TEST_F(MortonTest, MortonCodeXAxisInterleaved) {
    // x occupies bits 0,3,6,... so (1,0,0) -> bit 0 set = 1
    EXPECT_EQ(morton_code(1, 0, 0), 1u);
}

TEST_F(MortonTest, MortonCodeYAxisInterleaved) {
    // y occupies bits 1,4,7,... so (0,1,0) -> bit 1 set = 2
    EXPECT_EQ(morton_code(0, 1, 0), 2u);
}

TEST_F(MortonTest, MortonCodeZAxisInterleaved) {
    // z occupies bits 2,5,8,... so (0,0,1) -> bit 2 set = 4
    EXPECT_EQ(morton_code(0, 0, 1), 4u);
}

TEST_F(MortonTest, MortonCodeXYZOneEquals7) {
    // (1,1,1): bits 0,1,2 all set = 7
    EXPECT_EQ(morton_code(1, 1, 1), 7u);
}

TEST_F(MortonTest, MortonCodeIsDeterministic) {
    uint32_t a = morton_code(3, 7, 5);
    uint32_t b = morton_code(3, 7, 5);
    EXPECT_EQ(a, b);
}

TEST_F(MortonTest, MortonCodeDistinctForDifferentCoords) {
    // Each axis occupies different bit lanes so single-axis unit vectors are all distinct
    EXPECT_NE(morton_code(1, 0, 0), morton_code(0, 1, 0));
    EXPECT_NE(morton_code(1, 0, 0), morton_code(0, 0, 1));
    EXPECT_NE(morton_code(0, 1, 0), morton_code(0, 0, 1));
}

TEST_F(MortonTest, MortonCodeSwappingAxesGivesDifferentCode) {
    // (2,1,0) and (1,2,0) must differ because x and y occupy different bit lanes
    EXPECT_NE(morton_code(2, 1, 0), morton_code(1, 2, 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Z-insertion sort
// ─────────────────────────────────────────────────────────────────────────────

class ZSortTest : public SphSimulationTestBase {};

TEST_F(ZSortTest, AlreadySortedRemainsStable) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{1, 0}, {2, 1}, {3, 2}, {4, 3}};
    z_insertion_sort(vec);
    for (size_t i = 0; i + 1 < vec.size(); ++i) EXPECT_LE(vec[i].first, vec[i + 1].first);
}

TEST_F(ZSortTest, ReversedInputGetsSorted) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{9, 0}, {7, 1}, {4, 2}, {1, 3}};
    z_insertion_sort(vec);
    EXPECT_EQ(vec[0].first, 1u);
    EXPECT_EQ(vec[1].first, 4u);
    EXPECT_EQ(vec[2].first, 7u);
    EXPECT_EQ(vec[3].first, 9u);
}

TEST_F(ZSortTest, RandomOrderGetsSorted) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{5, 0}, {3, 1}, {8, 2}, {1, 3}, {6, 4}};
    z_insertion_sort(vec);
    for (size_t i = 0; i + 1 < vec.size(); ++i) EXPECT_LE(vec[i].first, vec[i + 1].first);
}

TEST_F(ZSortTest, EmptyVectorDoesNotCrash) {
    std::vector<std::pair<uint32_t, size_t>> vec;
    EXPECT_NO_THROW(z_insertion_sort(vec));
}

TEST_F(ZSortTest, SingleElementUnchanged) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{42, 0}};
    z_insertion_sort(vec);
    EXPECT_EQ(vec[0].first, 42u);
}

TEST_F(ZSortTest, DuplicateKeysAllPresent) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{5, 0}, {5, 1}, {5, 2}};
    z_insertion_sort(vec);
    EXPECT_EQ(vec.size(), 3u);
    for (auto &kv : vec) EXPECT_EQ(kv.first, 5u);
}

TEST_F(ZSortTest, IndexValuesPreservedAfterSort) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{9, 10}, {3, 20}, {6, 30}};
    z_insertion_sort(vec);
    EXPECT_EQ(vec[0].second, 20u);
    EXPECT_EQ(vec[1].second, 30u);
    EXPECT_EQ(vec[2].second, 10u);
}

TEST_F(ZSortTest, LargeVectorGetsSorted) {
    std::vector<std::pair<uint32_t, size_t>> vec;
    for (uint32_t i = 100; i > 0; --i) vec.push_back({i, i});
    z_insertion_sort(vec);
    for (size_t i = 0; i + 1 < vec.size(); ++i) EXPECT_LE(vec[i].first, vec[i + 1].first);
}

TEST_F(ZSortTest, TwoElementsOutOfOrderGetSorted) {
    std::vector<std::pair<uint32_t, size_t>> vec = {{10, 0}, {2, 1}};
    z_insertion_sort(vec);
    EXPECT_EQ(vec[0].first, 2u);
    EXPECT_EQ(vec[1].first, 10u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Z-sort particles
// ─────────────────────────────────────────────────────────────────────────────

class ZSortParticlesTest : public SphSimulationTestBase {};

TEST_F(ZSortParticlesTest, SortPreservesParticleCount) {
    Particle a, b, c;
    a.pos = {1.f, 2.f, 3.f};
    b.pos = {3.f, 1.f, 2.f};
    c.pos = {2.f, 3.f, 1.f};
    std::vector<Particle> particles{a, b, c};

    size_t before = particles.size();
    z_sort_particles(particles);
    EXPECT_EQ(particles.size(), before);
}

TEST_F(ZSortParticlesTest, SortDoesNotCrashWithOneParticle) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};
    std::vector<Particle> particles{p};
    EXPECT_NO_THROW(z_sort_particles(particles));
}

TEST_F(ZSortParticlesTest, SortDoesNotCrashWithEmptyVector) {
    std::vector<Particle> particles;
    EXPECT_NO_THROW(z_sort_particles(particles));
}

TEST_F(ZSortParticlesTest, SortPreservesAllPositions) {
    // Every position present before the sort must still be present after.
    Particle a, b, c;
    a.pos = {1.f, 0.f, 0.f};
    b.pos = {0.f, 1.f, 0.f};
    c.pos = {0.f, 0.f, 1.f};
    std::vector<Particle> particles{a, b, c};

    z_sort_particles(particles);

    auto has_pos = [&](glm::vec3 target) {
        return std::any_of(particles.begin(), particles.end(), [&](const Particle &p) {
            return std::abs(p.pos.x - target.x) < 1e-5f && std::abs(p.pos.y - target.y) < 1e-5f &&
                   std::abs(p.pos.z - target.z) < 1e-5f;
        });
    };
    EXPECT_TRUE(has_pos({1.f, 0.f, 0.f}));
    EXPECT_TRUE(has_pos({0.f, 1.f, 0.f}));
    EXPECT_TRUE(has_pos({0.f, 0.f, 1.f}));
}

TEST_F(ZSortParticlesTest, ParticlesAtBoxCornersDontCrash) {
    Particle a, b;
    a.pos = sim.simulation_data.BOX_START;
    b.pos = sim.simulation_data.BOX_END;
    std::vector<Particle> particles{a, b};
    EXPECT_NO_THROW(z_sort_particles(particles));
    EXPECT_EQ(particles.size(), 2u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Neighbour lookup
// ─────────────────────────────────────────────────────────────────────────────

class NeighborTest : public SphSimulationTestBase {};

TEST_F(NeighborTest, ParticleFindsItself) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};
    std::vector<Particle> particles{p};

    grid_clear();
    auto grid_pos = std::make_tuple(static_cast<int>(std::floor(particles[0].pos.x / sim.simulation_data.H_SMOOTHING)),
                                    static_cast<int>(std::floor(particles[0].pos.y / sim.simulation_data.H_SMOOTHING)),
                                    static_cast<int>(std::floor(particles[0].pos.z / sim.simulation_data.H_SMOOTHING)));
    grid_insert(grid_pos, 0);

    std::vector<size_t> result;
    get_neighbors(particles[0], result);

    EXPECT_NE(std::find(result.begin(), result.end(), 0u), result.end());
}

TEST_F(NeighborTest, DistantParticleNotFound) {
    Particle p0, p1;
    p0.pos = {0.f, 0.f, 0.f};
    p1.pos = {100.f, 100.f, 100.f};
    std::vector<Particle> particles{p0, p1};

    grid_clear();
    for (size_t i = 0; i < particles.size(); ++i) {
        auto gp = std::make_tuple(static_cast<int>(std::floor(particles[i].pos.x / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.y / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.z / sim.simulation_data.H_SMOOTHING)));
        grid_insert(gp, i);
    }

    std::vector<size_t> result;
    get_neighbors(particles[0], result);

    EXPECT_EQ(std::find(result.begin(), result.end(), 1u), result.end());
}

TEST_F(NeighborTest, AdjacentCellParticleIsFound) {
    Particle p0, p1;
    p0.pos = {0.5f, 0.5f, 0.5f};
    p1.pos = {1.2f, 0.5f, 0.5f};
    std::vector<Particle> particles{p0, p1};

    grid_clear();
    for (size_t i = 0; i < particles.size(); ++i) {
        auto gp = std::make_tuple(static_cast<int>(std::floor(particles[i].pos.x / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.y / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.z / sim.simulation_data.H_SMOOTHING)));
        grid_insert(gp, i);
    }

    std::vector<size_t> result;
    get_neighbors(particles[0], result);

    EXPECT_NE(std::find(result.begin(), result.end(), 1u), result.end());
}

TEST_F(NeighborTest, SameCellMultipleParticlesAllFound) {
    Particle p0, p1, p2;
    p0.pos = {0.1f, 0.1f, 0.1f};
    p1.pos = {0.2f, 0.2f, 0.2f};
    p2.pos = {0.3f, 0.3f, 0.3f};
    std::vector<Particle> particles{p0, p1, p2};

    grid_clear();
    for (size_t i = 0; i < particles.size(); ++i) {
        auto gp = std::make_tuple(static_cast<int>(std::floor(particles[i].pos.x / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.y / sim.simulation_data.H_SMOOTHING)),
                                  static_cast<int>(std::floor(particles[i].pos.z / sim.simulation_data.H_SMOOTHING)));
        grid_insert(gp, i);
    }

    std::vector<size_t> result;
    get_neighbors(particles[0], result);

    EXPECT_NE(std::find(result.begin(), result.end(), 1u), result.end());
    EXPECT_NE(std::find(result.begin(), result.end(), 2u), result.end());
}

TEST_F(NeighborTest, EmptyGridReturnsNoNeighbors) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};

    grid_clear();

    std::vector<size_t> result;
    get_neighbors(p, result);

    EXPECT_TRUE(result.empty());
}

TEST_F(NeighborTest, ResultClearedBetweenCalls) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};

    grid_clear();
    auto gp = std::make_tuple(0, 0, 0);
    grid_insert(gp, 0);

    std::vector<size_t> result;
    get_neighbors(p, result);
    size_t first_count = result.size();

    // Call again — result should be the same, not doubled
    get_neighbors(p, result);
    EXPECT_EQ(result.size(), first_count);
}

TEST_F(NeighborTest, DiagonalCellParticleIsFound) {
    // p1 is one cell diagonally away in all three axes
    Particle p0, p1;
    p0.pos = {0.5f, 0.5f, 0.5f};
    p1.pos = {1.6f, 1.6f, 1.6f};

    grid_clear();
    grid_insert(std::make_tuple(0, 0, 0), 0);
    grid_insert(std::make_tuple(1, 1, 1), 1);

    std::vector<size_t> result;
    get_neighbors(p0, result);

    EXPECT_NE(std::find(result.begin(), result.end(), 1u), result.end());
}

TEST_F(NeighborTest, TwoCellsAwayNotFound) {
    Particle p0, p1;
    p0.pos = {0.5f, 0.5f, 0.5f};
    p1.pos = {2.5f, 0.5f, 0.5f};  // 2 cells away in X

    grid_clear();
    grid_insert(std::make_tuple(0, 0, 0), 0);
    grid_insert(std::make_tuple(2, 0, 0), 1);

    std::vector<size_t> result;
    get_neighbors(p0, result);

    EXPECT_EQ(std::find(result.begin(), result.end(), 1u), result.end());
}

// ─────────────────────────────────────────────────────────────────────────────
// Regenerate particles
// ─────────────────────────────────────────────────────────────────────────────

class RegenerateTest : public SphSimulationTestBase {};

TEST_F(RegenerateTest, ProducesNonZeroParticles) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.5f);
    EXPECT_GT(particles.size(), 0u);
}

TEST_F(RegenerateTest, ClearsExistingParticles) {
    // Pre-load with sentinel particles that are way outside any valid spawn region.
    // After regenerate the vector must not contain them.
    Particle sentinel;
    sentinel.pos = {999.f, 999.f, 999.f};
    std::vector<Particle> particles{sentinel, sentinel};

    sim.regenerate_particles(particles, 0.5f);

    bool found_sentinel =
        std::any_of(particles.begin(), particles.end(), [](const Particle &p) { return p.pos.x > 100.f; });
    EXPECT_FALSE(found_sentinel) << "regenerate_particles should replace, not append to, the particle list";
}

TEST_F(RegenerateTest, NeighborsCacheResized) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.5f);
    EXPECT_EQ(neighbors_cache_size(), particles.size());
}

TEST_F(RegenerateTest, SmallerSpacingProducesMoreParticles) {
    std::vector<Particle> coarse, fine;
    sim.regenerate_particles(coarse, 1.0f);
    sim.regenerate_particles(fine, 0.5f);
    EXPECT_GT(fine.size(), coarse.size());
}

TEST_F(RegenerateTest, ParticlesSpawnedInsideBox) {
    // Spawn region is a strict sub-volume of the box so all particles, even
    // with jitter (±0.15 * spacing), should remain well within box bounds.
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.5f);

    for (auto &p : particles) {
        EXPECT_GE(p.pos.x, sim.simulation_data.BOX_START.x);
        EXPECT_LE(p.pos.x, sim.simulation_data.BOX_END.x);
        EXPECT_GE(p.pos.y, sim.simulation_data.BOX_START.y);
        EXPECT_LE(p.pos.y, sim.simulation_data.BOX_END.y);
        EXPECT_GE(p.pos.z, sim.simulation_data.BOX_START.z);
        EXPECT_LE(p.pos.z, sim.simulation_data.BOX_END.z);
    }
}

TEST_F(RegenerateTest, RegenerateCalledTwiceReplacesNotAppends) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 1.0f);
    size_t first_count = particles.size();

    sim.regenerate_particles(particles, 1.0f);
    EXPECT_EQ(particles.size(), first_count) << "Second call should replace the particle list, not append to it";
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration smoke test
// ─────────────────────────────────────────────────────────────────────────────

class UpdateParticlesTest : public SphSimulationTestBase {};

TEST_F(UpdateParticlesTest, SmallSystemDoesNotCrash) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    ASSERT_NO_THROW(sim.update_particles(particles, 0.05f));
}

TEST_F(UpdateParticlesTest, PositionsAreFiniteAfterStep) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    sim.update_particles(particles, 0.05f);

    for (auto &p : particles) {
        EXPECT_TRUE(std::isfinite(p.pos.x));
        EXPECT_TRUE(std::isfinite(p.pos.y));
        EXPECT_TRUE(std::isfinite(p.pos.z));
    }
}

TEST_F(UpdateParticlesTest, VelocitiesAreFiniteAfterStep) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    sim.update_particles(particles, 0.05f);

    for (auto &p : particles) {
        EXPECT_TRUE(std::isfinite(p.velocity.x));
        EXPECT_TRUE(std::isfinite(p.velocity.y));
        EXPECT_TRUE(std::isfinite(p.velocity.z));
    }
}

TEST_F(UpdateParticlesTest, GravityPullsParticlesDown) {
    Particle p;
    p.pos = {0.f, 0.f, 0.f};
    p.density = sim.simulation_data.REST_DENSITY;
    std::vector<Particle> particles{p};
    neighbors_cache_resize(1);

    float y_before = particles[0].pos.y;
    sim.update_particles(particles, 0.05f);
    float y_after = particles[0].pos.y;

    EXPECT_LT(y_after, y_before) << "Gravity should move the particle downward";
}

TEST_F(UpdateParticlesTest, ParticlesStayInsideBox) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);

    for (int step = 0; step < 20; ++step) sim.update_particles(particles, 0.05f);

    for (auto &p : particles) {
        EXPECT_GE(p.pos.x, sim.simulation_data.BOX_START.x);
        EXPECT_LE(p.pos.x, sim.simulation_data.BOX_END.x);
        EXPECT_GE(p.pos.y, sim.simulation_data.BOX_START.y);
        EXPECT_LE(p.pos.y, sim.simulation_data.BOX_END.y);
        EXPECT_GE(p.pos.z, sim.simulation_data.BOX_START.z);
        EXPECT_LE(p.pos.z, sim.simulation_data.BOX_END.z);
    }
}

TEST_F(UpdateParticlesTest, AccelerationsAreFiniteAfterStep) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    sim.update_particles(particles, 0.05f);

    for (auto &p : particles) {
        EXPECT_TRUE(std::isfinite(p.accerelation.x));
        EXPECT_TRUE(std::isfinite(p.accerelation.y));
        EXPECT_TRUE(std::isfinite(p.accerelation.z));
    }
}

TEST_F(UpdateParticlesTest, DensitiesArePositiveAfterStep) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    sim.update_particles(particles, 0.05f);

    for (auto &p : particles) {
        EXPECT_GT(p.density, 0.f) << "Each particle should have positive density after a step";
    }
}

TEST_F(UpdateParticlesTest, ParticleCountUnchangedAfterMultipleSteps) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);
    size_t count_before = particles.size();

    for (int step = 0; step < 5; ++step) sim.update_particles(particles, 0.05f);

    EXPECT_EQ(particles.size(), count_before);
}

TEST_F(UpdateParticlesTest, MultipleStepsRemainStable) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);

    // Run for 100 steps — should not blow up numerically
    for (int step = 0; step < 100; ++step) sim.update_particles(particles, 0.01f);

    for (auto &p : particles) {
        EXPECT_TRUE(std::isfinite(p.pos.x));
        EXPECT_TRUE(std::isfinite(p.pos.y));
        EXPECT_TRUE(std::isfinite(p.pos.z));
        EXPECT_TRUE(std::isfinite(p.velocity.x));
        EXPECT_TRUE(std::isfinite(p.velocity.y));
        EXPECT_TRUE(std::isfinite(p.velocity.z));
    }
}

TEST_F(UpdateParticlesTest, ZSortTriggeredAtStep100DoesNotCrash) {
    std::vector<Particle> particles;
    sim.regenerate_particles(particles, 0.8f);

    // The implementation z-sorts every 100 steps; run just past that threshold
    for (int step = 0; step < 101; ++step) {
        ASSERT_NO_THROW(sim.update_particles(particles, 0.01f));
    }
}
