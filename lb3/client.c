#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int clientSocket;
struct sockaddr_in for_addr;

struct Package {
  int gender;
  int count;
};

pthread_t thread;

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    printf("Error creating socket\n");
    return -1;
  }

  for_addr.sin_addr.s_addr = inet_addr("192.168.1.71");
  for_addr.sin_family = AF_INET;
  for_addr.sin_port = htons(5556);

  int connectionResult =
      connect(clientSocket, (struct sockaddr *)&for_addr, sizeof(for_addr));

  if (connectionResult == -1) {
    printf("Error connecting to server\n");
    return -1;
  }

  while (1) {
    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);

    int w, m, parsed;
    struct Package package;

    parsed = sscanf(buffer, "w %d m %d", &w, &m);
    if (parsed == 2) {
      printf("1");
      package.count = w;
      package.gender = 1;
      send(clientSocket, &package, sizeof(struct Package), 0);

      package.count = w;
      package.gender = 1;
      send(clientSocket, &package, sizeof(struct Package), 0);
      continue;
    }

    parsed = sscanf(buffer, "m %d w %d", &m, &w);
    if (parsed == 2) {
      printf("2");
      package.count = m;
      package.gender = 0;
      send(clientSocket, &package, sizeof(struct Package), 0);

      package.count = w;
      package.gender = 1;
      send(clientSocket, &package, sizeof(struct Package), 0);
      continue;
    }

    parsed = sscanf(buffer, "m %d", &m);
    if (parsed == 1) {
      printf("3");
      package.count = m;
      package.gender = 0;
      send(clientSocket, &package, sizeof(struct Package), 0);
      continue;
    }

    parsed = sscanf(buffer, "w %d", &w);
    if (parsed == 1) {
      printf("4");
      package.count = w;
      package.gender = 1;
      send(clientSocket, &package, sizeof(package), 0);
      continue;
    }
  }
  close(clientSocket);
  return 0;
}