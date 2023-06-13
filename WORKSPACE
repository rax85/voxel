load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

http_archive(
     name = "com_google_googletest",
     urls = ["https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip"],
     strip_prefix = "googletest-1.13.0",
     sha256 = "ffa17fbc5953900994e2deec164bb8949879ea09b411e07f215bfbb1f87f4632",
)

git_repository(
    name = "libmath",
    branch = "main",
    init_submodules = True,
    recursive_init_submodules = True,
    remote = "https://github.com/rax85/libmath.git",
)

git_repository(
    name = "simplestl",
    branch = "main",
    init_submodules = True,
    recursive_init_submodules = True,
    remote = "https://github.com/rax85/simple-stl.git",
)

git_repository(
    name = "simplebmp",
    branch = "main",
    init_submodules = True,
    recursive_init_submodules = True,
    remote = "https://github.com/rax85/simple-bmp.git",
)

git_repository(
    name = "workqueue",
    branch = "main",
    init_submodules = True,
    recursive_init_submodules = True,
    remote = "https://github.com/rax85/workqueue.git",
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "122fb6b712808ef43fbf80f75c52a21c9760683dae470154f02bddfc61135022",
    strip_prefix = "glog-0.6.0",
    urls = ["https://github.com/google/glog/archive/v0.6.0.zip"],
)