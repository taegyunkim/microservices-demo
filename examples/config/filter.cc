// NOLINT(namespace-envoy)
#include <cstdlib>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "filter.pb.h"
#include "google/protobuf/util/json_util.h"
#include "proxy_wasm_intrinsics.h"

class AddHeaderRootContext : public RootContext {
public:
  explicit AddHeaderRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
  bool onConfigure(size_t /* configuration_size */) override;

  std::vector<std::pair<std::string, std::string>> header_regexes_;
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

bool AddHeaderRootContext::onConfigure(size_t) {
  auto conf = getConfiguration();
  Config config;

  google::protobuf::util::JsonParseOptions options;
  options.case_insensitive_enum_parsing = true;
  options.ignore_unknown_fields = false;

  google::protobuf::util::JsonStringToMessage(conf->toString(), &config,
                                              options);
  // NOTE(taegyun): This function seems to be called multiple times, so populate
  // header_regexes just once.
  if (header_regexes_.empty()) {
    for (const auto &header : config.headers()) {
      header_regexes_.push_back(
          std::make_pair(header.name(), header.value_regex()));
    }
  }
  return true;
}

FilterHeadersStatus AddHeaderContext::onRequestHeaders(uint32_t) {
  auto tagged = getRequestHeader("x-tagged");
  if (tagged->data() != nullptr && tagged->size() > 0) {
    LOG_DEBUG("Already tagged");
    return FilterHeadersStatus::Continue;
  }
  for (const auto &header_regex : root_->header_regexes_) {
    auto value = getRequestHeader(header_regex.first);
    if (value->data() != nullptr && value->size() > 0) {
      const auto &regex = header_regex.second;
      if (std::regex_search(value->data(), std::regex(regex))) {
        addRequestHeader("x-tagged", "true");
        replaceRequestHeader("x-b3-sampled", "1");
        LOG_DEBUG("Set x-b3-sampled to 1.");
        break;
      } else {
        LOG_DEBUG("Pattern: " + regex + ", value: " + value->toString());
      }
    } else {
      LOG_DEBUG("Didn't find header " + header_regex.first);
    }
  }

  auto result = getRequestHeaderPairs();
  auto pairs = result->pairs();
  for (const auto &p : pairs) {
    LOG_DEBUG(std::string(p.first) + std::string(" -> ") +
              std::string(p.second));
  }

  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus AddHeaderContext::onResponseHeaders(uint32_t) {
  auto result = getResponseHeaderPairs();
  auto pairs = result->pairs();
  for (const auto &p : pairs) {
    LOG_DEBUG(std::string(p.first) + std::string(" -> ") +
              std::string(p.second));
  }

  // Sanity check this filter is installed and running.
  addResponseHeader("hello", "world");

  return FilterHeadersStatus::Continue;
}
