#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1025

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct in_addr in_addr;

int main(int argc, char* argv[]) {
	char buffer[BUFFER_SIZE];

	if (argc != 3) {
		perror("The udp client must have exactly 2 parameter as input: ip, port\n");
		exit(EXIT_FAILURE);
	}
	char *ip = argv[1];

	char *end;
    long port = strtol(argv[2], &end, 10);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in to = {0};

//	to.sin_addr = *(struct in_addr *) ip;
	to.sin_port = htons(port);
	to.sin_family = AF_INET;
	inet_aton(ip, &to.sin_addr);

	puts("Enter message (less than 1024 chars)");
	scanf("%s", buffer);

	if(sendto(sock, buffer, strlen(buffer), 0, (sockaddr *)&to, sizeof(to)) < 0)
	{
		perror("sendto()");
		exit(EXIT_FAILURE);
	}

	char buffer_rcv[BUFFER_SIZE];

	int n;
	socklen_t tosize = sizeof(to);
	if((n = recvfrom(sock, buffer_rcv, sizeof(buffer_rcv) - 1, 0, (sockaddr *)&to, &tosize)) < 0)
	{
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}

	buffer_rcv[n] = '\0';
	puts(buffer_rcv);
}

