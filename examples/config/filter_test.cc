#include "filter.pb.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"

// To run this test
// $> bazel --noworkspace_rc test :filter_test

TEST(FilterTest, ParseSingleMethod) {
  Config config;

  google::protobuf::util::JsonParseOptions options;
  options.case_insensitive_enum_parsing = true;
  options.ignore_unknown_fields = false;
  google::protobuf::util::JsonStringToMessage("{\"methods\": \"GET\"}", &config,
                                              options);

  ASSERT_EQ(config.methods().size(), 1);
  EXPECT_EQ(config.methods(0), "GET");
}

TEST(FilterTest, ParseMultiple) {
  Config config;

  google::protobuf::util::JsonParseOptions options;
  options.case_insensitive_enum_parsing = true;
  options.ignore_unknown_fields = false;
  google::protobuf::util::JsonStringToMessage(
      "{\"methods\": [\"GET\", \"POST\"] }", &config, options);

  ASSERT_EQ(config.methods().size(), 2);
  EXPECT_EQ(config.methods(0), "GET");
  EXPECT_EQ(config.methods(1), "SET");
}
