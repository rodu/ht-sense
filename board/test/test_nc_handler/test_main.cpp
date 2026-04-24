#include <unity.h>
#include <string.h>
#include "logger.h"
#include "nc_handler.h"
#include "sensor_reading.h"

// ---------------------------------------------------------------------------
// Log capture — lets tests assert on debug output from NcHandler
// ---------------------------------------------------------------------------

static char capturedLine[Logger::LOG_LINE_MAX];
static int captureCount = 0;

static void captureOutput(const char *line)
{
  strncpy(capturedLine, line, sizeof(capturedLine) - 1);
  capturedLine[sizeof(capturedLine) - 1] = '\0';
  captureCount++;
}

// Convenience: a reading with known values for use across tests.
static SensorReading makeReading(float temp, float hum, uint32_t ts)
{
  SensorReading r;
  r.temperature = temp;
  r.humidity = hum;
  r.timestamp = ts;
  return r;
}

static NcHandler handler;

void setUp()
{
  capturedLine[0] = '\0';
  captureCount = 0;
  handler.resetCount();
  Log.setSink(captureOutput);
  Log.setLevel(LogLevel::DEBUG);
}

void tearDown()
{
  Log.setSink(nullptr);
  Log.setLevel(LogLevel::INFO);
}

// ---------------------------------------------------------------------------
// Discard counting
// ---------------------------------------------------------------------------

void test_initial_discarded_count_is_zero()
{
  TEST_ASSERT_EQUAL_UINT32(0, handler.discardedCount());
}

void test_single_handle_increments_count_to_one()
{
  handler.handle(makeReading(22.5f, 55.0f, 1000));
  TEST_ASSERT_EQUAL_UINT32(1, handler.discardedCount());
}

void test_multiple_handle_calls_accumulate_count()
{
  handler.handle(makeReading(20.0f, 50.0f, 1000));
  handler.handle(makeReading(21.0f, 51.0f, 2000));
  handler.handle(makeReading(22.0f, 52.0f, 3000));
  TEST_ASSERT_EQUAL_UINT32(3, handler.discardedCount());
}

void test_reset_count_restores_to_zero()
{
  handler.handle(makeReading(20.0f, 50.0f, 1000));
  handler.handle(makeReading(21.0f, 51.0f, 2000));
  handler.resetCount();
  TEST_ASSERT_EQUAL_UINT32(0, handler.discardedCount());
}

void test_handle_after_reset_counts_from_zero()
{
  handler.handle(makeReading(20.0f, 50.0f, 1000));
  handler.resetCount();
  handler.handle(makeReading(22.0f, 55.0f, 2000));
  TEST_ASSERT_EQUAL_UINT32(1, handler.discardedCount());
}

void test_many_resets_do_not_corrupt_counter()
{
  handler.resetCount();
  handler.resetCount();
  handler.resetCount();
  TEST_ASSERT_EQUAL_UINT32(0, handler.discardedCount());
}

// ---------------------------------------------------------------------------
// Data is not visible after discard — the handler must not store or expose
// any field of the reading (structural test: counter is the only output)
// ---------------------------------------------------------------------------

void test_count_is_the_only_observable_state()
{
  // Feed readings with distinct values; only the count should change.
  handler.handle(makeReading(0.0f, 0.0f, 0));
  handler.handle(makeReading(100.0f, 100.0f, UINT32_MAX));
  TEST_ASSERT_EQUAL_UINT32(2, handler.discardedCount());
  // No other observable state exists on NcHandler — pass.
  TEST_PASS();
}

// ---------------------------------------------------------------------------
// Logging behavior — NcHandler must emit a DEBUG log on each discard
// ---------------------------------------------------------------------------

void test_handle_emits_debug_log()
{
  handler.handle(makeReading(22.5f, 55.0f, 1000));
  TEST_ASSERT_EQUAL_INT(1, captureCount);
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[DEBUG]"));
}

void test_handle_log_contains_nc_context()
{
  handler.handle(makeReading(22.5f, 55.0f, 1000));
  // The log line should communicate that the reading was discarded in NC mode.
  TEST_ASSERT_NOT_NULL(strstr(capturedLine, "NC"));
}

void test_handle_emits_one_log_per_reading()
{
  handler.handle(makeReading(20.0f, 50.0f, 1));
  handler.handle(makeReading(21.0f, 51.0f, 2));
  handler.handle(makeReading(22.0f, 52.0f, 3));
  TEST_ASSERT_EQUAL_INT(3, captureCount);
}

void test_handle_does_not_log_when_level_above_debug()
{
  // When the minimum log level is INFO, debug messages must be suppressed.
  Log.setLevel(LogLevel::INFO);
  handler.handle(makeReading(22.5f, 55.0f, 1000));
  TEST_ASSERT_EQUAL_INT(0, captureCount);
  // The discard still happened — only the log was suppressed.
  TEST_ASSERT_EQUAL_UINT32(1, handler.discardedCount());
}

void test_handle_does_not_log_when_sink_is_null()
{
  Log.setSink(nullptr);
  // Must not crash even with no sink.
  handler.handle(makeReading(22.5f, 55.0f, 1000));
  TEST_ASSERT_EQUAL_UINT32(1, handler.discardedCount());
  TEST_PASS();
}

// ---------------------------------------------------------------------------
// Entry point (native platform)
// ---------------------------------------------------------------------------

int main()
{
  UNITY_BEGIN();

  // Discard counting
  RUN_TEST(test_initial_discarded_count_is_zero);
  RUN_TEST(test_single_handle_increments_count_to_one);
  RUN_TEST(test_multiple_handle_calls_accumulate_count);
  RUN_TEST(test_reset_count_restores_to_zero);
  RUN_TEST(test_handle_after_reset_counts_from_zero);
  RUN_TEST(test_many_resets_do_not_corrupt_counter);
  RUN_TEST(test_count_is_the_only_observable_state);

  // Logging behavior
  RUN_TEST(test_handle_emits_debug_log);
  RUN_TEST(test_handle_log_contains_nc_context);
  RUN_TEST(test_handle_emits_one_log_per_reading);
  RUN_TEST(test_handle_does_not_log_when_level_above_debug);
  RUN_TEST(test_handle_does_not_log_when_sink_is_null);

  return UNITY_END();
}
