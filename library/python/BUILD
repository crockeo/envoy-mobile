load("@rules_python//python:defs.bzl", "py_binary")
load("@pybind11_bazel//:build_defs.bzl", "pybind_extension", "pybind_library")

licenses(["notice"])  # Apache 2

py_binary(
    name = "e2e_test",
    srcs = ["e2e_test.py"],
    data = [
        ":envoy_engine.so",
    ],
    main = "e2e_test.py",
    deps = [
        "//library/python/gevent_util",
    ],
)

pybind_library(
    name = "envoy_engine_lib",
    srcs = [
        "bytes_view.cc",
        "engine_builder_shim.cc",
        "stream_prototype_shim.cc",
        "stream_shim.cc",
    ],
    hdrs = [
        "bytes_view.h",
        "engine_builder_shim.h",
        "stream_prototype_shim.h",
        "stream_shim.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//library/cc:envoy_engine_cc_lib",
    ],
)

pybind_extension(
    name = "envoy_engine",
    srcs = [
        "module_definition.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":envoy_engine_lib",
        "//library/cc:envoy_engine_cc_lib",
        # for log levels enum
        "@envoy//source/common/common:base_logger_lib",
    ],
)
