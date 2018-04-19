#include <arm-tpl.h>

extern "C" void ARMTPLCondVarInit()
{
}

int __ARM_TPL_condvar_signal(__ARM_TPL_condvar_t* __cv)
{
  return 0;
}

int __ARM_TPL_condvar_broadcast(__ARM_TPL_condvar_t* __cv)
{
  return 0;
}

int __ARM_TPL_condvar_wait(__ARM_TPL_condvar_t* __cv,
                           __ARM_TPL_mutex_t* __m)
{
  return 0;
}

int __ARM_TPL_condvar_timedwait(__ARM_TPL_condvar_t* __cv,
                                __ARM_TPL_mutex_t* __m,
                                timespec* __ts)
{
  return 0;
}

int __ARM_TPL_condvar_destroy(__ARM_TPL_condvar_t* __cv)
{
  return 0;
}