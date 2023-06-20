#pragma once

#include <functional>

#include "voxel/voxel.h"
#include "simplebmp/canvas.h"

namespace voxel::renderer {

template <std::derived_from<Voxel> T>
void Render(const VoxelGrid2d<T>& grid, simplebmp::Canvas* canvas,
            std::function<simplebmp::Color4f(int64_t x, int64_t y, double step, const void* voxel)> per_voxel_callback) {
  for (int64_t x = 0; x < grid.XDim(); x++) {
    for (int64_t y = 0; y < grid.YDim(); y++) {
      simplebmp::Color4f color = per_voxel_callback(x, y, grid.Step(), grid.At(x, y));
      canvas->Set(x, y, color);
    }
  }
}

}