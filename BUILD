package(default_visibility=["//visibility:public"])

cc_library(
    name = "pybind11_protobuf",
    srcs = ["proto_utils.cc"],
    include_prefix = "pybind11_protobuf",
    hdrs = [
        "proto_utils.h",
        "proto_casters.h"
    ],
    deps = [
        "@pybind11",
        "@com_google_protobuf//:protobuf",
    ]
)
