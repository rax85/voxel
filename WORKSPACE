load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

http_archive(
     name = "com_google_googletest",
     urls = ["https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip"],
     strip_prefix = "googletest-1.13.0",
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

