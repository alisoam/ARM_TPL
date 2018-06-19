#include <arm-tpl.h>
#include "tpl.h"
#include <new>

static SemaphoreHandle_t conditionVariableCreationGuardMutex;

ConditionVariable::ConditionVariable()
{
  s = xSemaphoreCreateCounting(999999, 0);
  if (s == nullptr)
    throw std::bad_alloc();
  h = xSemaphoreCreateCounting(999999, 0);
  if (h == nullptr)
  {
    vSemaphoreDelete(s);
    throw std::bad_alloc();
  }
  x = xSemaphoreCreateMutex();
  if (x == nullptr)
  {
    vSemaphoreDelete(s);
    vSemaphoreDelete(h);
    throw std::bad_alloc();
  }
}

ConditionVariable::~ConditionVariable()
{
  vSemaphoreDelete(x);
  vSemaphoreDelete(h);
  vSemaphoreDelete(s);
}

void ConditionVariable::wait(SemaphoreHandle_t lock, bool recursive)
{
  while (xSemaphoreTake(x, portMAX_DELAY) != pdTRUE);
  xSemaphoreGive(s);
  xSemaphoreGive(x);
  if (recursive)
    xSemaphoreGiveRecursive(lock);
  else
    xSemaphoreGive(lock);
  while (xSemaphoreTake(h, portMAX_DELAY) != pdTRUE);
  if (recursive)
    while (xSemaphoreTakeRecursive(lock, portMAX_DELAY) != pdTRUE);
  else
    while (xSemaphoreTake(lock, portMAX_DELAY) != pdTRUE);
}

int ConditionVariable::timedWait(SemaphoreHandle_t lock, bool recursive, unsigned int timeoutMS)
{
  int toBeReturned = 0;
  while (xSemaphoreTake(x, portMAX_DELAY) != pdTRUE)
  xSemaphoreGive(s);
  xSemaphoreGive(x);
  if (recursive)
    xSemaphoreGiveRecursive(lock);
  else
    xSemaphoreGive(lock);
  if (xSemaphoreTake(h, timeoutMS * portTICK_RATE_MS) != pdTRUE)
  {
    while (xSemaphoreTake(x, portMAX_DELAY) != pdTRUE);
    if (xSemaphoreTake(s, 0) != pdTRUE)
    {
      if (xSemaphoreTake(h, 0) != pdTRUE)
        toBeReturned = -1;
    }
    else
      toBeReturned = 1;
    xSemaphoreGive(x);
  }
  if (recursive)
    while (xSemaphoreTakeRecursive(lock, portMAX_DELAY) != pdTRUE);
  else
    while (xSemaphoreTake(lock, portMAX_DELAY) != pdTRUE);
  return toBeReturned;
}

void ConditionVariable::signal()
{
  while (xSemaphoreTake(x, portMAX_DELAY) != pdTRUE);
  if (xSemaphoreTake(s, 0) == pdTRUE)
    xSemaphoreGive(h);
  xSemaphoreGive(x);
}

void ConditionVariable::broadcast()
{
  while (xSemaphoreTake(x, portMAX_DELAY) != pdTRUE);
  auto count = uxSemaphoreGetCount(s);
  for (auto i = 0; i < count; i++)
  {
    while (xSemaphoreTake(s, portMAX_DELAY) != pdTRUE);
    xSemaphoreGive(h);
  }
  xSemaphoreGive(x);
}

void ARMTPLCondVarInit()
{
  conditionVariableCreationGuardMutex = xSemaphoreCreateMutex();
}

static int checkCreate(volatile __ARM_TPL_condvar_t* __vcv)
{
  if (__vcv->data == 0)
  {
    while (xSemaphoreTake(conditionVariableCreationGuardMutex, portMAX_DELAY) != pdTRUE);
    if (__vcv->data == 0)
      try
      {
        __vcv->data = (uintptr_t)new ConditionVariable();
      }
      catch (...)
      {
        return -1;
      }
    xSemaphoreGive(conditionVariableCreationGuardMutex);
  }
  return 0;
}

extern "C" int __ARM_TPL_condvar_wait(__ARM_TPL_condvar_t* __cv,
                           __ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_condvar_t* __vcv = __cv;
  if (checkCreate(__vcv))
    return -1;
  struct MutexStruct* mS = (struct MutexStruct*)(__m->data);
  ((ConditionVariable*) __vcv->data)->wait(mS->mutex, mS->type == RECURSIVE);
  return 0;
}

extern "C" int __ARM_TPL_condvar_timedwait(__ARM_TPL_condvar_t* __cv,
                                __ARM_TPL_mutex_t* __m,
                                timespec* __ts)
{
  volatile __ARM_TPL_condvar_t* __vcv = __cv;
  if (checkCreate(__vcv))
    return -1;
  timespec now;
  if (__ARM_TPL_clock_realtime(&now) != 0)
    return -1;
  struct MutexStruct* mS = (struct MutexStruct*)(__m->data);
  unsigned int timeoutMS = (__ts->tv_sec - now.tv_sec) * 1000 + (__ts->tv_nsec - now.tv_nsec) / 1000000;
  return ((ConditionVariable*) __vcv->data)->timedWait(mS->mutex, mS->type == RECURSIVE, timeoutMS);
}

extern "C" int __ARM_TPL_condvar_signal(__ARM_TPL_condvar_t* __cv)
{
  volatile __ARM_TPL_condvar_t *__vcv = __cv;
  if (__vcv->data != 0)
    ((ConditionVariable*) __vcv->data)->signal();
  return 0;
}

extern "C" int __ARM_TPL_condvar_broadcast(__ARM_TPL_condvar_t* __cv)
{
  volatile __ARM_TPL_condvar_t *__vcv = __cv;
  if (__vcv->data != 0)
    ((ConditionVariable*) __vcv->data)->broadcast();
  return 0;
}

extern "C" int __ARM_TPL_condvar_destroy(__ARM_TPL_condvar_t* __cv)
{
  volatile __ARM_TPL_condvar_t *__vcv = __cv;
  if (__vcv->data != 0)
  {
    delete (ConditionVariable*) __vcv->data;
    __vcv->data = 0;
  }
  return 0;
}
