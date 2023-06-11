#pragma once

#include <cassert>
#include <cstdint>
#include <concepts>
#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>

#include "simplebmp/simplebmp.h"
#include "voxel/voxel.h"
#include "workqueue/grid.h"
#include "workqueue/workqueue.h"


namespace voxel {
namespace internal {

bool IsBlack(const simplebmp::Color& color) {
  return color.r == 0 && color.g == 0 && color.b == 0;
}

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

}