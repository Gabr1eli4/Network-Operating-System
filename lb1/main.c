#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int const NUMBER_OF_CABINS = 5;

struct Student {
  pthread_t thread;  // поток студента
  int id;            // id студента
  int gender;  // пол студента 0 - Мужской | 1 - Женский
};

// Количество студентов принявших ванну
int count = 0;
int maleCount = 0;
int femaleCount = 0;

// Показатель совести студентов (Количество студентов одного пола принявших
// ванну)
int maleConscience = 0, femaleConscience = 0;
int const MAX_CONSCIENCE = NUMBER_OF_CABINS * 2;
int const MAX_WAITING = 10;

// Ванные кабинки
sem_t cabins;
int keys = NUMBER_OF_CABINS;

// Мютекс совести
pthread_mutex_t conscienceMutex;
pthread_mutex_t maleMutex = PTHREAD_MUTEX_INITIALIZER,
                femaleMutex = PTHREAD_MUTEX_INITIALIZER;

// Строка обозначающая текущий пол студентов входящих в ванную комнату
int currentGender = 0;

void *program(void *arg) {
  struct Student *student = (struct Student *)arg;
  char *gender = student->gender == 0 ? "Мужской" : "Женский";
  pthread_mutex_t *toLock;
  int *conscience;

  int *counter;

  if (student->gender == 0) {
    toLock = &maleMutex;
    conscience = &maleConscience;
    counter = &maleCount;
  } else {
    toLock = &femaleMutex;
    conscience = &femaleConscience;
    counter = &femaleCount;
  }

  // printf("Студент %d {%s} пришёл в ванную комнату\n\n", student->id, gender);
  while (1) {
    pthread_mutex_lock(toLock);

    if ((student->gender == currentGender || keys == NUMBER_OF_CABINS)) {
      sem_wait(&cabins);

      if (currentGender != student->gender) {
        if (student->gender == 0) {
          femaleConscience = 0;
        } else {
          maleConscience = 0;
        }
      }

      currentGender = student->gender;

      if ((*conscience) >= MAX_CONSCIENCE) {
        printf("Десяти секундное ожидание - %s пол\n", gender);
        (*conscience) = 0;
        sem_post(&cabins);
        sleep(MAX_WAITING);
        pthread_mutex_unlock(toLock);
        continue;
      }

      pthread_mutex_unlock(toLock);

      keys--;
      pthread_mutex_lock(&conscienceMutex);
      (*conscience)++;
      pthread_mutex_unlock(&conscienceMutex);
      printf("Совесть - %d\n", (*conscience));

      int washingTime = rand() % 5 + 1;
      printf("Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
             student->id, gender, washingTime);

      sleep(washingTime);

      count++;
      (*counter)++;

      printf("Студент %d {%s} вышел из ванной комнаты\n\n", student->id,
             gender);
      // Освобождаем кабинку
      sem_post(&cabins);
      keys++;
      printf("keys = %d\n", keys);

      break;
    } else {
      pthread_mutex_unlock(toLock);
    }
  }
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  sem_init(&cabins, 0, NUMBER_OF_CABINS);

  clock_t start = time(NULL);

  clock_t r = time(NULL);
  srand(r);
  for (int i = 0; i < 20; i++) {
    clock_t end = time(NULL);
    if (end - start > 1) {
      break;
    }
    int gender = rand() % 2;  // 0 - Мужской | 1 - Женский
    // int gender = i % 2;

    struct Student *studentPtr = malloc(sizeof(struct Student));
    (*studentPtr).id = i;
    (*studentPtr).gender = gender;

    pthread_create(&studentPtr->thread, NULL, program, studentPtr);
  }

  // sem_destroy(&cabins);

  sleep(10);

  printf("Количество кабинок %d, Количество обслуженных студентов - %d\n",
         NUMBER_OF_CABINS, count);

  printf("Количество СТУДЕНТОВ - %d\nКоличество СТУДЕНТОК - %d", maleCount,
         femaleCount);

  pthread_exit(0);
}
