// NOLINT(namespace-envoy)
#include <string>
#include <unordered_map>

#include "filter.pb.h"
#include "google/protobuf/util/json_util.h"
#include "proxy_wasm_intrinsics.h"

class AddHeaderRootContext : public RootContext {
public:
  explicit AddHeaderRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
  bool onConfigure(size_t /* configuration_size */) override;

  bool onStart(size_t) override;

  void onTick() override;

  std::string header_name_;
  std::string header_value_;
  WasmDataPtr headers_;
  WasmDataPtr body_;
  WasmDataPtr trailers_;
};

class AddHeaderContext : public Context {
public:
  explicit AddHeaderContext(uint32_t id, RootContext *root)
      : Context(id, root),
        root_(static_cast<AddHeaderRootContext *>(static_cast<void *>(root))) {}

  void onCreate() override;
  FilterHeadersStatus onRequestHeaders(uint32_t headers) override;
  FilterDataStatus onRequestBody(size_t body_buffer_length,
                                 bool end_of_stream) override;
  FilterHeadersStatus onResponseHeaders(uint32_t headers) override;
  FilterDataStatus onResponseBody(size_t body_buffer_lengt,
                                  bool end_of_stream) override;
  FilterTrailersStatus onResponseTrailers(uint32_t trailers) override;
  void onDone() override;
  void onLog() override;
  void onDelete() override;

private:
  AddHeaderRootContext *root_;
};
static RegisterContextFactory
    register_AddHeaderContext(CONTEXT_FACTORY(AddHeaderContext),
                              ROOT_FACTORY(AddHeaderRootContext),
                              "hold_rpc_root_id");

bool AddHeaderRootContext::onConfigure(size_t) {
  auto conf = getConfiguration();
  Config config;

  google::protobuf::util::JsonParseOptions options;
  options.case_insensitive_enum_parsing = true;
  options.ignore_unknown_fields = false;

  google::protobuf::util::JsonStringToMessage(conf->toString(), &config,
                                              options);
  LOG_DEBUG("onConfigure name " + config.name());
  LOG_DEBUG("onConfigure " + config.value());
  header_name_ = config.name();
  header_value_ = config.value();
  return true;
}

bool AddHeaderRootContext::onStart(size_t) {
  LOG_DEBUG("onStart");
  return true;
}

void AddHeaderRootContext::onTick() {
  proxy_set_tick_period_milliseconds(0);

  HeaderStringPairs additional_headers;
  sendLocalResponse(200, "success", body_->view(), additional_headers);
}

void AddHeaderContext::onCreate() {
  LOG_DEBUG(std::string("onCreate " + std::to_string(id())));
}

FilterHeadersStatus AddHeaderContext::onRequestHeaders(uint32_t) {
  LOG_DEBUG(std::string("onRequestHeaders ") + std::to_string(id()));
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus AddHeaderContext::onResponseHeaders(uint32_t) {
  LOG_DEBUG(std::string("onResponseHeaders ") + std::to_string(id()));
  addResponseHeader("hello", "world");

  root_->headers_ = getResponseHeaderPairs();
  return FilterHeadersStatus::Continue;
}

FilterDataStatus AddHeaderContext::onRequestBody(size_t body_buffer_length,
                                                 bool end_of_stream) {
  return FilterDataStatus::Continue;
}

FilterDataStatus AddHeaderContext::onResponseBody(size_t body_buffer_length,
                                                  bool end_of_stream) {
  root_->body_ =
      getBufferBytes(BufferType::HttpResponseBody, 0, body_buffer_length);
  return FilterDataStatus::Continue;
}

FilterTrailersStatus AddHeaderContext::onResponseTrailers(uint32_t trailers) {
  root_->trailers_ = getResponseTrailerPairs();
  proxy_set_effective_context(root_->id());
  proxy_set_tick_period_milliseconds(5000);
  proxy_set_effective_context(id());
  return FilterTrailersStatus::Continue;
}

void AddHeaderContext::onDone() {
  LOG_DEBUG(std::string("onDone " + std::to_string(id())));
}

void AddHeaderContext::onLog() {
  LOG_DEBUG(std::string("onLog " + std::to_string(id())));
}

void AddHeaderContext::onDelete() {
  LOG_DEBUG(std::string("onDelete " + std::to_string(id())));
}
