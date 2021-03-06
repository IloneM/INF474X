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
#include <errno.h>

#include "stack.h"

#define BUFFER_SIZE 1025
#define NB_TIMEOUTS 10
#define TIMEOUT 100000 //in micro s
#define WORKTIME_SIM 100000 //in micro s

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct in_addr in_addr;

typedef struct Info {
	char* buffer;
	int bufferlen;
	sockaddr_in* sender;
	socklen_t senderlen;
} Info;

typedef Info* InfoPtr;
//typedef pthread_t Thread;

define_stack(InfoPtr)
//define_stack(Thread)

Stack_InfoPtr* info_stack = NULL;
pthread_mutex_t info_stack_mutex;

void* sendworker(void* data) {
	Info* workinfo = (Info*) data;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error");
	}

	while(1) {
		usleep(WORKTIME_SIM);

		if(sendto(sock, workinfo->buffer, workinfo->bufferlen, 0, (sockaddr *) workinfo->sender, workinfo->senderlen) < 0)
		{
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		//if((workinfo->bufferlen = recvfrom(sock, workinfo->buffer, BUFFER_SIZE - 1, MSG_DONTWAIT, (sockaddr *)workinfo->sender, &(workinfo->senderlen))) < 0)
		if((workinfo->bufferlen = recvfrom(sock, workinfo->buffer, BUFFER_SIZE - 1, 0, (sockaddr *)workinfo->sender, &(workinfo->senderlen))) < 0)
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("recvfrom()");
				exit(EXIT_FAILURE);
			}
			int nbtimeout = 1;
			while((workinfo->bufferlen = recvfrom(sock, workinfo->buffer, BUFFER_SIZE - 1, 0, (sockaddr *)workinfo->sender, &(workinfo->senderlen))) < 0 && nbtimeout < NB_TIMEOUTS) nbtimeout++;
			if(nbtimeout >= NB_TIMEOUTS) {
				fprintf(stderr, "Timeout reached. Ressource is considered unavailable. Closing.\n");
				break;
			}
		}
	}

	if(close(sock) > 0)
	{
		perror("close()");
		exit(EXIT_FAILURE);
	}

	free(workinfo->buffer);
	free(workinfo->sender);
	free(workinfo);

	return NULL;
}

void *manager_routine() {
//	Stack_Thread* thread_stack = NULL;
	pthread_t thread_tmp;

	while(1) {
		if(info_stack) {
			pthread_mutex_lock(&info_stack_mutex);
			Info* newInfo = Stack_pull_InfoPtr(&info_stack);
			pthread_mutex_unlock(&info_stack_mutex);

//			Stack_push_Thread(&thread_stack, 0);
			printf("New connection received\n");
			pthread_create(&thread_tmp, NULL, sendworker, (void*)newInfo);
		}
		usleep(1000);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		perror("Host must have exactly 1 parameter as input: port\n");
		exit(EXIT_FAILURE);
	}

	char *end;
    long port = strtol(argv[1], &end, 10);

	sockaddr_in sin = { 0 };
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(&info_stack_mutex, NULL) != 0){
        printf("info stack mutex init failed\n");
        return 1;
    }

	Info* next_info = NULL;

	pthread_t thread_tmp;
	pthread_create(&thread_tmp, NULL, manager_routine, NULL);

	while(1) {
			next_info = malloc(sizeof(Info));
			next_info->buffer = malloc(BUFFER_SIZE*sizeof(char));
			next_info->sender = malloc(sizeof(sockaddr_in));
			memset(next_info->sender, 0, sizeof(sockaddr_in));
			next_info->senderlen = sizeof(next_info->sender);

			if((next_info->bufferlen = recvfrom(sock, next_info->buffer, BUFFER_SIZE - 1, 0, (sockaddr *)next_info->sender, &(next_info->senderlen))) < 0)
			{
				perror("recvfrom()");
				exit(EXIT_FAILURE);
			}
			next_info->buffer[next_info->bufferlen] = '\0';

			pthread_mutex_lock(&info_stack_mutex);
			Stack_push_InfoPtr(&info_stack, next_info);
			pthread_mutex_unlock(&info_stack_mutex);
	}

	return EXIT_SUCCESS;
}

