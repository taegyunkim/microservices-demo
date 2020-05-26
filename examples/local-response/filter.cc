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

  uint32_t held_context_id_;
  std::string header_name_;
  std::string header_value_;
};

class AddHeaderContext : public Context {
public:
  explicit AddHeaderContext(uint32_t id, RootContext *root)
      : Context(id, root),
        root_(static_cast<AddHeaderRootContext *>(static_cast<void *>(root))) {}

  FilterHeadersStatus onResponseHeaders(uint32_t headers) override;
  FilterHeadersStatus onResponseTrailers(uint32_t trailers) override;

  void onDone() override;
  void onLog() override;
  void onDelete() override;

private:
  AddHeaderRootContext *root_;
};
static RegisterContextFactory
    register_AddHeaderContext(CONTEXT_FACTORY(AddHeaderContext),
                              ROOT_FACTORY(AddHeaderRootContext),
                              "add_header_root_id");

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
  if (getContext(held_context_id_) != nullptr) {
    proxy_set_effective_context(held_context_id_);
    continueResponse();
  }
}

FilterHeadersStatus AddHeaderContext::onResponseHeaders(uint32_t) {
  LOG_DEBUG(std::string("onResponseHeaders ") + std::to_string(id()));
  addResponseHeader("hello", "world");
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus AddHeaderContext::onResponseTrailers(uint32_t) {
  uint32_t current_id = id();
  root_->held_context_id_ = current_id;
  proxy_set_tick_period_milliseconds(5000);
  return FilterHeadersStatus::StopIteration;
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
