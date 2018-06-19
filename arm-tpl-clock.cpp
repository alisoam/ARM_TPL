#include <arm-tpl.h>
#include "tpl.h"
#include <ctime>

void ARMTPLClockInit()
{
}

extern "C" int __ARM_TPL_clock_realtime(timespec* __ts)
{
  unsigned int t = std::time(nullptr);
  __ts->tv_sec = t;
  __ts->tv_nsec = 0;
  return 0;
}

extern "C" int __ARM_TPL_clock_monotonic(timespec* __ts)
{
  unsigned int t = xTaskGetTickCount();
  __ts->tv_sec = t / configTICK_RATE_HZ;
  __ts->tv_nsec = (t % configTICK_RATE_HZ) * portTICK_RATE_MS * 1e6;
  return 0;
}
