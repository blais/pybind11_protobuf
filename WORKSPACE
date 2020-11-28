workspace(name="pybind11_protobuf")

if not native.existing_rule("com_google_protobuf"):
    http_archive(
        name = "com_google_protobuf",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v4.0.0-rc2.tar.gz"],
        sha256 = "cd26c9011e065b4eb95c79a74bb4f882f3b0beb6629a9c50312e387775c681c9",
        strip_prefix = "protobuf-4.0.0-rc2",
    )

if not native.existing_rule("pybind11"):
    http_archive(
        name = "pybind11",
        build_file = "@pybind11_bazel//:pybind11.BUILD",
        sha256 = "cdbe326d357f18b83d10322ba202d69f11b2f49e2d87ade0dc2be0c5c34f8e2a",
        strip_prefix = "pybind11-2.6.1",
        urls = ["https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz"],
    )

if not native.existing_rule("pybind11_bazel"):
    http_archive(
        name = "pybind11_bazel",
        strip_prefix = "pybind11_bazel-26973c0ff320cb4b39e45bc3e4297b82bc3a6c09",
        sha256 = "a5666d950c3344a8b0d3892a88dc6b55c8e0c78764f9294e806d69213c03f19d",
        urls = ["https://github.com/pybind/pybind11_bazel/archive/26973c0ff320cb4b39e45bc3e4297b82bc3a6c09.zip"],
    )
