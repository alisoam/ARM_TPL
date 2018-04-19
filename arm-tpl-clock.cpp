#include <arm-tpl.h>

extern "C" void ARMTPLClockInit()
{
}

int __ARM_TPL_clock_realtime(timespec* __ts)
{
  return 0;
}

int __ARM_TPL_clock_monotonic(timespec* __ts)
{
  return 0;
}

