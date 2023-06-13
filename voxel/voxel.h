#pragma once

#include <cassert>
#include <cstdint>
#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

#include "workqueue/grid.h"
#include "workqueue/workqueue.h"

namespace voxel {

constexpr int32_t kVoxelTypeUndefined = 0;
constexpr int32_t kVoxelTypeExternal = 1 << 1;
constexpr int32_t kVoxelTypeInternal = 1 << 2;
constexpr int32_t kVoxelTypeBoundary = 1 << 3;

class Voxel {
  public:
    int32_t type = kVoxelTypeUndefined;
};

template <std::derived_from<Voxel> T>
class VoxelGrid2d : public workqueue::Grid2d<T> {
  public:
    void Init(int64_t x_dim, int64_t y_dim, double step) {
      this->x_dim_ = x_dim;
      this->y_dim_ = y_dim;
      step_ = step;
      this->elements_.resize(this->x_dim_ * this->y_dim_);
    }

    int64_t XDim() const { return this->x_dim_; }
    int64_t YDim() const { return this->y_dim_; }
    double Step() const { return step_; }

    void AddForeachXYCallback(std::function<void(void*, int64_t, int64_t)> callback) {
      foreach_xy_callbacks_.push_back(callback);
    }

    void ClearForeachXYCallbacks() {
      foreach_xy_callbacks_.clear();
    }

    void Run() {
      for (int64_t x = 0; x < this->x_dim_; x++) {
        tasks_.push_back(std::make_shared<Task>(this, x));
      }
      for (auto& task : this->tasks_) {
        workqueue_.Enqueue(task);
      }
    }

    void WaitForCompletion() {
      for (auto& task : this->tasks_) {
        task->WaitForFinish();
      }
      tasks_.clear();
    }

    void RunSync(bool clear_on_completion) {
      Run();
      WaitForCompletion();
      if (clear_on_completion) {
        ClearForeachXYCallbacks();
      }
    }

  private:
    class Task : public workqueue::WorkItem {
      public:
        Task(VoxelGrid2d* grid, int64_t x) : grid_(grid), cur_x_(x) {}

        void Run() {
          for (int64_t y = 0; y < grid_->YDim(); y++) {
            for (auto& callback : grid_->foreach_xy_callbacks_) {
              callback(grid_, cur_x_, y);
            }
          }
        }

      private:
        VoxelGrid2d* grid_;
        int64_t cur_x_;
    };

    double step_ = 1.0;
    std::vector<std::shared_ptr<workqueue::WorkItem>> tasks_;
    workqueue::WorkQueue workqueue_;
    std::vector<std::function<void(void* data, int64_t x, int64_t y)>> foreach_xy_callbacks_;
};


template <std::derived_from<Voxel> T>
class VoxelGrid3d : public workqueue::Grid3d<T> {
  public:
    void Init(int64_t x_dim, int64_t y_dim, int64_t z_dim, size_t step) {
      this->x_dim_ = x_dim;
      this->y_dim_ = y_dim;
      this->z_dim_ = z_dim;
      step_ = step;
      this->elements_.resize(this->x_dim_ * this->y_dim_ * this->z_dim_);
    }

    int64_t XDim() const { return this->x_dim_; }
    int64_t YDim() const { return this->y_dim_; }
    int64_t ZDim() const { return this->z_dim_; }
    double Step() const { return step_; }

    void AddForeachXYZCallback(std::function<void(void*, int64_t, int64_t, int64_t)> callback) {
      foreach_xyz_callbacks_.push_back(callback);
    }

    void ClearForeachXYZCallbacks() {
      foreach_xyz_callbacks_.clear();
    }

    void Run() {
      for (int64_t x = 0; x < this->x_dim_; x++) {
        for (int64_t y = 0; y < this->y_dim_; y++) {
          tasks_.push_back(std::make_shared<Task>(this, x, y));
        }
      }
      for (auto& task : this->tasks_) {
        workqueue_.Enqueue(task);
      }
    }

    void WaitForCompletion() {
      for (auto& task : this->tasks_) {
        task->WaitForFinish();
      }
      tasks_.clear();
    }

    void RunSync(bool clear_on_completion) {
      Run();
      WaitForCompletion();
      if (clear_on_completion) {
        ClearForeachXYZCallbacks();
      }
    }

  private:
    class Task : public workqueue::WorkItem {
      public:
        Task(VoxelGrid3d* grid, int64_t x, int64_t y) : grid_(grid), cur_x_(x), cur_y_(y) {}

        void Run() {
          for (int64_t z = 0; z < grid_->ZDim(); z++) {
            for (auto& callback : grid_->foreach_xyz_callbacks_) {
              callback(grid_, cur_x_, cur_y_, z);
            }
          }
        }

      private:
        VoxelGrid3d* grid_;
        int64_t cur_x_;
        int64_t cur_y_;
    };

    double step_ = 0;
    std::vector<std::shared_ptr<workqueue::WorkItem>> tasks_;
    workqueue::WorkQueue workqueue_;
    std::vector<std::function<void(void* data, int64_t x, int64_t y, int64_t z)>> foreach_xyz_callbacks_;
};

}