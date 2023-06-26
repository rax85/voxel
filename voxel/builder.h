#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <concepts>
#include <filesystem>
#include <memory>
#include <vector>

#include "libmath/line.h"
#include "libmath/plane.h"
#include "simplebmp/simplebmp.h"
#include "simplestl/simplestl.h"
#include "voxel/voxel.h"
#include "workqueue/grid.h"
#include "workqueue/workqueue.h"


namespace voxel::builder {
namespace internal {

// Tests whether a pixel is black.
bool IsBlack(const simplebmp::Color& color);

// Computes the dims of the bounding box of a set of triangles from a stl file.
void ComputeBoundingBox(std::span<libmath::Triangle> triangles, double& width, double& height, double& depth);

// Computes a list of planes corresponding to the list of triangles, translated by an amount.
std::vector<libmath::Plane> ComputePlane(std::span<libmath::Triangle> triangles, double translate_x, double translate_y, double translate_z);

// Computes the minimum number of steps needed to discretize the dimension.
int64_t ComputeSteps(double dim, double step);

// Create a point given the x, y, and z step.
libmath::Point MakePoint(int64_t x, int64_t y, int64_t z, double step);

// Computes the number of triangles intersected by the line between the specified points.
int64_t ComputeIntersections(std::span<libmath::Plane> triangles, const libmath::Point& p1, const libmath::Point& p2);

}

template <std::derived_from<Voxel> T>
bool BuildFromBmp(const std::filesystem::path& bmp, double step, VoxelGrid2d<T>* grid) {
  auto maybe_image = simplebmp::Image::Load(bmp);
  if (!maybe_image.has_value()) {
    return false;
  }
  auto& image = maybe_image.value();
  grid->Init(image.Width(), image.Height(), step);

  // Mark voxels as internal or external.
  grid->AddForeachXYCallback([&](void* data, int64_t x, int64_t y) {
    VoxelGrid2d<T>* grid = reinterpret_cast<VoxelGrid2d<T>*>(data);
    grid->At(x, y)->type = internal::IsBlack(image.At(x, y)) ? kVoxelTypeInternal : kVoxelTypeExternal;
  });
  grid->RunSync(true);

  // Mark boundary voxels.
  grid->AddForeachXYCallback([&](void* data, int64_t x, int64_t y) {
    VoxelGrid2d<T>* grid = reinterpret_cast<VoxelGrid2d<T>*>(data);
    auto d2q9 = grid->D2Q9(x, y);
    auto* center = d2q9[0];

    // Boundaries are marked only on internal voxels.
    if (center->type == kVoxelTypeExternal) {
      return;
    }

    bool homogenous = true;
    for (auto element : grid->D2Q9(x, y)) {
      if (element != nullptr) {
        homogenous &= element->type != kVoxelTypeExternal;
      }
    }

    if (!homogenous) {
      center->type |= kVoxelTypeBoundary;
    }
  });
  grid->RunSync(true);

  return true;
}

template <std::derived_from<Voxel> T>
bool BuildFromStl(const std::filesystem::path& stl_path, VoxelGrid3d<T>* grid, double step,
                  int64_t extra_steps_x = 2, int64_t extra_steps_y = 2, int64_t extra_steps_z = 2) {
  simplestl::StlReader reader(stl_path);
  std::vector<libmath::Triangle> triangles;
  if (!reader.Read(&triangles)) {
    return false;
  }

  // At least 1 extra step is required for the algorithm to work, 2 to be safe.
  extra_steps_x = std::max(static_cast<int64_t>(2), extra_steps_x);
  extra_steps_y = std::max(static_cast<int64_t>(2), extra_steps_y);
  extra_steps_z = std::max(static_cast<int64_t>(2), extra_steps_z);

  // Compute bounding box and resize grid.
  double width = 0, height = 0, depth = 0;
  internal::ComputeBoundingBox(triangles, width, height, depth);
  width += 2 * extra_steps_x * step;
  height += 2 * extra_steps_y * step;
  depth += 2 * extra_steps_z * step;

  int64_t x_dim = internal::ComputeSteps(width, step);
  int64_t y_dim = internal::ComputeSteps(height, step);
  int64_t z_dim = internal::ComputeSteps(depth, step);
  grid->Init(x_dim, y_dim, z_dim, step);

  // Recompute triangles as planes and release triangle memory.
  std::vector<libmath::Plane> planes = internal::ComputePlane(triangles, extra_steps_x * step, extra_steps_y * step, extra_steps_z * step);
  triangles.clear();

  // Recalculate to adjust for the ceil used in the step calculation.
  width = x_dim * step;
  height = y_dim * step;
  depth = z_dim * step;

  // For each point, determine whether the voxel is internal or external.
  grid->AddForeachXYZCallback([&](void* data, int64_t x, int64_t y, int64_t z) {
    VoxelGrid3d<T>* grid = reinterpret_cast<VoxelGrid3d<T>*>(data);
    libmath::Point p = internal::MakePoint(x, y, z, step);
    auto* cur_voxel = grid->At(x, y, z);

    bool odd_intersection_count = true;
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {p.x, height, p.z}) % 2; // Up [ +y ]
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {p.x, 0, p.z}) % 2;      // Down [ -y ]
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {width, p.y, p.z}) % 2;  // Right [ +x ]
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {0, p.y, p.z}) % 2;      // Left [ -x ]
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {p.x, p.y, depth}) % 2;  // Front [ +z ]
    odd_intersection_count &= internal::ComputeIntersections(planes, p, {p.x, p.y, 0}) % 2;      // Back [ -z ]

    cur_voxel->type = odd_intersection_count ? kVoxelTypeInternal : kVoxelTypeExternal;
  });
  grid->RunSync(true);

  // Mark each internal voxel that has at least one non-internal neighbor, mark it as a boundary.
  grid->AddForeachXYZCallback([&](void* data, int64_t x, int64_t y, int64_t z) {
    VoxelGrid3d<T>* grid = reinterpret_cast<VoxelGrid3d<T>*>(data);
    auto* cur_voxel = grid->At(x, y, z);

    // If it's an external voxel, leave it alone.
    if (cur_voxel->type == kVoxelTypeExternal) {
      return;
    }

    // Check if it has any external neighbors.
    bool has_external_neighbor = false;
    for (const auto* neighbor : grid->D3Q27(x, y, z)) {
      if (neighbor != nullptr && neighbor->type == kVoxelTypeExternal) {
        has_external_neighbor = true;
        break;
      }
    }

    if (has_external_neighbor) {
      cur_voxel->type |= kVoxelTypeBoundary;
    }
  });
  grid->RunSync(true);

  return true;
}

}