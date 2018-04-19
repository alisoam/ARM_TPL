extern "C" void ARMTPLMutexInit();
extern "C" void ARMTPLThreadInit();
extern "C" void ARMTPLCondVarInit();
extern "C" void ARMTPLClockInit();
extern "C" void ARMTPLMiscellaneousInit();

extern "C" void ARMTPLInit()
{
  ARMTPLMutexInit();
  ARMTPLCondVarInit();
  ARMTPLThreadInit();
  ARMTPLClockInit();
  ARMTPLMiscellaneousInit();
}
