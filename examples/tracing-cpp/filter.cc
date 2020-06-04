// NOLINT(namespace-envoy)
#include <string>
#include <unordered_map>

#include "google/protobuf/util/json_util.h"
#include "proxy_wasm_intrinsics.h"

class AddHeaderRootContext : public RootContext {
public:
  explicit AddHeaderRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
};

class AddHeaderContext : public Context {
public:
  explicit AddHeaderContext(uint32_t id, RootContext *root)
      : Context(id, root),
        root_(static_cast<AddHeaderRootContext *>(static_cast<void *>(root))) {}

  FilterHeadersStatus onRequestHeaders(uint32_t headers) override;
  FilterHeadersStatus onResponseHeaders(uint32_t headers) override;

private:
  AddHeaderRootContext *root_;
};
static RegisterContextFactory
    register_AddHeaderContext(CONTEXT_FACTORY(AddHeaderContext),
                              ROOT_FACTORY(AddHeaderRootContext),
                              "add_header_root_id");

FilterHeadersStatus AddHeaderContext::onRequestHeaders(uint32_t) {
  if (getRequestHeader("x-wasm-trace-id")->data() == nullptr) {
    addRequestHeader("x-wasm-trace-id", std::to_string(id()));
  }
  auto request_header_pairs = getRequestHeaderPairs()->pairs();
  for (const auto &p : request_header_pairs) {
    LOG_DEBUG(std::string(p.first) + " -> " + std::string(p.second));
  }

  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus AddHeaderContext::onResponseHeaders(uint32_t) {
  addResponseHeader("hello", "world");

  auto response_header_pairs = getResponseHeaderPairs()->pairs();
  for (const auto &p : response_header_pairs) {
    LOG_DEBUG(std::string(p.first) + " -> " + std::string(p.second));
  }

  return FilterHeadersStatus::Continue;
}
