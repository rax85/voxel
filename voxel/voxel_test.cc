#include "voxel/voxel.h"

#include <filesystem>
#include <string>

#include "voxel/builder.h"
#include "voxel/renderer.h"

#include "gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"


namespace voxel {
namespace {

using bazel::tools::cpp::runfiles::Runfiles;

std::string ResolvePath(const std::string& path) {
  std::string error;
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
  EXPECT_TRUE(std::filesystem::exists(runfiles->Rlocation(path)));
  return runfiles->Rlocation(path);
}


class TestVoxel : public Voxel {
  public:
    int val_;
};


TEST(VoxelGrid2dTests, NonExistentBmp) {
  VoxelGrid2d<TestVoxel> grid;
  EXPECT_FALSE(voxel::builder::BuildFromBmp("/foo/bar.bmp", 1.0, &grid));
}

TEST(VoxelGrid2dTests, CreateGrid) {
  VoxelGrid2d<TestVoxel> grid;
  EXPECT_TRUE(voxel::builder::BuildFromBmp(ResolvePath("__main__/voxel/testdata/test.bmp"), 1.0, &grid));

  /*
   *        E  E  E  E  E (4, 4)
   *        E  B  B  B  E
   *        E  B  I  B  E
   *        E  B  B  B  E
   * (0, 0) E  E  E  E  E (4, 0)
   */


  EXPECT_EQ(grid.At(0, 0)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(0, 1)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(0, 2)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(0, 3)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(0, 4)->type, kVoxelTypeExternal);

  EXPECT_EQ(grid.At(1, 0)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(1, 1)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(1, 2)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(1, 3)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(1, 4)->type, kVoxelTypeExternal);

  EXPECT_EQ(grid.At(2, 0)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(2, 1)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(2, 2)->type, kVoxelTypeInternal);
  EXPECT_EQ(grid.At(2, 3)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(2, 4)->type, kVoxelTypeExternal);

  EXPECT_EQ(grid.At(3, 0)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(3, 1)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(3, 2)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(3, 3)->type, kVoxelTypeInternal | kVoxelTypeBoundary);
  EXPECT_EQ(grid.At(3, 4)->type, kVoxelTypeExternal);

  EXPECT_EQ(grid.At(4, 0)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(4, 1)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(4, 2)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(4, 3)->type, kVoxelTypeExternal);
  EXPECT_EQ(grid.At(4, 4)->type, kVoxelTypeExternal);
}

TEST(VoxelGrid2dTests, RenderTest) {
  VoxelGrid2d<TestVoxel> grid;
  ASSERT_TRUE(voxel::builder::BuildFromBmp(ResolvePath("__main__/voxel/testdata/test.bmp"), 1.0, &grid));
  simplebmp::Canvas canvas(grid.XDim(), grid.YDim(), 2);
  simplebmp::Color4f red(1.0f, 0.0f, 0.0f, 1.0f);
  simplebmp::Color4f white(1.0f, 1.0f, 1.0f, 1.0f);
  simplebmp::Color4f black;
  renderer::Render(grid, &canvas, [&](int64_t x, int64_t y, double step, const void* voxel) -> simplebmp::Color4f {
    const TestVoxel* v = reinterpret_cast<const TestVoxel*>(voxel);
    if ((v->type & kVoxelTypeBoundary) == kVoxelTypeBoundary) {
      return red;
    }
    if (v->type == kVoxelTypeInternal) {
      return black;
    }
    return white;
  });

  std::optional<simplebmp::Image> maybe_image = simplebmp::Image::Load(ResolvePath("__main__/voxel/testdata/test_render.bmp"));
  ASSERT_TRUE(maybe_image.has_value());
  const simplebmp::Image& reference = maybe_image.value();

  simplebmp::Image actual(canvas);
  EXPECT_EQ(actual, reference);
}

simplebmp::Color4f RenderCallback(int64_t x, int64_t y, int64_t z, double step, const void* voxel) {
  const TestVoxel* v = reinterpret_cast<const TestVoxel*>(voxel);
  simplebmp::Color4f red(1.0f, 0.0f, 0.0f, 1.0f);
  simplebmp::Color4f blue(0.0f, 0.0f, 1.0f, 1.0f);
  simplebmp::Color4f white(1.0f, 1.0f, 1.0f, 1.0f);
  simplebmp::Color4f black;

  if ((v->type & kVoxelTypeBoundary) == kVoxelTypeBoundary) {
    return red;
  }
  if (v->type == kVoxelTypeInternal) {
    return black;
  } else if (v->type == kVoxelTypeExternal) {
    return white;
  }
  return blue;
}

void StlTestHelper(const std::string& prefix) {
  VoxelGrid3d<TestVoxel> grid;
  EXPECT_TRUE(voxel::builder::BuildFromStl(ResolvePath("__main__/voxel/testdata/" + prefix + ".stl"), &grid, 1.0));

  {
    simplebmp::Canvas canvas_xy(grid.XDim(), grid.YDim(), 2);
    renderer::RenderSliceXY(grid, grid.ZDim() / 2, &canvas_xy, RenderCallback);
    simplebmp::Image actual_xy(canvas_xy);
    EXPECT_TRUE(actual_xy.Write(std::filesystem::temp_directory_path().append(prefix + "_xy.bmp").c_str()));

    auto maybe_ref = simplebmp::Image::Load(ResolvePath("__main__/voxel/testdata/" + prefix + "_xy.bmp"));
    ASSERT_TRUE(maybe_ref.has_value());
    EXPECT_EQ(maybe_ref.value(), actual_xy);
  }

  {
    simplebmp::Canvas canvas_xz(grid.XDim(), grid.ZDim(), 2);
    renderer::RenderSliceXZ(grid, grid.YDim() / 2, &canvas_xz, RenderCallback);
    simplebmp::Image actual_xz(canvas_xz);
    EXPECT_TRUE(actual_xz.Write(std::filesystem::temp_directory_path().append(prefix + "_xz.bmp").c_str()));

    auto maybe_ref = simplebmp::Image::Load(ResolvePath("__main__/voxel/testdata/" + prefix + "_xz.bmp"));
    ASSERT_TRUE(maybe_ref.has_value());
    EXPECT_EQ(maybe_ref.value(), actual_xz);
  }

  {
    simplebmp::Canvas canvas_yz(grid.ZDim(), grid.YDim(), 2);
    renderer::RenderSliceYZ(grid, grid.XDim() / 2, &canvas_yz, RenderCallback);
    simplebmp::Image actual_yz(canvas_yz);
    EXPECT_TRUE(actual_yz.Write(std::filesystem::temp_directory_path().append(prefix + "_yz.bmp").c_str()));

    auto maybe_ref = simplebmp::Image::Load(ResolvePath("__main__/voxel/testdata/" + prefix + "_yz.bmp"));
    ASSERT_TRUE(maybe_ref.has_value());
    EXPECT_EQ(maybe_ref.value(), actual_yz);
  }
}

TEST(VoxelGrid3dTests, StlCube) {
  StlTestHelper("cube");
}

TEST(VoxelGrid3dTests, StlSphere) {
  StlTestHelper("sphere");
}

TEST(VoxelGrid3dTests, StlCone) {
  StlTestHelper("cone");
}

TEST(VoxelGrid3dTests, StlCylinder) {
  StlTestHelper("cylinder");
}

TEST(VoxelGrid3dTests, StlCubeWithCutout) {
  StlTestHelper("cube_with_cutout");
}

TEST(VoxelGrid3dTests, StlHollowCube) {
  StlTestHelper("hollow_cube");
}

TEST(VoxelGrid3dTests, StlPyramid) {
  StlTestHelper("pyramid");
}

}
}