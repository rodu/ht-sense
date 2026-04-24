#include "data_mode.h"
#include <string.h>

DataMode parseDataMode(const char *str)
{
  if (str == nullptr || str[0] == '\0')
    return DataMode::UNKNOWN;
  if (strcmp(str, "NC") == 0)
    return DataMode::NC;
  if (strcmp(str, "LS") == 0)
    return DataMode::LS;
  if (strcmp(str, "SS") == 0)
    return DataMode::SS;
  if (strcmp(str, "RT") == 0)
    return DataMode::RT;
  if (strcmp(str, "LSRT") == 0)
    return DataMode::LSRT;
  if (strcmp(str, "SSRT") == 0)
    return DataMode::SSRT;
  return DataMode::UNKNOWN;
}

bool supportsLocalStorage(DataMode mode)
{
  switch (mode)
  {
  case DataMode::LS:
  case DataMode::SS:
  case DataMode::LSRT:
  case DataMode::SSRT:
    return true;
  default:
    return false;
  }
}

bool supportsRealtime(DataMode mode)
{
  switch (mode)
  {
  case DataMode::RT:
  case DataMode::LSRT:
  case DataMode::SSRT:
    return true;
  default:
    return false;
  }
}

bool supportsSync(DataMode mode)
{
  switch (mode)
  {
  case DataMode::SS:
  case DataMode::SSRT:
    return true;
  default:
    return false;
  }
}

DataMode fallbackModeWhenOffline(DataMode mode)
{
  switch (mode)
  {
  case DataMode::NC:
  case DataMode::LS:
    return mode; // already network-independent
  case DataMode::SS:
  case DataMode::RT:
  case DataMode::LSRT:
  case DataMode::SSRT:
    return DataMode::LS; // preserve collected data locally
  default:
    return DataMode::NC; // unknown mode: safest is to collect nothing
  }
}

const char *dataModeToString(DataMode mode)
{
  switch (mode)
  {
  case DataMode::NC:
    return "NC";
  case DataMode::LS:
    return "LS";
  case DataMode::SS:
    return "SS";
  case DataMode::RT:
    return "RT";
  case DataMode::LSRT:
    return "LSRT";
  case DataMode::SSRT:
    return "SSRT";
  default:
    return "UNKNOWN";
  }
}
