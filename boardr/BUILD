load("@rules_cc//cc:defs.bzl", "cc_binary")
package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "board",
    srcs = ["board.cc"],
    deps = [
        "@opencv//:opencv",
        "@com_google_absl//absl/flags:parse",
    ],
)
