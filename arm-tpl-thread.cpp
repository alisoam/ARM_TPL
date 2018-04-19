#include <FreeRTOS.h>
#include <task.h>
#include <arm-tpl.h>

extern "C" void ARMTPLThreadInit()
{
}

extern "C" int __ARM_TPL_thread_create(__ARM_TPL_thread_t* __t,
                            void* (*__func)(void*),
                            void* __arg)
{
  if (xTaskCreate((TaskFunction_t)__func, "",
                            200, __arg, 1, (void**)&(__t->data)) == pdTRUE)
    return 0;
  else
    return -1;
}

int __ARM_TPL_thread_id_compare(__ARM_TPL_thread_id __tid1,
                                 __ARM_TPL_thread_id __tid2)
{
  if (__tid1 > __tid2)
    return 1;
  else if (__tid1 < __tid2)
    return -1;
  else
    return 0;
}

__ARM_TPL_thread_id __ARM_TPL_thread_get_current_id()
{
  return (__ARM_TPL_thread_id)xTaskGetCurrentTaskHandle();
}

__ARM_TPL_thread_id __ARM_TPL_thread_get_id(
                    const __ARM_TPL_thread_t* __t)
{
    return (__ARM_TPL_thread_id)__t->data;
}

int __ARM_TPL_thread_join(__ARM_TPL_thread_t* __t)
{
  return 1;
}

int __ARM_TPL_thread_detach(__ARM_TPL_thread_t* __t)
{
  return 0;
}

void __ARM_TPL_thread_yield()
{
  portYIELD();
}

int __ARM_TPL_thread_nanosleep(const timespec* __req,
                               timespec* __rem)
{
  vTaskDelay(__req->tv_sec * configTICK_RATE_HZ +
             __req->tv_nsec /1e6 * portTICK_RATE_MS);
  return 0;
}

unsigned __ARM_TPL_thread_hw_concurrency() {
  return 1;
}

int __ARM_TPL_tls_create(__ARM_TPL_tls_key* __key,
                         void (*__at_exit)(void*))
{
  return 0;
}

void* __ARM_TPL_tls_get(__ARM_TPL_tls_key __key)
{
  return 0;
}

int __ARM_TPL_tls_set(__ARM_TPL_tls_key __key, void* __p)
{
  return 0;
}
