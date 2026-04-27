#include <unity.h>
#include <string.h>
#include "config_parser.h"

static ConfigParser parser;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// defaultConfig — safe defaults
// ---------------------------------------------------------------------------

void test_defaultConfig_dataMode_is_NC()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_EQUAL_STRING("NC", cfg.dataMode);
}

void test_defaultConfig_logLevel_is_INFO()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_EQUAL_STRING("INFO", cfg.logLevel);
}

void test_defaultConfig_mqttTopic_preset()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_EQUAL_STRING("ht-sense/data", cfg.mqttTopic);
}

void test_defaultConfig_credentials_empty()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_EQUAL_STRING("", cfg.wifiSsid);
  TEST_ASSERT_EQUAL_STRING("", cfg.wifiPassword);
  TEST_ASSERT_EQUAL_STRING("", cfg.syncToken);
}

// ---------------------------------------------------------------------------
// parse() — null / empty / invalid input
// ---------------------------------------------------------------------------

void test_parse_null_text_returns_false()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_FALSE(parser.parse(nullptr, 10, cfg));
}

void test_parse_zero_len_returns_false()
{
  AppConfig cfg = defaultConfig();
  TEST_ASSERT_FALSE(parser.parse("{\"dataMode\":\"LS\"}", 0, cfg));
}

void test_parse_invalid_json_returns_false()
{
  AppConfig cfg = defaultConfig();
  const char *bad = "not-json-at-all!!!";
  TEST_ASSERT_FALSE(parser.parse(bad, strlen(bad), cfg));
}

void test_parse_empty_json_object_returns_false()
{
  AppConfig cfg = defaultConfig();
  const char *empty = "{}";
  // Valid JSON but no recognised fields — should return false
  TEST_ASSERT_FALSE(parser.parse(empty, strlen(empty), cfg));
}

// ---------------------------------------------------------------------------
// parse() — top-level scalar fields
// ---------------------------------------------------------------------------

void test_parse_dataMode_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"dataMode\":\"LS\"}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("LS", cfg.dataMode);
}

void test_parse_logLevel_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"logLevel\":\"DEBUG\"}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("DEBUG", cfg.logLevel);
}

// ---------------------------------------------------------------------------
// parse() — nested wifi object
// ---------------------------------------------------------------------------

void test_parse_wifi_ssid_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"wifi\":{\"ssid\":\"HomeNet\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("HomeNet", cfg.wifiSsid);
}

void test_parse_wifi_password_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"wifi\":{\"password\":\"s3cr3t\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("s3cr3t", cfg.wifiPassword);
}

// ---------------------------------------------------------------------------
// parse() — nested mqtt object
// ---------------------------------------------------------------------------

void test_parse_mqtt_broker_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"mqtt\":{\"broker\":\"192.168.1.100\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("192.168.1.100", cfg.mqttBroker);
}

void test_parse_mqtt_topic_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"mqtt\":{\"topic\":\"sensors/ht\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("sensors/ht", cfg.mqttTopic);
}

// ---------------------------------------------------------------------------
// parse() — nested sync object
// ---------------------------------------------------------------------------

void test_parse_sync_url_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"sync\":{\"url\":\"https://example.com/upload\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("https://example.com/upload", cfg.syncUrl);
}

void test_parse_sync_token_field()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"sync\":{\"token\":\"tok_abc123\"}}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("tok_abc123", cfg.syncToken);
}

// ---------------------------------------------------------------------------
// parse() — complete config with all fields
// ---------------------------------------------------------------------------

void test_parse_complete_config()
{
  AppConfig cfg = defaultConfig();
  const char *json =
      "{"
      "\"dataMode\":\"RT\","
      "\"logLevel\":\"WARN\","
      "\"wifi\":{\"ssid\":\"MyNet\",\"password\":\"p@ss\"},"
      "\"mqtt\":{\"broker\":\"10.0.0.1\",\"topic\":\"ht/live\"},"
      "\"sync\":{\"url\":\"https://api.example.com\",\"token\":\"abc\"}"
      "}";

  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("RT", cfg.dataMode);
  TEST_ASSERT_EQUAL_STRING("WARN", cfg.logLevel);
  TEST_ASSERT_EQUAL_STRING("MyNet", cfg.wifiSsid);
  TEST_ASSERT_EQUAL_STRING("p@ss", cfg.wifiPassword);
  TEST_ASSERT_EQUAL_STRING("10.0.0.1", cfg.mqttBroker);
  TEST_ASSERT_EQUAL_STRING("ht/live", cfg.mqttTopic);
  TEST_ASSERT_EQUAL_STRING("https://api.example.com", cfg.syncUrl);
  TEST_ASSERT_EQUAL_STRING("abc", cfg.syncToken);
}

// ---------------------------------------------------------------------------
// parse() — "Default & Override": missing fields keep their defaults
// ---------------------------------------------------------------------------

void test_parse_partial_json_leaves_other_defaults()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"dataMode\":\"LS\"}";
  parser.parse(json, strlen(json), cfg);

  // Updated field
  TEST_ASSERT_EQUAL_STRING("LS", cfg.dataMode);
  // Untouched fields keep defaults
  TEST_ASSERT_EQUAL_STRING("INFO", cfg.logLevel);
  TEST_ASSERT_EQUAL_STRING("ht-sense/data", cfg.mqttTopic);
  TEST_ASSERT_EQUAL_STRING("", cfg.wifiSsid);
}

void test_parse_unknown_fields_are_ignored()
{
  AppConfig cfg = defaultConfig();
  const char *json = "{\"unknownKey\":\"value\",\"dataMode\":\"SS\"}";
  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  TEST_ASSERT_EQUAL_STRING("SS", cfg.dataMode);
}

// ---------------------------------------------------------------------------
// parse() — long value truncated safely (no buffer overrun)
// ---------------------------------------------------------------------------

void test_parse_long_value_truncated_safely()
{
  // syncUrl buffer is 128 bytes; supply a value longer than that.
  AppConfig cfg = defaultConfig();
  const char *json =
      "{\"sync\":{\"url\":\"https://very-long-hostname.example.com/api/endpoint"
      "/that/exceeds/the/buffer/size/limit/quite/significantly/upload\"}}";

  TEST_ASSERT_TRUE(parser.parse(json, strlen(json), cfg));
  // Must be null-terminated and fit within the buffer
  TEST_ASSERT_EQUAL_INT('\0', cfg.syncUrl[sizeof(cfg.syncUrl) - 1]);
  TEST_ASSERT_LESS_OR_EQUAL(sizeof(cfg.syncUrl) - 1, strlen(cfg.syncUrl));
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  // defaultConfig
  RUN_TEST(test_defaultConfig_dataMode_is_NC);
  RUN_TEST(test_defaultConfig_logLevel_is_INFO);
  RUN_TEST(test_defaultConfig_mqttTopic_preset);
  RUN_TEST(test_defaultConfig_credentials_empty);

  // parse() — invalid input
  RUN_TEST(test_parse_null_text_returns_false);
  RUN_TEST(test_parse_zero_len_returns_false);
  RUN_TEST(test_parse_invalid_json_returns_false);
  RUN_TEST(test_parse_empty_json_object_returns_false);

  // parse() — individual fields
  RUN_TEST(test_parse_dataMode_field);
  RUN_TEST(test_parse_logLevel_field);
  RUN_TEST(test_parse_wifi_ssid_field);
  RUN_TEST(test_parse_wifi_password_field);
  RUN_TEST(test_parse_mqtt_broker_field);
  RUN_TEST(test_parse_mqtt_topic_field);
  RUN_TEST(test_parse_sync_url_field);
  RUN_TEST(test_parse_sync_token_field);

  // parse() — composite / edge cases
  RUN_TEST(test_parse_complete_config);
  RUN_TEST(test_parse_partial_json_leaves_other_defaults);
  RUN_TEST(test_parse_unknown_fields_are_ignored);
  RUN_TEST(test_parse_long_value_truncated_safely);

  return UNITY_END();
}
