#include "voxel/voxel.h"

#include <algorithm>
#include <cmath>
#include <cassert>
#include <limits>
#include <vector>

#include "simplebmp/simplebmp.h"
#include "simplestl/simplestl.h"
#include "workqueue/workqueue.h"


namespace voxel::internal {

void GetBoundsFromBmp(const std::filesystem::path& bmp, int64_t* x_max, int64_t* y_max) {
  auto image = simplebmp::Image::Load(bmp);
  assert(image.has_value());
  *x_max = image.value().Width();
  *y_max = image.value().Height();
}

void GetBoundsFromStl(const std::filesystem::path& stl, double step, int64_t* x_max, int64_t* y_max, int64_t* z_max) {
  simplestl::StlReader reader(stl);
  std::vector<libmath::Triangle> triangles;
  bool succeeded = reader.Read(&triangles);
  assert(succeeded);

  double xmax = std::numeric_limits<double>::lowest();
  double ymax = std::numeric_limits<double>::lowest();
  double zmax = std::numeric_limits<double>::lowest();

  for (const auto& triangle : triangles) {
    for (const auto& vertex : triangle.vertices) {
      xmax = std::max(xmax, vertex.x);
      ymax = std::max(xmax, vertex.y);
      zmax = std::max(xmax, vertex.z);
    }
  }

  *x_max = static_cast<int64_t>(std::ceil(xmax / step));
  *y_max = static_cast<int64_t>(std::ceil(ymax / step));
  *z_max = static_cast<int64_t>(std::ceil(zmax / step));
}

}