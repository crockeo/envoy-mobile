#pragma once

#include "envoy/http/filter.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/local_error/filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

class LocalErrorFilterConfig {
public:
  LocalErrorFilterConfig(
      const envoymobile::extensions::filters::http::local_error::LocalError& proto_config);

private:
};

typedef std::shared_ptr<LocalErrorFilterConfig> LocalErrorFilterConfigSharedPtr;

/**
 * Filter to assert expectations on HTTP requests.
 */
class LocalErrorFilter final : public Http::PassThroughFilter {
public:
  LocalErrorFilter(LocalErrorFilterConfigSharedPtr config);

  // StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap& trailers) override;

  // StreamEncoderFilter
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus encodeTrailers(Http::ResponseTrailerMap& trailers) override;

private:
  const LocalErrorFilterConfigSharedPtr config_;
};

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
