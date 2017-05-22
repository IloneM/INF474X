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

#define BUFFER_SIZE 1025
//#define NB_THREADS 10

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct in_addr in_addr;

typedef struct Info {
	char* buffer;
	int bufferlen;
	sockaddr_in* sender;
	socklen_t senderlen;
} Info;

//#define TYPE Info*
#include "stack.h"

typedef Info* InfoPtr;
//typedef pthread_t Thread;

define_stack(InfoPtr)
//define_stack(Thread)

Stack_InfoPtr* info_stack = NULL;
pthread_mutex_t info_stack_mutex;

//pthread_mutex_t buffer_lock, socket_lock;

void* sendworker(void* data) {
	Info* workinfo = (Info*) data;

	sockaddr_in sin = { 0 };
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(0);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	usleep(1000);

	if(bind(sock, (sockaddr *) &sin, sizeof(sin)) > 0)
//	if(bind(sock, (sockaddr *) workinfo->sender, workinfo->senderlen) > 0)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}
//	if(bind(sock, (sockaddr *) workinfo->sender, sizeof(workinfo->sender)) > 0)
//	{
//		perror("bind()");
//		exit(EXIT_FAILURE);
//	}
	printf("%d\n", sock);
	printf("buffer: %s\nbufferlen: %d\nsenderlen: %d\nip: %d\n", workinfo->buffer, workinfo->bufferlen, workinfo->senderlen, workinfo->sender->sin_addr.s_addr);
	if(sendto(sock, workinfo->buffer, workinfo->bufferlen, 0, (sockaddr *) workinfo->sender, workinfo->senderlen) < 0)
	{
		perror("sendto()");
		exit(EXIT_FAILURE);
	}
	if(close(sock) > 0)
	{
		perror("close()");
		exit(EXIT_FAILURE);
	}

	free(workinfo->buffer);
	free(workinfo->sender);
	free(workinfo);

//	printf("ended!\n");

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
			pthread_create(&thread_tmp, NULL, sendworker, (void*)newInfo);
		}
		usleep(100);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		perror("Host must have exactly 1 parameter as input: port. \n");
		exit(EXIT_FAILURE);
	}

	char *end;
    long port = strtol(argv[1], &end, 10);

	sockaddr_in sin = { 0 };

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(port);

//	int sock;
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(bind(sock, (sockaddr *) &sin, sizeof sin) > 0)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	printf("%d\n", sock);

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

			//pthread_mutex_lock(&socket_lock);
			if((next_info->bufferlen = recvfrom(sock, next_info->buffer, BUFFER_SIZE - 1, 0, (sockaddr *)next_info->sender, &(next_info->senderlen))) < 0)
			{
				perror("recvfrom()");
				exit(EXIT_FAILURE);
			}
//			if(close(sock) > 0)
//			{
//				perror("close()");
//				exit(EXIT_FAILURE);
//			}
			//pthread_mutex_unlock(&socket_lock);
			next_info->buffer[next_info->bufferlen] = '\0';

			printf("%d", next_info->sender->sin_addr.s_addr);

			pthread_mutex_lock(&info_stack_mutex);
			Stack_push_InfoPtr(&info_stack, next_info);
//			puts(next_info->buffer);
			pthread_mutex_unlock(&info_stack_mutex);


//			printf("waiting3\n");

//			pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
//	void *(*start_routine)(void *), void *arg);
	}

	return EXIT_SUCCESS;
}

