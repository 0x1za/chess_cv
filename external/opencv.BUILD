# Bazel build for OpenCV.

cc_library(
    name = "dynamicuda",
    hdrs = glob([
        "modules/dynamicuda/include/**/*.hpp",
    ]),
    includes = [
        "modules/dynamicuda/include",
    ],
)

genrule(
    name = "module_includes",
    outs = [
        "opencv2/opencv_modules.hpp"
    ],
    cmd = """
cat > $@ <<"EOF"
#define HAVE_OPENCV_CORE
#define HAVE_OPENCV_IMGPROC
#define HAVE_OPENCV_ML
EOF"""
)

genrule(
    name = "cvconfig",
    outs = [
        "cvconfig.h",
    ],
    cmd = """
cat > $@ <<"EOF"
#define HAVE_JASPER
#define HAVE_JPEG
#define HAVE_PNG
#define HAVE_TIFF
#define CUDA_ARCH_BIN ""
#define CUDA_ARCH_FEATURES ""
#define CUDA_ARCH_PTX ""
EOF"""
)

genrule(
    name = "version_string",
    outs = [
        "version_string.inc"
    ],
    cmd = """
cat > $@ <<"EOF"
"\\n"
"General configuration for OpenCV 2.4.13.2 ===========\\n"
"\\n"
EOF"""
)

cc_library(
    name = "core",
    visibility = ["//visibility:public"],
    hdrs = glob([
        "modules/core/src/**/*.hpp",
        "modules/core/include/**/*.hpp",
    ]) + [
        ":module_includes",
        ":cvconfig",
        ":version_string",
    ],
    srcs = glob(["modules/core/src/**/*.cpp"]),
    copts = [
        "-Imodules/dynamicuda/include",
    ],
    linkopts = [
        "-pthread",
        "-lz",
    ],
    includes = [
        "modules/core/include",
    ],
    deps = [
        ":dynamicuda",
    ],
)

cc_library(
    name = "imgproc",
    visibility = ["//visibility:public"],
    hdrs = glob([
        "modules/imgproc/src/**/*.hpp",
        "modules/imgproc/src/**/*.h",
        "modules/imgproc/include/**/*.hpp",
        "modules/imgproc/include/**/*.h",
    ]) + [
        ":module_includes",
    ],
    srcs = glob([
        "modules/imgproc/src/**/*.cpp",
    ]),
    includes = [
        "modules/imgproc/include",
    ],
    deps = [
        ":core",
    ],
)

cc_library(
    name = "ml",
    visibility = [
        "//visibility:public",
    ],
    hdrs = glob([
        "modules/ml/src/**/*.hpp",
        "modules/ml/src/**/*.h",
        "modules/ml/include/**/*.hpp",
        "modules/ml/include/**/*.h",
    ]) + [
        ":module_includes",
    ],
    includes = [
        "modules/ml/include",
    ],
    deps = [
        ":core",
    ],
)