#include <unity.h>
#include <string.h>
#include "config_parser.h"

static ConfigParser parser;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// Null / empty input
// ---------------------------------------------------------------------------

void test_null_text_returns_false()
{
  char buf[16] = "unchanged";
  TEST_ASSERT_FALSE(parser.findValue(nullptr, "KEY", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("unchanged", buf); // buf must not be modified
}

void test_empty_text_returns_false()
{
  char buf[16];
  TEST_ASSERT_FALSE(parser.findValue("", "KEY", buf, sizeof(buf)));
}

void test_null_key_returns_false()
{
  char buf[16];
  TEST_ASSERT_FALSE(parser.findValue("KEY=value\n", nullptr, buf, sizeof(buf)));
}

void test_null_out_buf_returns_false()
{
  TEST_ASSERT_FALSE(parser.findValue("KEY=value\n", "KEY", nullptr, 16));
}

void test_zero_out_len_returns_false()
{
  char buf[16];
  TEST_ASSERT_FALSE(parser.findValue("KEY=value\n", "KEY", buf, 0));
}

// ---------------------------------------------------------------------------
// Basic key lookup
// ---------------------------------------------------------------------------

void test_single_key_found()
{
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue("DATA_MODE=NC\n", "DATA_MODE", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("NC", buf);
}

void test_key_not_present_returns_false()
{
  char buf[32];
  TEST_ASSERT_FALSE(parser.findValue("DATA_MODE=NC\n", "LOG_LEVEL", buf, sizeof(buf)));
}

void test_multiple_keys_correct_value_returned()
{
  const char *text =
      "DATA_MODE=NC\n"
      "LOG_LEVEL=DEBUG\n"
      "WIFI_SSID=MyNet\n";

  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("DEBUG", buf);
}

void test_first_key_in_file()
{
  const char *text = "LOG_LEVEL=WARN\nDATA_MODE=LS\n";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("WARN", buf);
}

void test_last_key_in_file_without_trailing_newline()
{
  const char *text = "DATA_MODE=NC\nLOG_LEVEL=ERROR";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("ERROR", buf);
}

// ---------------------------------------------------------------------------
// Comment and blank line handling
// ---------------------------------------------------------------------------

void test_comment_line_is_skipped()
{
  const char *text =
      "# This is a comment\n"
      "LOG_LEVEL=INFO\n";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("INFO", buf);
}

void test_key_in_comment_is_not_matched()
{
  const char *text =
      "# LOG_LEVEL=DEBUG\n"
      "LOG_LEVEL=WARN\n";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("WARN", buf); // comment must be ignored
}

void test_blank_lines_are_skipped()
{
  const char *text = "\n\nLOG_LEVEL=INFO\n\n";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("INFO", buf);
}

// ---------------------------------------------------------------------------
// CRLF line endings
// ---------------------------------------------------------------------------

void test_crlf_line_endings_handled()
{
  const char *text = "DATA_MODE=NC\r\nLOG_LEVEL=DEBUG\r\n";
  char buf[32];
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("DEBUG", buf);
}

// ---------------------------------------------------------------------------
// Key prefix collision — "LOG" must not match "LOG_LEVEL"
// ---------------------------------------------------------------------------

void test_key_prefix_does_not_match_longer_key()
{
  const char *text = "LOG_LEVEL=INFO\n";
  char buf[32];
  TEST_ASSERT_FALSE(parser.findValue(text, "LOG", buf, sizeof(buf)));
}

void test_longer_key_does_not_match_prefix()
{
  const char *text = "LOG=INFO\n";
  char buf[32];
  TEST_ASSERT_FALSE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
}

// ---------------------------------------------------------------------------
// Output buffer truncation
// ---------------------------------------------------------------------------

void test_value_truncated_when_buffer_too_small()
{
  const char *text = "LOG_LEVEL=DEBUG\n";
  char buf[4]; // only fits "DEB\0"
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_INT('\0', buf[3]); // always null-terminated
  TEST_ASSERT_EQUAL_INT(3, (int)strlen(buf));
}

// ---------------------------------------------------------------------------
// Empty value
// ---------------------------------------------------------------------------

void test_empty_value_is_copied_as_empty_string()
{
  const char *text = "LOG_LEVEL=\n";
  char buf[16] = "previous";
  TEST_ASSERT_TRUE(parser.findValue(text, "LOG_LEVEL", buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("", buf);
}

// ---------------------------------------------------------------------------
// Entry point (native platform)
// ---------------------------------------------------------------------------

int main()
{
  UNITY_BEGIN();

  // Null / empty input
  RUN_TEST(test_null_text_returns_false);
  RUN_TEST(test_empty_text_returns_false);
  RUN_TEST(test_null_key_returns_false);
  RUN_TEST(test_null_out_buf_returns_false);
  RUN_TEST(test_zero_out_len_returns_false);

  // Basic key lookup
  RUN_TEST(test_single_key_found);
  RUN_TEST(test_key_not_present_returns_false);
  RUN_TEST(test_multiple_keys_correct_value_returned);
  RUN_TEST(test_first_key_in_file);
  RUN_TEST(test_last_key_in_file_without_trailing_newline);

  // Comment and blank line handling
  RUN_TEST(test_comment_line_is_skipped);
  RUN_TEST(test_key_in_comment_is_not_matched);
  RUN_TEST(test_blank_lines_are_skipped);

  // CRLF line endings
  RUN_TEST(test_crlf_line_endings_handled);

  // Key prefix collision
  RUN_TEST(test_key_prefix_does_not_match_longer_key);
  RUN_TEST(test_longer_key_does_not_match_prefix);

  // Output buffer truncation
  RUN_TEST(test_value_truncated_when_buffer_too_small);

  // Empty value
  RUN_TEST(test_empty_value_is_copied_as_empty_string);

  return UNITY_END();
}
