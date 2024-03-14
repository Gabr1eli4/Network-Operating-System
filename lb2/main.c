#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct Student {
  int id;
  int gender;
};

struct shmstruct {
  int count;

  int maleConscience, femaleConscience;

  sem_t cabins;
  int keys;

  pthread_mutex_t conscienceMutex;
  pthread_mutex_t maleMutex, femaleMutex;

  int NUMBER_OF_CABINS, MAX_CONSCIENCE, MAX_WAITING;

  int currentGender;
};

struct shmstruct *ptr;

struct shmstruct *proccessSharedMemoryPtr;

pthread_mutexattr_t attr;

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  srand(time(NULL));
  int fd;

  shm_unlink("shm");

  fd = shm_open("shm", O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

  ftruncate(fd, sizeof(struct shmstruct));

  ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED,
             fd, 0);

  close(fd);

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

  ptr->NUMBER_OF_CABINS = 2;
  ptr->MAX_CONSCIENCE = 5;
  ptr->MAX_WAITING = 10;
  ptr->count = 0;
  ptr->maleConscience = 0;
  ptr->femaleConscience = 0;
  sem_init(&ptr->cabins, 1, 2);
  ptr->keys = 2;
  pthread_mutex_init(&ptr->conscienceMutex, &attr);
  pthread_mutex_init(&ptr->maleMutex, &attr);
  pthread_mutex_init(&ptr->femaleMutex, &attr);
  ptr->currentGender = 0;

  for (int i = 0; i < 20; i++) {
    struct Student student;
    student.id = i;
    student.gender = i % 2;

    if (fork() == 0) {
      setvbuf(stdout, NULL, _IONBF, 0);

      proccessSharedMemoryPtr = mmap(NULL, sizeof(struct shmstruct),
                                     PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

      char *gender = student.gender == 0 ? "Мужской" : "Женский";
      pthread_mutex_t *toLock;
      int *conscience;

      if (student.gender == 0) {
        toLock = &ptr->maleMutex;
        conscience = &ptr->maleConscience;
      } else {
        toLock = &ptr->femaleMutex;
        conscience = &ptr->femaleConscience;
      }

      while (1) {
        pthread_mutex_lock(toLock);

        if ((student.gender == ptr->currentGender ||
             ptr->keys == ptr->NUMBER_OF_CABINS)) {
          sem_wait(&ptr->cabins);

          if (ptr->currentGender != student.gender) {
            if (student.gender == 0) {
              ptr->femaleConscience = 0;
            } else {
              ptr->maleConscience = 0;
            }
          }

          ptr->currentGender = student.gender;

          if ((*conscience) >= ptr->MAX_CONSCIENCE) {
            printf("Десяти секундное ожидание - %s пол\n", gender);
            (*conscience) = 0;
            sem_post(&ptr->cabins);
            sleep(ptr->MAX_WAITING);
            pthread_mutex_unlock(toLock);
            continue;
          }

          pthread_mutex_unlock(toLock);

          ptr->keys--;
          pthread_mutex_lock(&ptr->conscienceMutex);
          (*conscience)++;
          pthread_mutex_unlock(&ptr->conscienceMutex);
          printf("Совесть - %d\n", (*conscience));

          int washingTime = rand() % 5 + 1;
          printf("Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
                 student.id, gender, washingTime);

          sleep(washingTime);

          ptr->count++;
          // ptr->maleCount++;

          printf("Студент %d {%s} вышел из ванной комнаты\n\n", student.id,
                 gender);
          // Освобождаем кабинку
          sem_post(&ptr->cabins);
          ptr->keys++;

          return 0;
        } else {
          pthread_mutex_unlock(toLock);
        }
      }
    } else {
    }
  }
  pid_t wpid;
  int status = 0;
  while ((wpid = wait(&status) > 0))
    ;

  return 0;
}
