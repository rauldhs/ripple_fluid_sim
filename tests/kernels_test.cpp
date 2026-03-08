#include "engine/simulation/kernels.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

static constexpr float H = 1.0f;
static constexpr float NEAR_TOL = 1e-4f;

// ─────────────────────────────────────────────────────────────────────────────
// Poly6 kernel
// W(r,h) = (315 / (64*pi*h^9)) * (h^2 - r^2)^3   for 0 <= r <= h
// ─────────────────────────────────────────────────────────────────────────────

class Poly6Test : public ::testing::Test {};

TEST_F(Poly6Test, ZeroAtBoundary) {
    // (h^2 - h^2)^3 = 0
    EXPECT_NEAR(poly6_kernel({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(Poly6Test, ZeroOutsideSupport) {
    EXPECT_EQ(poly6_kernel({H + 0.01f, 0.f, 0.f}, H), 0.f);
    EXPECT_EQ(poly6_kernel({2.f * H, 0.f, 0.f}, H), 0.f);
}

TEST_F(Poly6Test, PositiveInsideSupport) {
    for (float t : {0.0f, 0.1f, 0.5f, 0.9f}) EXPECT_GT(poly6_kernel({t * H, 0.f, 0.f}, H), 0.f) << "t=" << t;
}

TEST_F(Poly6Test, PeakAtOrigin) { EXPECT_GT(poly6_kernel({0.f, 0.f, 0.f}, H), poly6_kernel({0.5f * H, 0.f, 0.f}, H)); }

TEST_F(Poly6Test, MonotonicallyDecreasingWithRadius) {
    float prev = poly6_kernel({0.f, 0.f, 0.f}, H);
    for (float t : {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f}) {
        float curr = poly6_kernel({t * H, 0.f, 0.f}, H);
        EXPECT_LT(curr, prev) << "t=" << t;
        prev = curr;
    }
}

TEST_F(Poly6Test, IsRadiallySymmetric) {
    // (0.3, 0.4, 0) and (0.5, 0, 0) both have magnitude 0.5
    EXPECT_NEAR(poly6_kernel({0.3f, 0.4f, 0.f}, H), poly6_kernel({0.5f, 0.f, 0.f}, H), NEAR_TOL);
}

TEST_F(Poly6Test, EvenFunction) {
    // W(r) = W(-r) since it depends only on |r|
    glm::vec3 r = {0.3f, 0.2f, 0.1f};
    EXPECT_NEAR(poly6_kernel(r, H), poly6_kernel(-r, H), NEAR_TOL);
}

TEST_F(Poly6Test, KnownValueAtHalfRadius) {
    // W(0.5, 1) = (315/(64*pi)) * (1 - 0.25)^3
    //           = (315/(64*pi)) * (0.75)^3
    //           = (315/(64*pi)) * (27/64)
    //           = 8505 / (4096*pi)
    //           ≈ 0.660944
    float expected = 8505.0f / (4096.0f * std::numbers::pi_v<float>);
    EXPECT_NEAR(poly6_kernel({0.5f, 0.f, 0.f}, H), expected, NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Poly6 gradient
// grad W = -6 * (315/(64*pi*h^9)) * (h^2 - r^2)^2 * r_vec
// ─────────────────────────────────────────────────────────────────────────────

class GradPoly6Test : public ::testing::Test {};

TEST_F(GradPoly6Test, ZeroAtOrigin) {
    // The r_vec factor is zero at the origin
    glm::vec3 g = grad_poly6({0.f, 0.f, 0.f}, H);
    EXPECT_NEAR(g.x, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

TEST_F(GradPoly6Test, ZeroAtBoundary) {
    // (h^2 - r^2)^2 = 0 at r=h
    glm::vec3 g = grad_poly6({H, 0.f, 0.f}, H);
    EXPECT_NEAR(g.x, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

TEST_F(GradPoly6Test, ZeroOutsideSupport) {
    glm::vec3 g = grad_poly6({H + 0.1f, 0.f, 0.f}, H);
    EXPECT_EQ(g.x, 0.f);
    EXPECT_EQ(g.y, 0.f);
    EXPECT_EQ(g.z, 0.f);
}

TEST_F(GradPoly6Test, PointsOppositeToR) {
    // poly6 decreases with |r|, so its gradient points in the -r direction
    glm::vec3 g = grad_poly6({0.4f, 0.f, 0.f}, H);
    EXPECT_LT(g.x, 0.f);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

TEST_F(GradPoly6Test, AntiSymmetric) {
    // grad W(-r) = -grad W(r)
    glm::vec3 r = {0.3f, 0.2f, 0.1f};
    glm::vec3 gp = grad_poly6(r, H);
    glm::vec3 gn = grad_poly6(-r, H);
    EXPECT_NEAR(gp.x, -gn.x, NEAR_TOL);
    EXPECT_NEAR(gp.y, -gn.y, NEAR_TOL);
    EXPECT_NEAR(gp.z, -gn.z, NEAR_TOL);
}

TEST_F(GradPoly6Test, ParallelToR) {
    // For a radial kernel grad W is parallel to r — cross product must be zero
    glm::vec3 r = {0.3f, 0.4f, 0.f};
    glm::vec3 cross = glm::cross(r, grad_poly6(r, H));
    EXPECT_NEAR(cross.x, 0.f, NEAR_TOL);
    EXPECT_NEAR(cross.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(cross.z, 0.f, NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Poly6 laplacian
// L = -6 * (315/(64*pi*h^9)) * (h^2 - r^2) * (3h^2 - 7r^2)
// ─────────────────────────────────────────────────────────────────────────────

class LaplacianPoly6Test : public ::testing::Test {};

TEST_F(LaplacianPoly6Test, ZeroOutsideSupport) { EXPECT_EQ(laplacian_poly6({H + 0.1f, 0.f, 0.f}, H), 0.f); }

TEST_F(LaplacianPoly6Test, ZeroAtBoundary) {
    // (h^2 - h^2) = 0
    EXPECT_NEAR(laplacian_poly6({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(LaplacianPoly6Test, NegativeAtOrigin) {
    // L(0,h) = -6*factor * h^2 * 3h^2 = -18*factor*h^4 < 0
    // factor = 315/(64*pi*h^9) > 0, so the whole expression is negative
    EXPECT_LT(laplacian_poly6({0.f, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianPoly6Test, ZeroCrossingAtSqrt3Over7) {
    // Setting 3h^2 - 7r^2 = 0  =>  r = h * sqrt(3/7)  ≈ 0.6547 for h=1
    // The (h^2-r^2) factor is nonzero here, so the zero comes entirely
    // from the (3h^2-7r^2) factor.
    float r_zero = H * std::sqrt(3.0f / 7.0f);
    EXPECT_NEAR(laplacian_poly6({r_zero, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(LaplacianPoly6Test, NegativeBelowZeroCrossing) {
    // For r < h*sqrt(3/7): (h^2-r^2) > 0 and (3h^2-7r^2) > 0
    // => -6*factor*positive*positive < 0
    float r = H * std::sqrt(3.0f / 7.0f) - 0.1f;
    EXPECT_LT(laplacian_poly6({r, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianPoly6Test, PositiveAboveZeroCrossing) {
    // For h*sqrt(3/7) < r < h: (h^2-r^2) > 0 and (3h^2-7r^2) < 0
    // => -6*factor*positive*negative > 0
    float r = H * std::sqrt(3.0f / 7.0f) + 0.05f;
    ASSERT_LT(r, H) << "test point must be inside support";
    EXPECT_GT(laplacian_poly6({r, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianPoly6Test, IsRadiallySymmetric) {
    EXPECT_NEAR(laplacian_poly6({0.4f, 0.f, 0.f}, H), laplacian_poly6({0.f, 0.4f, 0.f}, H), NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Spiky kernel
// W(r,h) = (15 / (pi*h^6)) * (h - r)^3   for 0 <= r <= h
// ─────────────────────────────────────────────────────────────────────────────

class SpikyTest : public ::testing::Test {};

TEST_F(SpikyTest, ZeroAtBoundary) {
    // (h - h)^3 = 0
    EXPECT_NEAR(spiky_kernel({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(SpikyTest, ZeroOutsideSupport) { EXPECT_EQ(spiky_kernel({H + 0.01f, 0.f, 0.f}, H), 0.f); }

TEST_F(SpikyTest, PositiveInsideSupport) {
    for (float t : {0.0f, 0.1f, 0.5f, 0.9f}) EXPECT_GT(spiky_kernel({t * H, 0.f, 0.f}, H), 0.f) << "t=" << t;
}

TEST_F(SpikyTest, MonotonicallyDecreasingWithRadius) {
    float prev = spiky_kernel({0.f, 0.f, 0.f}, H);
    for (float t : {0.1f, 0.2f, 0.3f, 0.5f, 0.7f, 0.9f}) {
        float curr = spiky_kernel({t * H, 0.f, 0.f}, H);
        EXPECT_LT(curr, prev) << "t=" << t;
        prev = curr;
    }
}

TEST_F(SpikyTest, IsRadiallySymmetric) {
    // (0.3, 0.4, 0) and (0.5, 0, 0) both have magnitude 0.5
    EXPECT_NEAR(spiky_kernel({0.3f, 0.4f, 0.f}, H), spiky_kernel({0.5f, 0.f, 0.f}, H), NEAR_TOL);
}

TEST_F(SpikyTest, KnownValueAtHalfRadius) {
    // W(0.5, 1) = (15/pi) * (1 - 0.5)^3
    //           = (15/pi) * (1/8)
    //           = 15 / (8*pi)
    //           ≈ 0.596831
    float expected = 15.0f / (8.0f * std::numbers::pi_v<float>);
    EXPECT_NEAR(spiky_kernel({0.5f, 0.f, 0.f}, H), expected, NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Spiky gradient
// grad W = -3 * (15/(pi*h^6)) * (h - r)^2 * (r_vec / r)   for r > EPSILON
// ─────────────────────────────────────────────────────────────────────────────

class GradSpikyTest : public ::testing::Test {};

TEST_F(GradSpikyTest, ZeroAtOrigin) {
    // r <= EPSILON guard
    glm::vec3 g = grad_spiky({0.f, 0.f, 0.f}, H);
    EXPECT_EQ(g.x, 0.f);
    EXPECT_EQ(g.y, 0.f);
    EXPECT_EQ(g.z, 0.f);
}

TEST_F(GradSpikyTest, ZeroAtBoundary) {
    // (h - r)^2 = 0 at r=h
    glm::vec3 g = grad_spiky({H, 0.f, 0.f}, H);
    EXPECT_NEAR(g.x, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

TEST_F(GradSpikyTest, ZeroOutsideSupport) {
    glm::vec3 g = grad_spiky({H + 0.1f, 0.f, 0.f}, H);
    EXPECT_EQ(g.x, 0.f);
    EXPECT_EQ(g.y, 0.f);
    EXPECT_EQ(g.z, 0.f);
}

TEST_F(GradSpikyTest, PointsOppositeToR) {
    // spiky decreases with |r|, gradient must point in -r direction
    glm::vec3 g = grad_spiky({0.5f, 0.f, 0.f}, H);
    EXPECT_LT(g.x, 0.f);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

TEST_F(GradSpikyTest, AntiSymmetric) {
    glm::vec3 r = {0.3f, 0.2f, 0.1f};
    glm::vec3 gp = grad_spiky(r, H);
    glm::vec3 gn = grad_spiky(-r, H);
    EXPECT_NEAR(gp.x, -gn.x, NEAR_TOL);
    EXPECT_NEAR(gp.y, -gn.y, NEAR_TOL);
    EXPECT_NEAR(gp.z, -gn.z, NEAR_TOL);
}

TEST_F(GradSpikyTest, ParallelToR) {
    glm::vec3 r = {0.3f, 0.4f, 0.f};
    glm::vec3 cross = glm::cross(r, grad_spiky(r, H));
    EXPECT_NEAR(cross.x, 0.f, NEAR_TOL);
    EXPECT_NEAR(cross.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(cross.z, 0.f, NEAR_TOL);
}

TEST_F(GradSpikyTest, KnownMagnitudeAtHalfRadius) {
    // |grad W(0.5, 1)| = 3 * (15/pi) * (1 - 0.5)^2
    //                  = 3 * (15/pi) * 0.25
    //                  = 45 / (4*pi)
    //                  ≈ 3.580986
    float expected = 45.0f / (4.0f * std::numbers::pi_v<float>);
    glm::vec3 g = grad_spiky({0.5f, 0.f, 0.f}, H);
    EXPECT_NEAR(std::abs(g.x), expected, NEAR_TOL);
    EXPECT_NEAR(g.y, 0.f, NEAR_TOL);
    EXPECT_NEAR(g.z, 0.f, NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Spiky laplacian
// L = 6 * (15/(pi*h^6)) * (h - r) * (2r - h) / r   for r > EPSILON
// ─────────────────────────────────────────────────────────────────────────────

class LaplacianSpikyTest : public ::testing::Test {};

TEST_F(LaplacianSpikyTest, ZeroOutsideSupport) { EXPECT_EQ(laplacian_spiky({H + 0.1f, 0.f, 0.f}, H), 0.f); }

TEST_F(LaplacianSpikyTest, ZeroAtOrigin) {
    // r <= EPSILON guard
    EXPECT_EQ(laplacian_spiky({0.f, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianSpikyTest, ZeroAtBoundary) {
    // (h - r) = 0 at r=h
    EXPECT_NEAR(laplacian_spiky({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(LaplacianSpikyTest, ZeroCrossingAtHalfH) {
    // (2r - h) = 0 at r = h/2
    // (h - r) is nonzero there, so the zero comes entirely from (2r - h)
    EXPECT_NEAR(laplacian_spiky({0.5f * H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(LaplacianSpikyTest, NegativeBelowHalfH) {
    // For r < h/2: (h-r) > 0 and (2r-h) < 0 => product negative
    EXPECT_LT(laplacian_spiky({0.4f * H, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianSpikyTest, PositiveAboveHalfH) {
    // For h/2 < r < h: (h-r) > 0 and (2r-h) > 0 => product positive
    EXPECT_GT(laplacian_spiky({0.6f * H, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianSpikyTest, IsRadiallySymmetric) {
    EXPECT_NEAR(laplacian_spiky({0.6f, 0.f, 0.f}, H), laplacian_spiky({0.f, 0.6f, 0.f}, H), NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Viscosity kernel
// W(r,h) = (15/(2*pi*h^3)) * (-r^3/(2h^3) + r^2/h^2 + h/(2r) - 1)
//          for EPSILON < r <= h
// ─────────────────────────────────────────────────────────────────────────────

class ViscosityTest : public ::testing::Test {};

TEST_F(ViscosityTest, ZeroAtOrigin) {
    // r <= EPSILON guard
    EXPECT_EQ(viscosity_kernel({0.f, 0.f, 0.f}, H), 0.f);
}

TEST_F(ViscosityTest, ZeroAtBoundary) {
    // At r=h: -h^3/(2h^3) + h^2/h^2 + h/(2h) - 1
    //       = -1/2 + 1 + 1/2 - 1 = 0
    EXPECT_NEAR(viscosity_kernel({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(ViscosityTest, ZeroOutsideSupport) { EXPECT_EQ(viscosity_kernel({H + 0.01f, 0.f, 0.f}, H), 0.f); }

TEST_F(ViscosityTest, PositiveInsideSupport) {
    for (float t : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) EXPECT_GT(viscosity_kernel({t * H, 0.f, 0.f}, H), 0.f) << "t=" << t;
}

TEST_F(ViscosityTest, MonotonicallyDecreasingWithRadius) {
    float prev = viscosity_kernel({0.05f, 0.f, 0.f}, H);
    for (float t : {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f}) {
        float curr = viscosity_kernel({t * H, 0.f, 0.f}, H);
        EXPECT_LT(curr, prev) << "t=" << t;
        prev = curr;
    }
}

TEST_F(ViscosityTest, IsRadiallySymmetric) {
    // (0.3, 0.4, 0) and (0.5, 0, 0) both have magnitude 0.5
    EXPECT_NEAR(viscosity_kernel({0.3f, 0.4f, 0.f}, H), viscosity_kernel({0.5f, 0.f, 0.f}, H), NEAR_TOL);
}

TEST_F(ViscosityTest, KnownValueAtHalfRadius) {
    // W(0.5, 1) = (15/(2*pi)) * (-0.5^3/2 + 0.5^2 + 1/(2*0.5) - 1)
    //           = (15/(2*pi)) * (-0.0625 + 0.25 + 1.0 - 1)
    //           = (15/(2*pi)) * 0.1875
    //           = 2.8125 / (2*pi)
    //           ≈ 0.447623
    float expected = 2.8125f / (2.0f * std::numbers::pi_v<float>);
    EXPECT_NEAR(viscosity_kernel({0.5f, 0.f, 0.f}, H), expected, NEAR_TOL);
}

// ─────────────────────────────────────────────────────────────────────────────
// Viscosity laplacian
// L(r,h) = 45 / (pi*h^5) * (1 - r/h)   for EPSILON < r <= h
// ─────────────────────────────────────────────────────────────────────────────

class LaplacianViscosityTest : public ::testing::Test {};

TEST_F(LaplacianViscosityTest, ZeroAtOrigin) {
    // r <= EPSILON guard
    EXPECT_EQ(laplacian_viscosity({0.f, 0.f, 0.f}, H), 0.f);
}

TEST_F(LaplacianViscosityTest, ZeroAtBoundary) {
    // (1 - r/h) = 0 at r=h
    EXPECT_NEAR(laplacian_viscosity({H, 0.f, 0.f}, H), 0.f, NEAR_TOL);
}

TEST_F(LaplacianViscosityTest, ZeroOutsideSupport) { EXPECT_EQ(laplacian_viscosity({H + 0.01f, 0.f, 0.f}, H), 0.f); }

TEST_F(LaplacianViscosityTest, PositiveInsideSupport) {
    // 45/(pi*h^5) > 0 and (1 - r/h) > 0 for r < h => product positive
    for (float t : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f})
        EXPECT_GT(laplacian_viscosity({t * H, 0.f, 0.f}, H), 0.f) << "t=" << t;
}

TEST_F(LaplacianViscosityTest, MonotonicallyDecreasingWithRadius) {
    // L is linear in r so it strictly decreases from r=0 to r=h
    float prev = laplacian_viscosity({0.05f, 0.f, 0.f}, H);
    for (float t : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) {
        float curr = laplacian_viscosity({t * H, 0.f, 0.f}, H);
        EXPECT_LT(curr, prev) << "t=" << t;
        prev = curr;
    }
}

TEST_F(LaplacianViscosityTest, IsRadiallySymmetric) {
    EXPECT_NEAR(laplacian_viscosity({0.4f, 0.f, 0.f}, H), laplacian_viscosity({0.f, 0.4f, 0.f}, H), NEAR_TOL);
}

TEST_F(LaplacianViscosityTest, KnownValueAtHalfRadius) {
    // L(0.5, 1) = (45/pi) * (1 - 0.5/1) = (45/pi) * 0.5 = 45/(2*pi)  ≈ 7.161972
    float expected = 45.0f / (2.0f * std::numbers::pi_v<float>);
    EXPECT_NEAR(laplacian_viscosity({0.5f, 0.f, 0.f}, H), expected, NEAR_TOL);
}
