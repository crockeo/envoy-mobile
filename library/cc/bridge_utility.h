#pragma once

#include <string>
#include <vector>

#include "headers.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Platform {

envoy_data string_as_envoy_data(const std::string& str);
std::string envoy_data_as_string(envoy_data data);

envoy_headers raw_header_map_as_envoy_headers(const RawHeaderMap& headers);
RawHeaderMap envoy_headers_as_raw_header_map(envoy_headers raw_headers);

} // namespace Platform
} // namespace Envoy
