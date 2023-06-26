#pragma once

#include <functional>

#include "voxel/voxel.h"
#include "simplebmp/canvas.h"

#include "glog/logging.h"

namespace voxel::renderer {

template <std::derived_from<Voxel> T>
void Render(const VoxelGrid2d<T>& grid, simplebmp::Canvas* canvas,
            std::function<simplebmp::Color4f(int64_t x, int64_t y, double step, const void* voxel)> per_voxel_callback) {
  CHECK_EQ(canvas->Width(), grid.XDim());
  CHECK_EQ(canvas->Height(), grid.YDim());
  for (int64_t x = 0; x < grid.XDim(); x++) {
    for (int64_t y = 0; y < grid.YDim(); y++) {
      simplebmp::Color4f color = per_voxel_callback(x, y, grid.Step(), grid.At(x, y));
      canvas->Set(x, y, color);
    }
  }
}

template <std::derived_from<Voxel> T>
void RenderSliceXY(const VoxelGrid3d<T>& grid, int64_t z_offset, simplebmp::Canvas* canvas,
            std::function<simplebmp::Color4f(int64_t x, int64_t y, int64_t z, double step, const void* voxel)> per_voxel_callback) {
  CHECK_EQ(canvas->Width(), grid.XDim());
  CHECK_EQ(canvas->Height(), grid.YDim());
  for (int64_t x = 0; x < grid.XDim(); x++) {
    for (int64_t y = 0; y < grid.YDim(); y++) {
      simplebmp::Color4f color = per_voxel_callback(x, y, z_offset, grid.Step(), grid.At(x, y, z_offset));
      canvas->Set(x, y, color);
    }
  }
}

template <std::derived_from<Voxel> T>
void RenderSliceXZ(const VoxelGrid3d<T>& grid, int64_t y_offset, simplebmp::Canvas* canvas,
            std::function<simplebmp::Color4f(int64_t x, int64_t y, int64_t z, double step, const void* voxel)> per_voxel_callback) {
  CHECK_EQ(canvas->Width(), grid.XDim());
  CHECK_EQ(canvas->Height(), grid.ZDim());
  for (int64_t x = 0; x < grid.XDim(); x++) {
    for (int64_t z = 0; z < grid.ZDim(); z++) {
      simplebmp::Color4f color = per_voxel_callback(x, y_offset, z, grid.Step(), grid.At(x, y_offset, z));
      canvas->Set(x, z, color);
    }
  }
}

template <std::derived_from<Voxel> T>
void RenderSliceYZ(const VoxelGrid3d<T>& grid, int64_t x_offset, simplebmp::Canvas* canvas,
            std::function<simplebmp::Color4f(int64_t x, int64_t y, int64_t z, double step, const void* voxel)> per_voxel_callback) {
  CHECK_EQ(canvas->Height(), grid.YDim());
  CHECK_EQ(canvas->Width(), grid.ZDim());

  for (int64_t y = 0; y < grid.YDim(); y++) {
    for (int64_t z = 0; z < grid.ZDim(); z++) {
      simplebmp::Color4f color = per_voxel_callback(x_offset, y, z, grid.Step(), grid.At(x_offset, y, z));
      canvas->Set(z, y, color);
    }
  }
}

}