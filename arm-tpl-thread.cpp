#include <arm-tpl.h>
#include "tpl.h"
#include <cstdio>
static unsigned int localStorageKeyCounter = 0;

static void cTask(void* arg)
{
  for (unsigned int i = 0; i < configNUM_THREAD_LOCAL_STORAGE_POINTERS; i++)
    vTaskSetThreadLocalStoragePointer(nullptr, i, nullptr);
  ThreadStruct* threadStructPtr = (ThreadStruct*)arg;
  threadStructPtr->func(threadStructPtr->arg);
  xSemaphoreGive(threadStructPtr->joinSemaphore);
  while (xSemaphoreTake(threadStructPtr->detachSemaphore, portMAX_DELAY) != pdTRUE);
  for (volatile unsigned int i = 0; i < localStorageKeyCounter; i++)
  {
    unsigned int k = 2 * i;
    void* val = pvTaskGetThreadLocalStoragePointer(nullptr, k);
    if (val != nullptr)
    {
      void (*__at_exit)(void*) = (void (*)(void*))(pvTaskGetThreadLocalStoragePointer(nullptr, k + 1));
      if (__at_exit != nullptr)
        __at_exit(val);
    }
  }
  vSemaphoreDelete(threadStructPtr->detachSemaphore);
  vSemaphoreDelete(threadStructPtr->joinSemaphore);
  vPortFree((void*)threadStructPtr);
  vTaskDelete(nullptr);
  for(;;);
}

extern "C" int __ARM_TPL_thread_create(__ARM_TPL_thread_t* __t,
                            void* (*__func)(void*),
                            void* __arg)
{
  ThreadStruct* threadStructPtr = (ThreadStruct*)pvPortMalloc(sizeof(ThreadStruct));
  if (threadStructPtr == nullptr)
    goto exit1;
  threadStructPtr->arg = __arg;
  threadStructPtr->func = __func;
  threadStructPtr->joinSemaphore = xSemaphoreCreateBinary();
  if (threadStructPtr->joinSemaphore == nullptr)
    goto exit2;
  threadStructPtr->detachSemaphore = xSemaphoreCreateBinary();
  if (threadStructPtr->detachSemaphore == nullptr)
    goto exit3;
  if (xTaskCreate((TaskFunction_t)cTask, "C++", configMINIMAL_STACK_SIZE, (void*)threadStructPtr, tskIDLE_PRIORITY, &(threadStructPtr->task)) == pdTRUE)
  {
    __t->data = (std::uintptr_t)threadStructPtr;
    return 0;
  }
  exit:
    vSemaphoreDelete(threadStructPtr->detachSemaphore);
  exit3:
    vSemaphoreDelete(threadStructPtr->joinSemaphore);
  exit2:
    vPortFree(threadStructPtr);
  exit1:
    return -1;
}

extern "C" int __ARM_TPL_thread_id_compare(__ARM_TPL_thread_id __tid1,
                                 __ARM_TPL_thread_id __tid2)
{
  if (__tid1 > __tid2)
    return 1;
  else if (__tid1 < __tid2)
    return -1;
  else
    return 0;
}

extern "C" __ARM_TPL_thread_id __ARM_TPL_thread_get_current_id()
{
  return (__ARM_TPL_thread_id)xTaskGetCurrentTaskHandle();
}

extern "C" __ARM_TPL_thread_id __ARM_TPL_thread_get_id(
                    const __ARM_TPL_thread_t* __t)
{
    return (__ARM_TPL_thread_id)(((ThreadStruct*)(__t->data))->task);
}

extern "C" int __ARM_TPL_thread_join(__ARM_TPL_thread_t* __t)
{
  ThreadStruct* threadStructPtr = (ThreadStruct*)(__t->data);
  while (xSemaphoreTake(threadStructPtr->joinSemaphore, portMAX_DELAY) != pdTRUE);
  xSemaphoreGive(threadStructPtr->detachSemaphore);
  return 0;
}

extern "C" int __ARM_TPL_thread_detach(__ARM_TPL_thread_t* __t)
{
  ThreadStruct* threadStructPtr = (ThreadStruct*)(__t->data);
  xSemaphoreGive(threadStructPtr->detachSemaphore);
  return 0;
}

extern "C" void __ARM_TPL_thread_yield()
{
  portYIELD();
}

extern "C" int __ARM_TPL_thread_nanosleep(const timespec* __req,
                               timespec* __rem)
{
  vTaskDelay(__req->tv_sec * configTICK_RATE_HZ +
             __req->tv_nsec /1e6 * portTICK_RATE_MS);
  // FIXME
  if (__rem != nullptr)
  {
    __rem->tv_sec = 0;
    __rem->tv_nsec = 0;
  }
  return 0;
}

extern "C" unsigned __ARM_TPL_thread_hw_concurrency() {
  return 1;
}

extern "C" int __ARM_TPL_tls_create(__ARM_TPL_tls_key* __key,
                         void (*__at_exit)(void*))
{
  if (localStorageKeyCounter > configNUM_THREAD_LOCAL_STORAGE_POINTERS / 2)
    return -1;
  *__key = localStorageKeyCounter;
  unsigned int k = 2 * *__key ;
  vTaskSetThreadLocalStoragePointer(NULL, k, nullptr);
  vTaskSetThreadLocalStoragePointer(NULL, k + 1, (void*) __at_exit);
  localStorageKeyCounter++;
  return 0;
}

extern "C" void* __ARM_TPL_tls_get(__ARM_TPL_tls_key __key)
{
  if (__key >= localStorageKeyCounter)
    return nullptr;
  unsigned int k = 2 * __key;
  return pvTaskGetThreadLocalStoragePointer(nullptr, k);
}

extern "C" int __ARM_TPL_tls_set(__ARM_TPL_tls_key __key, void* __p)
{
  if (__key >= localStorageKeyCounter)
    return -1;
  unsigned int k = 2 * __key;
  vTaskSetThreadLocalStoragePointer(nullptr, k, __p);
  return 0;
}
