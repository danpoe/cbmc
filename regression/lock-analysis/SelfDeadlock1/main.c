// self deadlock

#include <pthread.h>

pthread_mutex_t m1;

int x;

void *thr1(void *arg)
{
  pthread_mutex_lock(&m1);
  ++x;
  pthread_mutex_lock(&m1);
}

int main()
{
  pthread_t tid1;

  pthread_create(&tid1, NULL, thr1, NULL);
  pthread_join(tid1, NULL);

  return 0;
}
