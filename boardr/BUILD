package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "board",
    srcs = ["board.cc"],
    deps = [
        "@opencv//:opencv",
        "@com_github_gflags_gflags//:gflags"
    ],
)
