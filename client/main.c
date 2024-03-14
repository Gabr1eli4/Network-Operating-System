#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

struct sockaddr_in addr;

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0) {
    printf("socket error");
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(9999);
  addr.sin_addr.s_addr = inet_addr("192.168.43.37");

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    printf("error");
    return -1;
  }
  // char Data[30] = "0abcdefghijklmnopqrstuvwxyz";
  char name[40] = "Александров\0";

  // while (1) {
  send(sock, &name, sizeof(name), 0);
  // send(sock, &Data, sizeof(Data), 0);
  // }

  return 0;
}