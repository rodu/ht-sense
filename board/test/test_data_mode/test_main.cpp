#include <unity.h>
#include "data_mode.h"

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// parseDataMode — valid strings
// ---------------------------------------------------------------------------

void test_parse_nc()
{
  TEST_ASSERT_EQUAL(DataMode::NC, parseDataMode("NC"));
}

void test_parse_ls()
{
  TEST_ASSERT_EQUAL(DataMode::LS, parseDataMode("LS"));
}

void test_parse_ss()
{
  TEST_ASSERT_EQUAL(DataMode::SS, parseDataMode("SS"));
}

void test_parse_rt()
{
  TEST_ASSERT_EQUAL(DataMode::RT, parseDataMode("RT"));
}

void test_parse_lsrt()
{
  TEST_ASSERT_EQUAL(DataMode::LSRT, parseDataMode("LSRT"));
}

void test_parse_ssrt()
{
  TEST_ASSERT_EQUAL(DataMode::SSRT, parseDataMode("SSRT"));
}

// ---------------------------------------------------------------------------
// parseDataMode — invalid input
// ---------------------------------------------------------------------------

void test_parse_null_returns_unknown()
{
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode(nullptr));
}

void test_parse_empty_string_returns_unknown()
{
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode(""));
}

void test_parse_lowercase_returns_unknown()
{
  // Parsing is case-sensitive; lowercase values are not accepted.
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("ls"));
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("nc"));
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("rt"));
}

void test_parse_mixed_case_returns_unknown()
{
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("Ls"));
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("sS"));
}

void test_parse_whitespace_not_trimmed()
{
  // Trimming is the caller's responsibility; raw whitespace is unknown.
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("LS "));
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode(" LS"));
}

void test_parse_garbage_returns_unknown()
{
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("INVALID"));
  TEST_ASSERT_EQUAL(DataMode::UNKNOWN, parseDataMode("123"));
}

// ---------------------------------------------------------------------------
// supportsLocalStorage
// ---------------------------------------------------------------------------

void test_local_storage_nc_false()
{
  TEST_ASSERT_FALSE(supportsLocalStorage(DataMode::NC));
}

void test_local_storage_ls_true()
{
  TEST_ASSERT_TRUE(supportsLocalStorage(DataMode::LS));
}

void test_local_storage_ss_true()
{
  TEST_ASSERT_TRUE(supportsLocalStorage(DataMode::SS));
}

void test_local_storage_rt_false()
{
  TEST_ASSERT_FALSE(supportsLocalStorage(DataMode::RT));
}

void test_local_storage_lsrt_true()
{
  TEST_ASSERT_TRUE(supportsLocalStorage(DataMode::LSRT));
}

void test_local_storage_ssrt_true()
{
  TEST_ASSERT_TRUE(supportsLocalStorage(DataMode::SSRT));
}

void test_local_storage_unknown_false()
{
  TEST_ASSERT_FALSE(supportsLocalStorage(DataMode::UNKNOWN));
}

// ---------------------------------------------------------------------------
// supportsRealtime
// ---------------------------------------------------------------------------

void test_realtime_nc_false()
{
  TEST_ASSERT_FALSE(supportsRealtime(DataMode::NC));
}

void test_realtime_ls_false()
{
  TEST_ASSERT_FALSE(supportsRealtime(DataMode::LS));
}

void test_realtime_ss_false()
{
  TEST_ASSERT_FALSE(supportsRealtime(DataMode::SS));
}

void test_realtime_rt_true()
{
  TEST_ASSERT_TRUE(supportsRealtime(DataMode::RT));
}

void test_realtime_lsrt_true()
{
  TEST_ASSERT_TRUE(supportsRealtime(DataMode::LSRT));
}

void test_realtime_ssrt_true()
{
  TEST_ASSERT_TRUE(supportsRealtime(DataMode::SSRT));
}

void test_realtime_unknown_false()
{
  TEST_ASSERT_FALSE(supportsRealtime(DataMode::UNKNOWN));
}

// ---------------------------------------------------------------------------
// supportsSync
// ---------------------------------------------------------------------------

void test_sync_nc_false()
{
  TEST_ASSERT_FALSE(supportsSync(DataMode::NC));
}

void test_sync_ls_false()
{
  TEST_ASSERT_FALSE(supportsSync(DataMode::LS));
}

void test_sync_ss_true()
{
  TEST_ASSERT_TRUE(supportsSync(DataMode::SS));
}

void test_sync_rt_false()
{
  TEST_ASSERT_FALSE(supportsSync(DataMode::RT));
}

void test_sync_lsrt_false()
{
  TEST_ASSERT_FALSE(supportsSync(DataMode::LSRT));
}

void test_sync_ssrt_true()
{
  TEST_ASSERT_TRUE(supportsSync(DataMode::SSRT));
}

void test_sync_unknown_false()
{
  TEST_ASSERT_FALSE(supportsSync(DataMode::UNKNOWN));
}

// ---------------------------------------------------------------------------
// fallbackModeWhenOffline
// ---------------------------------------------------------------------------

void test_fallback_nc_stays_nc()
{
  // NC has no network dependency; no change expected.
  TEST_ASSERT_EQUAL(DataMode::NC, fallbackModeWhenOffline(DataMode::NC));
}

void test_fallback_ls_stays_ls()
{
  // LS is already local-only; no change expected.
  TEST_ASSERT_EQUAL(DataMode::LS, fallbackModeWhenOffline(DataMode::LS));
}

void test_fallback_ss_becomes_ls()
{
  // SS cannot sync while offline; keep collecting locally.
  TEST_ASSERT_EQUAL(DataMode::LS, fallbackModeWhenOffline(DataMode::SS));
}

void test_fallback_rt_becomes_ls()
{
  // RT cannot stream while offline; buffer locally to avoid data loss.
  TEST_ASSERT_EQUAL(DataMode::LS, fallbackModeWhenOffline(DataMode::RT));
}

void test_fallback_lsrt_becomes_ls()
{
  // LSRT drops the RT stream; keep the local storage part.
  TEST_ASSERT_EQUAL(DataMode::LS, fallbackModeWhenOffline(DataMode::LSRT));
}

void test_fallback_ssrt_becomes_ls()
{
  // SSRT drops both the stream and sync; keep collecting locally.
  TEST_ASSERT_EQUAL(DataMode::LS, fallbackModeWhenOffline(DataMode::SSRT));
}

void test_fallback_unknown_becomes_nc()
{
  // Unknown mode falls back to the safest option: collect nothing.
  TEST_ASSERT_EQUAL(DataMode::NC, fallbackModeWhenOffline(DataMode::UNKNOWN));
}

// ---------------------------------------------------------------------------
// dataModeToString — round-trip with parseDataMode
// ---------------------------------------------------------------------------

void test_roundtrip_nc()
{
  TEST_ASSERT_EQUAL(DataMode::NC, parseDataMode(dataModeToString(DataMode::NC)));
}

void test_roundtrip_ls()
{
  TEST_ASSERT_EQUAL(DataMode::LS, parseDataMode(dataModeToString(DataMode::LS)));
}

void test_roundtrip_ss()
{
  TEST_ASSERT_EQUAL(DataMode::SS, parseDataMode(dataModeToString(DataMode::SS)));
}

void test_roundtrip_rt()
{
  TEST_ASSERT_EQUAL(DataMode::RT, parseDataMode(dataModeToString(DataMode::RT)));
}

void test_roundtrip_lsrt()
{
  TEST_ASSERT_EQUAL(DataMode::LSRT, parseDataMode(dataModeToString(DataMode::LSRT)));
}

void test_roundtrip_ssrt()
{
  TEST_ASSERT_EQUAL(DataMode::SSRT, parseDataMode(dataModeToString(DataMode::SSRT)));
}

// ---------------------------------------------------------------------------
// Entry point (native platform)
// ---------------------------------------------------------------------------

int main()
{
  UNITY_BEGIN();

  // Parsing — valid
  RUN_TEST(test_parse_nc);
  RUN_TEST(test_parse_ls);
  RUN_TEST(test_parse_ss);
  RUN_TEST(test_parse_rt);
  RUN_TEST(test_parse_lsrt);
  RUN_TEST(test_parse_ssrt);

  // Parsing — invalid input
  RUN_TEST(test_parse_null_returns_unknown);
  RUN_TEST(test_parse_empty_string_returns_unknown);
  RUN_TEST(test_parse_lowercase_returns_unknown);
  RUN_TEST(test_parse_mixed_case_returns_unknown);
  RUN_TEST(test_parse_whitespace_not_trimmed);
  RUN_TEST(test_parse_garbage_returns_unknown);

  // supportsLocalStorage
  RUN_TEST(test_local_storage_nc_false);
  RUN_TEST(test_local_storage_ls_true);
  RUN_TEST(test_local_storage_ss_true);
  RUN_TEST(test_local_storage_rt_false);
  RUN_TEST(test_local_storage_lsrt_true);
  RUN_TEST(test_local_storage_ssrt_true);
  RUN_TEST(test_local_storage_unknown_false);

  // supportsRealtime
  RUN_TEST(test_realtime_nc_false);
  RUN_TEST(test_realtime_ls_false);
  RUN_TEST(test_realtime_ss_false);
  RUN_TEST(test_realtime_rt_true);
  RUN_TEST(test_realtime_lsrt_true);
  RUN_TEST(test_realtime_ssrt_true);
  RUN_TEST(test_realtime_unknown_false);

  // supportsSync
  RUN_TEST(test_sync_nc_false);
  RUN_TEST(test_sync_ls_false);
  RUN_TEST(test_sync_ss_true);
  RUN_TEST(test_sync_rt_false);
  RUN_TEST(test_sync_lsrt_false);
  RUN_TEST(test_sync_ssrt_true);
  RUN_TEST(test_sync_unknown_false);

  // fallbackModeWhenOffline
  RUN_TEST(test_fallback_nc_stays_nc);
  RUN_TEST(test_fallback_ls_stays_ls);
  RUN_TEST(test_fallback_ss_becomes_ls);
  RUN_TEST(test_fallback_rt_becomes_ls);
  RUN_TEST(test_fallback_lsrt_becomes_ls);
  RUN_TEST(test_fallback_ssrt_becomes_ls);
  RUN_TEST(test_fallback_unknown_becomes_nc);

  // Round-trip
  RUN_TEST(test_roundtrip_nc);
  RUN_TEST(test_roundtrip_ls);
  RUN_TEST(test_roundtrip_ss);
  RUN_TEST(test_roundtrip_rt);
  RUN_TEST(test_roundtrip_lsrt);
  RUN_TEST(test_roundtrip_ssrt);

  return UNITY_END();
}
