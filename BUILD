package(default_visibility=["//visibility:public"])

load("@pybind11_bazel//:build_defs.bzl", "pybind_extension", "pybind_library")

pybind_library(
    name = "proto_utils",
    srcs = ["proto_utils.cc"],
    hdrs = ["proto_utils.h"],
    include_prefix = "pybind11_protobuf",
    deps = [
        "@com_google_protobuf//:protobuf",
    ]
)

pybind_library(
    name = "proto_casters",
    hdrs = ["proto_casters.h"],
    include_prefix = "pybind11_protobuf",
    defines = ["PYBIND11_PROTOBUF_MODULE_PATH=pybind11_protobuf.proto"],
    deps = [
        ":proto_utils",
        "@com_google_protobuf//:protobuf",

        # Note: This is not necessary, as the registration of the bindings will
        # get called automatically on the first module to call
        # ImportProtoModule().
        # ":proto.so",
    ],
)

pybind_extension(
    name = "proto",
    srcs = ["proto.cc"],
    deps = [
        ":proto_utils",
    ],
)




