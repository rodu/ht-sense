#include <unity.h>
#include <string.h>
#include "logger.h"

// ---------------------------------------------------------------------------
// Capture infrastructure — function-pointer sink that records output
// ---------------------------------------------------------------------------

static char capturedLine[Logger::LOG_LINE_MAX];
static int captureCount = 0;

static void captureOutput(const char *line)
{
  strncpy(capturedLine, line, sizeof(capturedLine) - 1);
  capturedLine[sizeof(capturedLine) - 1] = '\0';
  captureCount++;
}

static void noopSink(const char *) {}

void setUp()
{
  capturedLine[0] = '\0';
  captureCount = 0;
  Log.setSink(captureOutput);
  Log.setLevel(LogLevel::DEBUG); // capture everything by default
}

void tearDown()
{
  Log.setSink(nullptr);
  Log.setLevel(LogLevel::INFO);
}

// ---------------------------------------------------------------------------
// Sink management
// ---------------------------------------------------------------------------

void test_null_sink_does_not_crash()
{
  Log.setSink(nullptr);
  // None of these should crash or assert.
  Log.debug("d");
  Log.info("i");
  Log.warn("w");
  Log.error("e");
  TEST_PASS();
}

void test_sink_is_called_for_passing_message()
{
  Log.info("hello");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
}

void test_sink_can_be_replaced_at_runtime()
{
  Log.setSink(noopSink);
  Log.info("ignored");
  TEST_ASSERT_EQUAL_INT(0, captureCount); // captureOutput was replaced

  Log.setSink(captureOutput);
  Log.info("captured");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
}

// ---------------------------------------------------------------------------
// Level filtering
// ---------------------------------------------------------------------------

void test_getLevel_returns_set_level()
{
  Log.setLevel(LogLevel::WARN);
  TEST_ASSERT_EQUAL(LogLevel::WARN, Log.getLevel());
  Log.setLevel(LogLevel::DEBUG);
}

void test_message_at_min_level_passes()
{
  Log.setLevel(LogLevel::WARN);
  Log.warn("threshold message");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
}

void test_message_above_min_level_passes()
{
  Log.setLevel(LogLevel::WARN);
  Log.error("above threshold");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
}

void test_message_below_min_level_is_dropped()
{
  Log.setLevel(LogLevel::WARN);
  Log.info("below threshold");
  Log.debug("also below");
  TEST_ASSERT_EQUAL_INT(0, captureCount);
}

void test_none_level_suppresses_all()
{
  Log.setLevel(LogLevel::NONE);
  Log.debug("d");
  Log.info("i");
  Log.warn("w");
  Log.error("e");
  TEST_ASSERT_EQUAL_INT(0, captureCount);
}

void test_debug_level_passes_all()
{
  Log.setLevel(LogLevel::DEBUG);
  Log.debug("d");
  Log.info("i");
  Log.warn("w");
  Log.error("e");
  TEST_ASSERT_EQUAL_INT(4, captureCount);
}

// ---------------------------------------------------------------------------
// Output format — each level must carry the correct prefix
// ---------------------------------------------------------------------------

void test_debug_prefix()
{
  Log.debug("msg");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[DEBUG]"));
}

void test_info_prefix()
{
  Log.info("msg");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[INFO]"));
}

void test_warn_prefix()
{
  Log.warn("msg");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[WARN]"));
}

void test_error_prefix()
{
  Log.error("msg");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[ERROR]"));
}

void test_message_body_appears_in_output()
{
  Log.info("sensor initialised");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "sensor initialised"));
}

void test_prefix_and_body_both_appear()
{
  Log.warn("low memory");
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[WARN]"));
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "low memory"));
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

void test_null_message_does_not_crash()
{
  Log.info(nullptr);
  TEST_ASSERT_EQUAL_INT(0, captureCount); // null message must be silently dropped
}

void test_empty_message_is_forwarded()
{
  Log.info("");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[INFO]"));
}

void test_long_message_is_truncated_safely()
{
  // Feed a message longer than LOG_LINE_MAX; should not overflow buffer.
  char longMsg[Logger::LOG_LINE_MAX * 2];
  memset(longMsg, 'A', sizeof(longMsg) - 1);
  longMsg[sizeof(longMsg) - 1] = '\0';

  Log.info(longMsg); // must not crash or overflow
  TEST_ASSERT_EQUAL_INT(1, captureCount);
  // Output must be null-terminated within the buffer bounds.
  TEST_ASSERT_LESS_OR_EQUAL(Logger::LOG_LINE_MAX - 1,
                            strlen(capturedLine));
}

void test_sink_called_exactly_once_per_log_call()
{
  Log.info("once");
  Log.info("twice");
  Log.info("three");
  TEST_ASSERT_EQUAL_INT(3, captureCount);
}

// ---------------------------------------------------------------------------
// Level changes take effect immediately (no buffering of level changes)
// ---------------------------------------------------------------------------

void test_level_change_takes_effect_immediately()
{
  Log.setLevel(LogLevel::ERROR);
  Log.warn("dropped");
  TEST_ASSERT_EQUAL_INT(0, captureCount);

  Log.setLevel(LogLevel::DEBUG);
  Log.warn("now passes");
  TEST_ASSERT_EQUAL_INT(1, captureCount);
}

// ---------------------------------------------------------------------------
// Entry point (native platform)
// ---------------------------------------------------------------------------

int main()
{
  UNITY_BEGIN();

  // Sink management
  RUN_TEST(test_null_sink_does_not_crash);
  RUN_TEST(test_sink_is_called_for_passing_message);
  RUN_TEST(test_sink_can_be_replaced_at_runtime);

  // Level filtering
  RUN_TEST(test_getLevel_returns_set_level);
  RUN_TEST(test_message_at_min_level_passes);
  RUN_TEST(test_message_above_min_level_passes);
  RUN_TEST(test_message_below_min_level_is_dropped);
  RUN_TEST(test_none_level_suppresses_all);
  RUN_TEST(test_debug_level_passes_all);

  // Output format
  RUN_TEST(test_debug_prefix);
  RUN_TEST(test_info_prefix);
  RUN_TEST(test_warn_prefix);
  RUN_TEST(test_error_prefix);
  RUN_TEST(test_message_body_appears_in_output);
  RUN_TEST(test_prefix_and_body_both_appear);

  // Edge cases
  RUN_TEST(test_null_message_does_not_crash);
  RUN_TEST(test_empty_message_is_forwarded);
  RUN_TEST(test_long_message_is_truncated_safely);
  RUN_TEST(test_sink_called_exactly_once_per_log_call);
  RUN_TEST(test_level_change_takes_effect_immediately);

  return UNITY_END();
}
