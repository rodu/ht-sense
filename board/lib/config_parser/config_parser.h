#pragma once
#include <stddef.h>

/**
 * Parser for .env-style key=value configuration text.
 *
 * Accepts a null-terminated buffer containing one key=value pair per line.
 * Lines beginning with '#' are treated as comments and are skipped.
 * Empty lines are ignored.
 * Key matching is exact and case-sensitive.
 *
 * No heap allocation is used; output is written into caller-supplied buffers.
 */
class ConfigParser
{
public:
  /**
   * Search text for a line matching KEY=value and copy the value.
   *
   * @param text     Null-terminated key=value text. May be nullptr or empty.
   * @param key      Key to search for (case-sensitive, without trailing '=').
   * @param out_buf  Buffer to receive the null-terminated value string.
   * @param out_len  Size of out_buf in bytes (must be >= 1).
   * @returns        true if the key was found and the value copied;
   *                 false if the key was not found or inputs are invalid.
   *                 When false, out_buf is not modified.
   */
  bool findValue(const char *text, const char *key,
                 char *out_buf, size_t out_len) const;
};
