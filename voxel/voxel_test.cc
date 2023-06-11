#include "voxel/voxel.h"

#include "voxel/builder.h"

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


class TestVoxel2d : public Voxel {
  public:
    int val_;
};


TEST(VoxelGrid2dTests, NonExistentBmp) {
  VoxelGrid2d<TestVoxel2d> grid;
  EXPECT_FALSE(BuildFromBmp("/foo/bar.bmp", 1.0, &grid));
}

TEST(VoxelGrid2dTests, CreateGrid) {
  VoxelGrid2d<TestVoxel2d> grid;
  EXPECT_TRUE(BuildFromBmp(ResolvePath("__main__/voxel/testdata/test.bmp"), 1.0, &grid));

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

}
}