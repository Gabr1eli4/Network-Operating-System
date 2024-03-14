#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_t thread;

void *program() {
  int index = fork();
  if (index == 0) {
    printf("Thread\n");
  } else {
  }
}

int main() {
  pthread_create(&thread, NULL, program, NULL);
  pthread_join(thread, NULL);

  return 0;
}