#include "voxel/voxel.h"

#include <algorithm>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <span>
#include <vector>

#include "glog/logging.h"
#include "libmath/line.h"
#include "libmath/plane.h"
#include "libmath/point.h"
#include "simplebmp/simplebmp.h"
#include "simplestl/simplestl.h"
#include "workqueue/workqueue.h"


namespace voxel::builder::internal {

bool IsBlack(const simplebmp::Color& color) {
  return color.r == 0 && color.g == 0 && color.b == 0;
}

void ComputeBoundingBox(std::span<libmath::Triangle> triangles, double& width, double& height, double& depth) {
  width = std::numeric_limits<double>::lowest();
  height = std::numeric_limits<double>::lowest();
  depth = std::numeric_limits<double>::lowest();

  // All triangles in stl are in the positive octant.
  for (const auto& triangle : triangles) {
    for (const auto& vertex : triangle.vertices) {
      CHECK(vertex.x >= 0);
      CHECK(vertex.y >= 0);
      CHECK(vertex.z >= 0);

      width = std::max(width, vertex.x);
      height = std::max(height, vertex.y);
      depth = std::max(depth, vertex.z);
    }
  }
}

std::vector<libmath::Plane> ComputePlane(std::span<libmath::Triangle> triangles, double translate_x, double translate_y, double translate_z) {
  std::vector<libmath::Plane> planes;
  for (auto& triangle : triangles) {
    planes.emplace_back(triangle.vertices[0].Translate(translate_x, translate_y, translate_z),
                        triangle.vertices[1].Translate(translate_x, translate_y, translate_z),
                        triangle.vertices[2].Translate(translate_x, translate_y, translate_z));
  }
  return planes;
}

libmath::Point MakePoint(int64_t x, int64_t y, int64_t z, double step) {
  libmath::Point p;
  double halfstep = step / 2;
  p.x = std::fma(x, step, halfstep);
  p.y = std::fma(y, step, halfstep);
  p.z = std::fma(z, step, halfstep);
  return p;
}

int64_t ComputeSteps(double dim, double step) {
  int64_t num_steps = static_cast<int64_t>(std::ceil(dim / step));
  return num_steps;
}


int64_t ComputeIntersections(std::span<libmath::Plane> triangles, const libmath::Point& p1, const libmath::Point& p2) {
  libmath::Line line(p1, p2);
  std::vector<const libmath::Plane*> intersected_triangles;

  for (const auto& triangle : triangles) {
    // The line must cut the triangle for it to be considered intersecting.
    if (!line.LiesOnPlane(triangle) && line.IntersectsWithinBounds(triangle)) {
      // Skip a triangle that has a shared edge with one that has already been intersected to prevent double counting.
      // This happens at the boundary of adjacent triangles.
      bool duplicate_intersection = false;
      for (const auto* prev_intersection : intersected_triangles) {
        if (prev_intersection->ToTriangle().SharesEdge(triangle.ToTriangle())) {
          duplicate_intersection = true;
          break;
        }
      }

      if (duplicate_intersection) {
        continue;
      }

      intersected_triangles.push_back(&triangle);
    }
  }

  return intersected_triangles.size();
}

}