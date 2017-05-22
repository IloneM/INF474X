#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "stack.h"

#define BUFFER_SIZE 1025
#define NB_THREADS 10

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct in_addr in_addr;


int n;
sockaddr_in from = { 0 };
int fromsize = sizeof from;

char buffer[BUFFER_SIZE];

int threadsit = 0;
pthread_t threads[NB_THREADS] = {0};

pthread_mutex_t buffer_lock, socket_lock;

void *sendroutine() {
	sockaddr_in sin = { 0 };
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(0);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in fromcpy = from;

	int ncpy = n;
	char* buffercpy = (char*)malloc(sizeof(char) * (ncpy+1));

	strcpy(buffercpy, buffer);

//	puts(buffercpy);

	pthread_mutex_unlock(&buffer_lock);

	usleep(1000000);

	if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	printf("%d\n", sock);
	if(sendto(sock, buffercpy, ncpy, 0, (sockaddr *)&fromcpy, sizeof(fromcpy)) < 0)
	{
		perror("sendto()");
		exit(EXIT_FAILURE);
	}
	if(close(sock) > 0)
	{
		perror("close()");
		exit(EXIT_FAILURE);
	}

	free(buffercpy);

	printf("ended!\n");

	return NULL;
}

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

	int sock;
//	int sock = socket(AF_INET, SOCK_DGRAM, 0);

//	if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
//	{
//		perror("bind()");
//		exit(EXIT_FAILURE);
//	}

	if (pthread_mutex_init(&buffer_lock, NULL) != 0){
        printf("buffer lock mutex init failed\n");
        return 1;
    }
	if (pthread_mutex_init(&socket_lock, NULL) != 0){
        printf("socket lock mutex init failed\n");
        return 1;
    }

	while(1) {
			if(threads[threadsit]) {
				pthread_join(threads[threadsit], NULL);
			}
			printf("waiting1\n");

			pthread_mutex_lock(&buffer_lock);

			sock = socket(AF_INET, SOCK_DGRAM, 0);
			printf("%d\n", sock);
			if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
			{
				perror("bind()");
				exit(EXIT_FAILURE);
			}

			printf("waiting2\n");
			//pthread_mutex_lock(&socket_lock);
			if((n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&from, &fromsize)) < 0)
			{
				perror("recvfrom()");
				exit(EXIT_FAILURE);
			}
			if(close(sock) > 0)
			{
				perror("close()");
				exit(EXIT_FAILURE);
			}
			//pthread_mutex_unlock(&socket_lock);
			buffer[n] = '\0';

			printf("waiting3\n");

			pthread_create(&threads[threadsit], NULL, sendroutine, NULL);
			threadsit = (threadsit+1) % NB_THREADS;
//			pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
//	void *(*start_routine)(void *), void *arg);
	}

	return EXIT_SUCCESS;
}

