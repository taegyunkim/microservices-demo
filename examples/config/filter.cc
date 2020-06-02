// NOLINT(namespace-envoy)
#include <string>
#include <unordered_map>
#include <vector>

#include "filter.pb.h"
#include "google/protobuf/util/json_util.h"
#include "proxy_wasm_intrinsics.h"

class AddHeaderRootContext : public RootContext {
public:
  explicit AddHeaderRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
  bool onConfigure(size_t /* configuration_size */) override;

  std::vector<std::string> methods_;
};

class AddHeaderContext : public Context {
public:
  explicit AddHeaderContext(uint32_t id, RootContext *root)
      : Context(id, root),
        root_(static_cast<AddHeaderRootContext *>(static_cast<void *>(root))) {}

  FilterHeadersStatus onResponseHeaders(uint32_t headers) override;

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
  // NOTE(taegyun): This function seems to be called multiple times, so populate
  // methods_ just once.
  if (methods_.empty()) {
    for (const auto &method : config.methods()) {
      LOG_DEBUG(method);
      methods_.push_back(method);
    }
  }
  return true;
}

FilterHeadersStatus AddHeaderContext::onResponseHeaders(uint32_t) {
  LOG_DEBUG(std::string("onResponseHeaders ") + std::to_string(id()));
  addResponseHeader("hello", "world");
  std::string methods;
  LOG_DEBUG("Num elements in methods variable: " +
            std::to_string(root_->methods_.size()));
  for (const auto &method : root_->methods_) {
    methods += method;
  }
  replaceResponseHeader("methods", methods);
  LOG_DEBUG("methods: " + methods);
  return FilterHeadersStatus::Continue;
}
