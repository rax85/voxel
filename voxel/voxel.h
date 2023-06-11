#pragma once

#include <cassert>
#include <cstdint>
#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

#include "simplebmp/simplebmp.h"
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

namespace internal {

  void GetBoundsFromBmp(const std::filesystem::path& bmp, int64_t* x_max, int64_t* y_max);
  void GetBoundsFromStl(const std::filesystem::path& stl, double step, int64_t* x_max, int64_t* y_max, int64_t* z_max);

}


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
class VoxelGrid3d : workqueue::Grid3d<T> {
  public:
    VoxelGrid3d(const std::filesystem::path& stl, size_t step) {
      step_ = step;
      internal::GetBoundsFromStl(stl, step, &this->x_dim_, &this->y_dim_, &this->z_dim_);
      assert(this->x_dim_ > 0);
      assert(this->y_dim_ > 0);
      assert(this->z_dim_ > 0);
      this->elements_.resize(this->x_dim_ * this->y_dim_ * this->z_dim_);
    }

    int64_t XDim() const { return this->x_dim_; }
    int64_t YDim() const { return this->y_dim_; }
    int64_t ZDim() const { return this->z_dim_; }
    double Step() const { return step_; }

  private:
    double step_ = 0;
};


template <std::derived_from<Voxel> T>
class VoxelGrid3dParallelTask {
  public:
    VoxelGrid3dParallelTask(workqueue::WorkQueue* work_queue, VoxelGrid3d<T>* grid) : work_queue_(work_queue), grid_(grid) {}

    virtual void ForEachXY(VoxelGrid3d<T>* grid, int64_t x, int64_t y) {}
    virtual void ForEachXYZ(VoxelGrid3d<T>* grid, int64_t x, int64_t y, int64_t z) {}

    void Run() {
      for (int64_t x = 0; x < grid_->XDim(); x++) {
        for (int64_t y = 0; y < grid_->YDim(); y++) {
          tasks_->push_back(std::make_shared<SubTask>(this, x, y));
        }
      }
      for (auto& task : tasks_) {
        work_queue_->Enqueue(task);
      }
    }

    void WaitForCompletion() {
      for (auto& task : tasks_) {
        task->WaitForFinish();
      }
    }

    void RunSync() {
      Run();
      WaitForCompletion();
    }

  private:
    class SubTask : workqueue::WorkItem {
      public:
        SubTask(VoxelGrid3dParallelTask* parent, int64_t x, int64_t y) : parent_(parent), x_(x), y_(y) {}

        void Run() override {
          parent_->ForEachXY(parent_->grid_, x_, y_);
          for (int64_t z = 0; z < parent_->grid_->ZDim(); z++) {
            parent_->ForEachXYZ(parent_->grid_, x_, y_, z);
          }
        }

      private:
        VoxelGrid3dParallelTask* parent_;
        int64_t x_;
        int64_t y_;
    };

    workqueue::WorkQueue* work_queue_;
    VoxelGrid3d<T>* grid_;
    std::vector<std::shared_ptr<SubTask>> tasks_;
};

}