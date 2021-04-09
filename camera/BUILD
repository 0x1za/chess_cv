package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "calibrate",
    srcs = ["calibrate.cc"],
    deps = [
        "@opencv//:opencv",
        "@com_github_gflags_gflags//:gflags"
    ],
)

cc_binary(
    name = "rtsp_stream",
    srcs = ["rtsp_stream.cpp"],
    deps = [
        "@opencv//:opencv",
        "@com_github_gflags_gflags//:gflags"
    ],
)
