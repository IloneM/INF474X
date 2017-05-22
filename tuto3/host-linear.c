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
	if (argc != 2) {
		perror("The wgetX must have exactly 1 parameter as input. \n");
		exit(EXIT_FAILURE);
	}

	char *end;
    long port = strtol(argv[1], &end, 10);

	sockaddr_in sin = { 0 };

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(port);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	int n;
	sockaddr_in from = { 0 };
	int fromsize = sizeof from;
	char buffer[BUFFER_SIZE];

	while(1) {
		if((n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&from, &fromsize)) < 0)
		{
			perror("recvfrom()");
			exit(EXIT_FAILURE);
		}

		printf("%d", next_info->sender->sin_addr.s_addr);
		buffer[n] = '\0';

		if(sendto(sock, buffer, strlen(buffer), 0, (sockaddr *)&from, fromsize) < 0)
		{
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

