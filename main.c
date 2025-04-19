#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h> // sockaddr_un
#include <unistd.h> // unlink

#define SOCKET_PATH "/tmp/test-socket"

static void server(void) {
  printf("Running server\n");
  // AF_LOCAL for processes on the same host
  int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  // remove the socket file if it already exists
  unlink(addr.sun_path);

  if ((bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)) {
    perror("Failed to bind socket to address");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 5) < 0) {
    perror("failed to listen to server");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server is listening for connections..\n");

  // Accept client connection
  struct sockaddr_un client_addr;
  socklen_t client_len = sizeof(client_addr);

  int client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    perror("Failed to connect to client");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("CLient connected!\n");

  char *message = "Hello, from the server\n";
  size_t size = strlen(message);

  if ((send(client_fd, message, size, 0) < 0)) {
    perror("Failed to send buffer\n");
    close(server_fd);
    close(client_fd);
    exit(EXIT_FAILURE);
  }

  printf("Sent message to client\n");

  close(server_fd);
  close(client_fd);
  unlink(SOCKET_PATH);
  exit(EXIT_SUCCESS);
}

static void client(void) {
  printf("Running client\n");
  int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("Failed to connect to socket\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Failed to connect to server");
    close(fd);
    exit(EXIT_FAILURE);
  }

  printf("Connected to server\n");

  char message[256];
  memset(&message, 0, sizeof(message));

  size_t received = recv(fd, &message, sizeof(message) - 1, 0);
  if (!received) {
    perror("Failed to receive data\n");
    close(fd);
    exit(EXIT_FAILURE);
  }

  message[received] = '\0';

  printf("RECEIVED: %s\n", message);

  close(fd);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    perror("Specify client or server\n");
    exit(EXIT_FAILURE);
  }

  if (!(strcmp(argv[1], "server"))) {
    server();
    exit(EXIT_SUCCESS);
  } else if (!(strcmp(argv[1], "client"))) {
    client();
    exit(EXIT_SUCCESS);
  } else {
    fprintf(stderr, "Unknown option: %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  return 0;
}
