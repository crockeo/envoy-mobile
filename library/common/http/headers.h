#pragma once

namespace Envoy {
namespace Http {

/**
 * Constant HTTP headers used internally for in-band signalling in the request/response path.
 */
class InternalHeaderValues
public:
  const LowerCaseString InternalErrorCode{"x-internal-error-code"};
  const LowerCaseString InternalErrorMessage{"x-internal-error-message"};
}

} // namespace Http
} // namespace Envo 
