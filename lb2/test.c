#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shmstruct {
  int count;
};

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  int fd;
  struct shmstruct *ptr;

  shm_unlink("shm");

  fd = shm_open("shm", O_CREAT | O_RDWR | O_EXCL, FILE_MODE);

  ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED,
             fd, 0);

  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);

  ptr->count = 0;

  int index = fork();

  if (index == 0) {
    for (int i = 0; i < 10; i++) {
      ptr->count++;
      printf("Child: %d\n", ptr->count);
    }
  } else {
    for (int i = 0; i < 10; i++) {
      wait(NULL);
      ptr->count++;
      printf("Parent: %d\n", ptr->count);
    }
  }

  return 0;
}