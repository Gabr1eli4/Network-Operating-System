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

void sendResponse(char *message, size_t size) {
  for (int i = 0; i < clientsCount; i++) {
    send(clients[i], message, size, 0);
  }
}

void *program(void *arg) {
  struct Student *student = (struct Student *)arg;
  char *gender = student->gender == 0 ? "Мужской" : "Женский";
  char buffer[256] = {0};

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

      sprintf(buffer, "Студент %d {%s} вошёл в ванную комнату на %d секунд\n\n",
              student->id, gender, washingTime);
      sendResponse(buffer, sizeof(buffer));

      sleep(washingTime);

      count++;
      keys++;

      printf("Студент %d {%s} вышел из ванной комнаты\n\n", student->id,
             gender);
      sprintf(buffer, "Студент %d {%s} вышел из ванной комнаты\n\n",
              student->id, gender);
      sendResponse(buffer, sizeof(buffer));

      sem_post(&cabins);

      break;
    } else {
      pthread_mutex_unlock(toLock);
    }
  }
  printf("Студент {%d} закончил мыться\n", student->id);
  sprintf(buffer, "Студент {%d} закончил мыться\n", student->id);
  sendResponse(buffer, sizeof(buffer));
}

pthread_t newClients, receiveThread;
pthread_mutex_t newClientsMutex = PTHREAD_MUTEX_INITIALIZER;

void *receiveMessage(void *arg) {
  int clientSocket = *(int *)arg;
  printf("%d", clientSocket);
  struct Package package;

  while (1) {
    int recvResult = recv(clientSocket, &package, sizeof(package), 0);

    if (recvResult == -1) {
      printf("Error receiving message\n");
      continue;
    }

    // printf("Received message: %d %d\n", package.count, package.gender);

    for (int i = 0; i < package.count; i++) {
      struct Student *student = malloc(sizeof(struct Student));
      student->id = i;
      student->gender = package.gender;
      pthread_create(&student->thread, NULL, program, student);
    }
  }
}

void *commingClients() {
  while (1) {
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
      printf("Error accepting new client\n");
      continue;
    }

    printf("Подключился клиент с Id = %d\n", clientSocket);
    pthread_mutex_lock(&newClientsMutex);
    clients[clientsCount] = clientSocket;
    clientsCount++;
    pthread_mutex_unlock(&newClientsMutex);
    pthread_create(&receiveThread, NULL, receiveMessage, &clientSocket);
  }
}

int main() {
  setbuf(stdout, NULL);

  srand(time(NULL));

  sem_init(&cabins, 0, NUMBER_OF_CABINS);

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  for_addr.sin_addr.s_addr = inet_addr("192.168.26.239");
  for_addr.sin_family = AF_INET;
  for_addr.sin_port = htons(5558);

  bind(serverSocket, (struct sockaddr *)&for_addr, sizeof(for_addr));
  listen(serverSocket, 5);
  pthread_create(&newClients, NULL, commingClients, NULL);

  char keyboard[256];
  fgets(keyboard, sizeof(keyboard), stdin);
  close(serverSocket);

  pthread_exit(0);
}
