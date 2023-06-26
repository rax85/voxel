cc_library(
    name = "voxel",
    srcs = ["voxel.cc"],
    hdrs = [
        "voxel.h",
        "builder.h",
        "renderer.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
      "@com_github_google_glog//:glog",
      "@libmath//libmath:line",
      "@libmath//libmath:plane",
      "@libmath//libmath:triangle",
      "@simplebmp//simplebmp:canvas",
      "@simplebmp//simplebmp:simplebmp",
      "@simplestl//simplestl:simplestl",
      "@workqueue//workqueue:grid",
      "@workqueue//workqueue:workqueue",
    ]
)

cc_test(
  name = "voxel_test",
  srcs = ["voxel_test.cc"],
  data = glob(["testdata/**"]),
  deps = [
    ":voxel",
    "@bazel_tools//tools/cpp/runfiles",
    "@com_google_googletest//:gtest_main",
  ]
)