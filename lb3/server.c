#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int count = 0;
int maleConscience = 0, femaleConscience = 0;

int const NUMBER_OF_CABINS = 2;
int const MAX_CONSCIENCE = 5;
int const MAX_WAITING = 10;

sem_t cabins;
int keys = NUMBER_OF_CABINS;

pthread_mutex_t conscienceMutex;
pthread_mutex_t maleMutex = PTHREAD_MUTEX_INITIALIZER,
                femaleMutex = PTHREAD_MUTEX_INITIALIZER;

int currentGender = 0;

int clients[100];
int clientsCount = 0;

int serverSocket;
struct sockaddr_in for_addr;

struct Student {
  pthread_t thread;
  int id;
  int gender;
  int socket;
};

struct Package {
  int gender;
  int count;
};

void *program(void *arg) {
  struct Student *student = (struct Student *)arg;
  char *gender = student->gender == 0 ? "Мужской" : "Женский";

  int *conscience;
  pthread_mutex_t *toLock;

  if (student->gender == 0) {
    toLock = &maleMutex;
    conscience = &maleConscience;
  } else {
    toLock = &femaleMutex;
    conscience = &femaleConscience;
  }

  while (1) {
    pthread_mutex_lock(toLock);
    if (student->gender == currentGender || keys == NUMBER_OF_CABINS) {
      sem_wait(&cabins);
      currentGender = student->gender;

      if ((*conscience) >= MAX_CONSCIENCE) {
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

      int washingTime = rand() % 5 + 1;

      printf("Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
             student->id, gender, washingTime);

      sleep(washingTime);

      count++;
      keys++;

      printf("Студент %d {%s} вышел из ванной комнаты\n\n", student->id,
             gender);

      sem_post(&cabins);

      break;
    } else {
      pthread_mutex_unlock(toLock);
    }
  }
  send(student->socket, "Студент закончил мыться\n", 46, 0);
}

void procedure() {
  while (1) {
    struct Package *package = malloc(sizeof(struct Package));

    for (int i = 0; i < clientsCount; i++) {
      int receiveResult = recv(clients[i], package, sizeof(struct Package), 0);

      if (receiveResult == -1) {
        printf("ERROR OCCURED");
        return;
      }

      for (int i = 0; i < package->count; i++) {
        struct Student *student = malloc(sizeof(struct Student));
        student->id = i;
        student->gender = package->gender;
        student->socket = clients[i];

        pthread_create(&student->thread, NULL, program, student);
      }
    }
  }
}

pthread_t newClients;
pthread_mutex_t newClientsMutex = PTHREAD_MUTEX_INITIALIZER;

void *commingClients() {
  while (1) {
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
      printf("Error accepting new client\n");
      return 0;
    }

    printf("Подключился клиент с Id = %d\n", clientSocket);
    pthread_mutex_lock(&newClientsMutex);
    clients[clientsCount] = clientSocket;
    clientsCount++;
    pthread_mutex_unlock(&newClientsMutex);
  }
}

int main() {
  setbuf(stdout, NULL);

  srand(time(NULL));

  sem_init(&cabins, 0, NUMBER_OF_CABINS);

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  for_addr.sin_addr.s_addr = inet_addr("192.168.1.71");
  for_addr.sin_family = AF_INET;
  for_addr.sin_port = htons(5556);

  bind(serverSocket, (struct sockaddr *)&for_addr, sizeof(for_addr));
  listen(serverSocket, 5);
  pthread_create(&newClients, NULL, commingClients, NULL);

  procedure();

  close(serverSocket);

  pthread_exit(0);
}
