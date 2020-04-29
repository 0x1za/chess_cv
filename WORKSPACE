load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
# OpenCV 2.4.13.2

# Please run $ brew install opencv@3
new_local_repository(
    name = "opencv",
    build_file = "third_party/opencv.BUILD",
    path = "/usr",
)

git_repository(
    name = "com_github_gflags_gflags",
    remote = "https://github.com/gflags/gflags.git",
    tag = "v2.2.2"
)
