#pragma once
#ifndef __cplusplus
void ARMTPLInit();
#else
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

enum MutexType
{
  NORMAL,
  RECURSIVE,
};

struct MutexStruct
{
  SemaphoreHandle_t mutex;
  MutexType type;
};

struct ThreadStruct
{
  TaskHandle_t task;
  void* (*func)(void*);
  void* arg;
  SemaphoreHandle_t joinSemaphore;
  SemaphoreHandle_t detachSemaphore;
};

class ConditionVariable
{
public:
  ConditionVariable();
  ~ConditionVariable();
  void wait(SemaphoreHandle_t lock, bool recursive);
  int timedWait(SemaphoreHandle_t lock, bool recursive, unsigned int timeoutMS);
  void signal();
  void broadcast();
private:
  SemaphoreHandle_t s;
  SemaphoreHandle_t h;
  SemaphoreHandle_t x;
};

#endif
