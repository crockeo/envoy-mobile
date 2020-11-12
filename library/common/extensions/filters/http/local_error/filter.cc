#include "library/common/extensions/filters/http/local_error/filter.h"

#include "envoy/http/codes.h"
#include "envoy/server/filter_config.h"

#include "common/http/codes.h"
#include "common/http/headers.h"
#include "common/http/header_map_impl.h"
#include "common/http/utility.h"

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
  uint64_t response_status = Utility::getResponseStatus(headers);
  // Track success for later bookkeeping (stream could still be reset).
  success_ = CodeUtility::is2xx(response_status);

  // TODO: ***HACK*** currently Envoy sends local replies in cases where an error ought to be
  // surfaced via the error path. There are ways we can clean up Envoy's local reply path to
  // make this possible, but nothing expedient. For the immediate term this is our only real
  // option. See https://github.com/lyft/envoy-mobile/issues/460

  // Error path: missing EnvoyUpstreamServiceTime implies this is a local reply, which we treat as
  // a stream error.
  if (!success_ && headers.get(Headers::get().EnvoyUpstreamServiceTime).empty()) {
    ENVOY_LOG(debug, "[S{}] intercepted local response", direct_stream_.stream_handle_);
    mapLocalResponseToError(headers);
    if (end_stream) {
      onError();
    }
    return HttpFilterHeadersStatus::StopIteration;
  }

return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus LocalErrorFilter::encodeData(Buffer::Instance& data, bool end_stream) {
  // Error path.
  if (error_code_.has_value()) {
    ASSERT(end_stream,
           "local response has to end the stream with a single data frame. If Envoy changes "
           "this expectation, this code needs to be updated.");
    encoder_callbacks_.sendLocalReply(); 
    return Http:FilterDataStatus::StopIteration;
  }

  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus LocalErrorFilter::encodeTrailers(Http::ResponseTrailerMap& /*trailers*/) {
  if (error_code_.has_value()) {
    return Http::FilterTrailersStatus::StopIteration;
  }
  return Http::FilterTrailersStatus::Continue;
}

void Dispatcher::DirectStreamCallbacks::mapLocalResponseToError(ResponseHeaderMap& headers) {
  // Deal with a local response based on the HTTP status code received. Envoy Mobile treats
  // successful local responses as actual success. Envoy Mobile surfaces non-200 local responses as
  // errors via callbacks rather than an HTTP response. This is inline with behaviour of other
  // mobile networking libraries.
  switch (Utility::getResponseStatus(headers)) {
  case 503:
    error_code_ = ENVOY_CONNECTION_FAILURE;
    break;
  default:
    error_code_ = ENVOY_UNDEFINED_ERROR;
  }

  uint32_t attempt_count;
  if (headers.EnvoyAttemptCount() &&
      absl::SimpleAtoi(headers.EnvoyAttemptCount()->value().getStringView(), &attempt_count)) {
    error_attempt_count_ = attempt_count;
  }
}

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
