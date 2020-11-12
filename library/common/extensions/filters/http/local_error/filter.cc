#include "library/common/extensions/filters/http/local_error/filter.h"

#include "envoy/http/codes.h"
#include "envoy/server/filter_config.h"

#include "common/http/header_map_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

LocalErrorFilterConfig::LocalErrorFilterConfig(
    const envoymobile::extensions::filters::http::local_error::LocalError& proto_config)
    : enabled_(proto_config.enabled()) {}

LocalErrorFilter::LocalErrorFilter(LocalErrorFilterConfigSharedPtr config) : config_(config) {}

Http::FilterHeadersStatus LocalErrorFilter::encodeHeaders(Http::ResponseHeaderMap& /*headers*/,
                                                          bool /*end_stream*/) {
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus LocalErrorFilter::encodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus LocalErrorFilter::encodeTrailers(Http::ResponseTrailerMap& /*trailers*/) {
  return Http::FilterTrailersStatus::Continue;
}

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
