#include "tpl.h"

extern "C" void ARMTPLInit()
{
  ARMTPLMutexInit();
  ARMTPLCondVarInit();
  ARMTPLThreadInit();
  ARMTPLClockInit();
  ARMTPLMiscellaneousInit();
}
