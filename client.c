#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./sha256.h"

#define PORT 6969
#define MAX_MESSAGE_LENGTH 1024

static const char *KEY = "Bruh this is key";

void hash_message_with_key(const char *mess, BYTE hash[]) {
  SHA256_CTX ctx = {0};
  sha256_init(&ctx);

  sha256_update(&ctx, (BYTE*)KEY, strlen(KEY));
  sha256_update(&ctx, (BYTE*)mess, strlen(mess));
  
  sha256_final(&ctx, hash);
}

char *get_message(const char *incoming_msg) {
  size_t size = 0;
  static char buffer[MAX_MESSAGE_LENGTH] = {0};

  for (size_t i = 0; i < strlen(incoming_msg); ++i) {
    size++;
    if (incoming_msg[i] == '*') {
      size--;
      break;
    }
  }
  
  memcpy(buffer, incoming_msg, size);
  return buffer;
}

char *get_hash(const char *incoming_msg) {
  size_t size = 0;
  static char buffer[MAX_MESSAGE_LENGTH] = {0};

  for (size_t i = 0; i < strlen(incoming_msg); ++i) {
    size++;
    if (incoming_msg[i] == '*') {
      break;
    }
  }
  
  memcpy(buffer, incoming_msg + size, strlen(incoming_msg) - size);
  return buffer;
}

bool authentication(const char *incoming_msg) {
  BYTE hash[256] = {0};
  char *msg = get_message(incoming_msg);
  char *sha_code = get_hash(incoming_msg);

  hash_message_with_key(msg, hash);

  if (strcmp(sha_code, (char*)hash) == 0) {
    return true;
  } else {
    return false;
  }
}

int main(void) {
  int sockfd;
  pid_t pid;
  char buffer[MAX_MESSAGE_LENGTH] = {0};

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));

  // Creating a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "ERROR: Could not create Socket: %s\n", strerror(errno));
    exit(1);
  } else {
    fprintf(stdout, "Socket sucessfully created\n");
  }

  // INADDR_LOOPBACK basicly is "127.0.0.1"
  server_addr.sin_family	= AF_INET;
  server_addr.sin_addr.s_addr	= htonl(INADDR_LOOPBACK);
  server_addr.sin_port		= htons(PORT);
  
  // Establishes connection with the server using server IP address
  if (connect(sockfd, (const struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
    fprintf(stderr, "ERROR: Could not connect to the server: %s\n", strerror(errno));
    exit(1);
  } else {
    fprintf(stdout, "Succesfully connected to the server\n");
  }

  pid = fork();
  if (pid == 0) {
    // Send Messages
    while (true) {
      BYTE hash[256] = {0};

      bzero(buffer, MAX_MESSAGE_LENGTH);
      printf("Send some messages to the server (or type 'exit' to close the connection)\n");
      printf("    -> ");

      fgets(buffer, MAX_MESSAGE_LENGTH, stdin);

      // Adding the hash value(key + message)
      hash_message_with_key(buffer, hash);
      strcat(buffer, "*");
      strcat(buffer, (char*)hash);
      char *message = get_message(buffer);

      if (strncmp(message, "exit", 4) == 0) {
	printf("Client exiting...\n");
	break; 
      }

      send(sockfd, buffer, strlen(buffer) + 1, 0);

      printf("Message sent!\n");
      printf("----------------------------------------\n");
    }  
  } else {
    // Recieve Messages
    while (true) {
      bzero(buffer, MAX_MESSAGE_LENGTH);

      recv(sockfd, buffer, sizeof(buffer), 0);
      char *mess = get_message(buffer);

      if (strncmp(mess, "exit", 4) == 0) {
	printf("Client exiting...\n");
	break; 
      }

      if (authentication(buffer)) {
	printf("Message recived from server: %s\n", mess);
      } else {
	printf("Message recived from server: %s\n", mess);
	break;
      }
    }
  }

  if (close(sockfd) < 0) {
    fprintf(stderr, "ERROR: Could not close the socket: %s\n", strerror(errno));
    exit(1);
  } else {
    fprintf(stdout, "Sucessfully close the socket\n");
  }
  return 0;
}
