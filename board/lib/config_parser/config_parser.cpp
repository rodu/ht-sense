#include "config_parser.h"
#include <string.h>

bool ConfigParser::findValue(const char *text, const char *key,
                             char *out_buf, size_t out_len) const
{
  if (text == nullptr || key == nullptr || out_buf == nullptr || out_len == 0)
    return false;

  size_t keyLen = strlen(key);
  const char *p = text;

  while (*p != '\0')
  {
    // Skip comment lines (lines starting with '#')
    if (*p == '#')
    {
      while (*p != '\0' && *p != '\n')
        p++;
      if (*p == '\n')
        p++;
      continue;
    }

    // Skip blank lines (\r and \n)
    if (*p == '\r' || *p == '\n')
    {
      p++;
      continue;
    }

    // Check for an exact KEY= match at the start of this line
    if (strncmp(p, key, keyLen) == 0 && p[keyLen] == '=')
    {
      const char *valueStart = p + keyLen + 1;

      // Find end of the value (newline or end of string)
      const char *valueEnd = valueStart;
      while (*valueEnd != '\0' && *valueEnd != '\n' && *valueEnd != '\r')
        valueEnd++;

      // Copy value, honouring the output buffer limit
      size_t valueLen = (size_t)(valueEnd - valueStart);
      if (valueLen >= out_len)
        valueLen = out_len - 1;

      strncpy(out_buf, valueStart, valueLen);
      out_buf[valueLen] = '\0';
      return true;
    }

    // Advance past this line
    while (*p != '\0' && *p != '\n')
      p++;
    if (*p == '\n')
      p++;
  }

  return false;
}
