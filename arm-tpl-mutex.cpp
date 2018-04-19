#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <arm-tpl.h>

static SemaphoreHandle_t guard_mut;

extern "C" void ARMTPLMutexInit()
{
  guard_mut = xSemaphoreCreateMutex();
}

static int checkCreate(volatile __ARM_TPL_mutex_t** __vm)
{
  if ((*__vm)->data == 0) {
		xSemaphoreTake(guard_mut, portMAX_DELAY);
		if ((*__vm)->data == 0)
    {
      SemaphoreHandle_t newMutex = xSemaphoreCreateMutex();
      if (newMutex == nullptr)
        return -1;
			(*__vm)->data = (uintptr_t)(newMutex);
    }
	}
  return 0;
}

extern "C" int __ARM_TPL_recursive_mutex_init(__ARM_TPL_mutex_t* __m)
{
  return 0;
}

extern "C" int __ARM_TPL_mutex_lock(__ARM_TPL_mutex_t* __m) {
	volatile __ARM_TPL_mutex_t *__vm = __m;
	if (checkCreate(&__vm))
    return -1;
	xSemaphoreTake((SemaphoreHandle_t)(__vm)->data, portMAX_DELAY);
  return 0;
}

extern "C" int __ARM_TPL_mutex_trylock(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t *__vm = __m;
	if (checkCreate(&__vm))
    return -1;
	if (xSemaphoreTake((SemaphoreHandle_t)(__vm)->data, 0) == pdTRUE)
    return 0;
  else
    return 1;
}


extern "C" int __ARM_TPL_mutex_unlock(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t* __vm = __m;
  if (xSemaphoreGive((__vm)->data) == pdTRUE)
    return 0;
  return -1;
}

extern "C" int __ARM_TPL_mutex_destroy(__ARM_TPL_mutex_t* __m)
{
  volatile __ARM_TPL_mutex_t *__vm = __m;
  vSemaphoreDelete((__vm)->data);
  return 0;
}
