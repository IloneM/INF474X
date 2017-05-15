/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "url.h"
#include "wgetX.h"

/*
	#define MAXRCVLEN 500
   char buffer[MAXRCVLEN + 1];
   int len, mysocket;
   struct sockaddr_in dest;

   mysocket = socket(AF_INET, SOCK_STREAM, 0);
   memset(&dest, 0, sizeof(dest));
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   dest.sin_port = htons(5000);
   connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));

   len = recv(mysocket, buffer, MAXRCVLEN, 0);
   buffer[len] = '\0';
   printf("Received %s (%d bytes).\n", buffer, len);
   close(mysocket);
   return 0;
*/
char *build_get_query(char *host, char *page);

int main(int argc, char* argv[])
{

	url_info info;


	if (argc != 2) {
		exit_with_error("The wgetX must have exactly 1 parameter as input. \n");
	}
	char *url = argv[1];

	printf("Downloading %s \n", url);

	//get the url
	parse_url(url, &info);

	print_url_info(info);

	//download page
	char *recv_buf_t;
	recv_buf_t = malloc(sizeof(char)*B_SIZE);
	bzero(recv_buf_t, sizeof(recv_buf_t));
	char *buff = download_page(info, recv_buf_t);
	puts(buff);

	//write to the file
	write_data("received_page", buff);

	free(recv_buf_t);

	puts("the file is saved in received_page.");
	return (EXIT_SUCCESS);
}

char* download_page(url_info info, char *recv_buf_t)
{
   //int len, mysocket;
   int mysocket;
   struct sockaddr_in dest;

   mysocket = socket(AF_INET, SOCK_STREAM, 0);
   memset(&dest, 0, sizeof(dest));
   dest.sin_family = AF_INET;
   dest.sin_port = htons(info.port);
   dest.sin_addr = *(struct in_addr *) gethostbyname(info.host)->h_addr;
//   dest.sin_addr.s_addr = inet_addr(gethostbyname(info.host)->h_addr);
//   dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	
	char* get = build_get_query(info.host, info.path);

	int tmpres,sent = 0;
	while(sent < strlen(get))
	{
		tmpres = send(mysocket, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1){
			perror("Can't send query");
			exit(1);
		}
		sent += tmpres;
	}

	char* temp_recv_buf  = malloc(sizeof(char)*4096); //a temp buffer for each reading
	while(1) { //we need to read until the transmission is finished
		int bytes;

		memset(temp_recv_buf,0, sizeof(char)*4096);

		// it's possible to use read() or recv(). read() is more general, and recv() is more specific to TCP socket
		//		bytes = read(socketfd, temp_recv_buf, 4096);
		bytes = recv(mysocket, temp_recv_buf, 4096, 0);

		if (bytes == 0) { //reading finished
			break;
		}

		//		sprintf(recv_buf + strlen(recv_buf), "%s",temp_recv_buf); //work code

		memcpy(recv_buf_t+strlen(recv_buf_t), temp_recv_buf, strlen(temp_recv_buf)); //copy the temp buffer to the "big" buffer
	}
	free(temp_recv_buf);

	//	puts(recv_buf);

	// now begin parsing the http reply

	//first line, get the status code
	char *status_line = strstr(recv_buf_t, "\r\n");
	*status_line = '\0';
	//		puts(recv_buf_t);
	char status[4];
	memcpy(status, recv_buf_t + 9, 3); //get the status string
	status[3] = '\0';
	int status_code = atoi(status);

	recv_buf_t = status_line + 2; //now move to the the next line.

	char* page;
	switch(status_code){
	case 200: //all ok

		page = strstr(recv_buf_t, "\r\n\r\n");
		*page = '\0';
		recv_buf_t = page + 4; //now the recv_buf pointer is pointing to the begin of the document

		break;
	case 302: //redirect
		//do the redirect here
		break;

	}
//	close(socketfd);
//   len = recv(mysocket, recv_buf_t, B_SIZE, 0);
//   recv_buf_t[len] = '\0';
//   printf("Received %s (%d bytes).\n", recv_buf_t, len);
   close(mysocket);

	return recv_buf_t;
}

void write_data(const char *path, const char * data)
{
	FILE* output = fopen(path, "w");
	fputs(data, output);
	fclose(output);
}

#define USERAGENT "HTMLGET 1.0"

char *build_get_query(char *host, char *page)
{
  char *query;
  char *getpage = page;
  char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
  if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0
  query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, getpage, host, USERAGENT);
  return query;
}

