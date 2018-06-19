#include <arm-tpl.h>
#include "tpl.h"

static SemaphoreHandle_t mutexCreationGuardMutex;

void ARMTPLMutexInit()
{
  mutexCreationGuardMutex = xSemaphoreCreateMutex();
}

static int checkCreate(volatile __ARM_TPL_mutex_t* __vm, bool recursive = false)
{
  if (__vm->data == 0)
  {
		xSemaphoreTake(mutexCreationGuardMutex, portMAX_DELAY);
    int toBeReturned = -1;
		if (__vm->data == 0)
    {
      MutexStruct* mutexStructPtr = (MutexStruct*)pvPortMalloc(sizeof(MutexStruct));
      if (mutexStructPtr == nullptr)
        goto exit;
      if (recursive)
        mutexStructPtr->mutex = xSemaphoreCreateRecursiveMutex();
      else
        mutexStructPtr->mutex = xSemaphoreCreateMutex();
      if (mutexStructPtr->mutex == nullptr)
      {
        vPortFree(mutexStructPtr);
        goto exit;
      }
      mutexStructPtr->type = recursive? RECURSIVE: NORMAL;
      __vm->data = (uintptr_t)(mutexStructPtr);
    }
    toBeReturned = 0;
    exit:
      xSemaphoreGive(mutexCreationGuardMutex);
      return toBeReturned;
	}
  return 0;
}

static int mutexLock(MutexStruct* mutexStructPtr, TickType_t timeOut)
{
  if (mutexStructPtr->type == RECURSIVE)
  {
    if (xSemaphoreTakeRecursive(mutexStructPtr->mutex, timeOut) == pdTRUE)
      return 0;
  }
  else
  {
    if (xSemaphoreTake(mutexStructPtr->mutex, timeOut) == pdTRUE)
      return 0;
  }
  return -1;
}

static int mutexUnlock(MutexStruct* mutexStructPtr)
{
  if (mutexStructPtr->type == RECURSIVE)
    xSemaphoreGiveRecursive(mutexStructPtr->mutex);
  else
    xSemaphoreGive(mutexStructPtr->mutex);
  return 0;
}

extern "C" int __ARM_TPL_recursive_mutex_init(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t *__vm = __m;
  return checkCreate(__vm, true);
}

extern "C" int __ARM_TPL_mutex_lock(__ARM_TPL_mutex_t* __m) {
	volatile __ARM_TPL_mutex_t *__vm = __m;
	if (checkCreate(__vm))
    return -1;
  while (mutexLock((MutexStruct*)(__vm->data), portMAX_DELAY) != 0);
  return 0;
}

extern "C" int __ARM_TPL_mutex_trylock(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t *__vm = __m;
	if (checkCreate(__vm))
    return -1;
  return mutexLock((MutexStruct*)(__vm->data), 0);
}

extern "C" int __ARM_TPL_mutex_unlock(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t* __vm = __m;
  return mutexUnlock((MutexStruct*)(__vm->data));
}

extern "C" int __ARM_TPL_mutex_destroy(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t *__vm = __m;
  if (__vm->data != 0)
  {
    vSemaphoreDelete(((MutexStruct*)(__vm->data))->mutex);
    vPortFree((void*)(__vm->data));
    __vm->data = 0;
  }
  return 0;
}
