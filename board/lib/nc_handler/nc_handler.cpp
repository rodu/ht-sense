#include "nc_handler.h"
#include "logger.h"

void NcHandler::handle(const SensorReading &reading)
{
  (void)reading; // intentional discard — suppress unused-parameter warning
  _discardedCount++;
  Log.debug("NC: reading discarded");
}

uint32_t NcHandler::discardedCount() const
{
  return _discardedCount;
}

void NcHandler::resetCount()
{
  _discardedCount = 0;
}
