/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "url.h"
#include "wgetX.h"

int main(int argc, char* argv[])
{

	url_info info;

	char *url;
	if (argc != 2) {
		//		exit_with_error("The wgetX must have exactly 1 parameter as input. \n");
		char tempURL[] = "http://www.rfc-editor.org/rfc/rfc791.txt";
		url = tempURL;
		printf("URL not defined. Using default URL http://www.rfc-editor.org/rfc/rfc791.txt \n");
	} else
		url = argv[1];

	printf("Downloading %s \n", url);

	//get the url
	parse_url(url, &info);

	//	print_url_info(info);

	//download page
	char *recv_buf_t;
	recv_buf_t = malloc(sizeof(char)*B_SIZE);
	memset(recv_buf_t, 0, sizeof(char)*B_SIZE); //initialize the memory
	char *buff = download_page(info, recv_buf_t); //download the page and return the pointer to the downloaded buffer
//	puts(buff); //print it out

	//write to the file. the author is so lazy that he didn't take the original file name
	write_data("received_page", buff);

	free(recv_buf_t);

	puts("the file is saved in received_page.");
	return (EXIT_SUCCESS);
}


char* download_page(url_info info, char *recv_buf_t)
{
	//BEGIN_SOLUTION
	int socketfd;

	struct sockaddr server_addr;


	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if(socketfd < 0 ){
		exit_with_error("ERROR opening socket");
	}

	// there are two ways to obtain the IP address of the host: using gethostbyname, or getaddrinfo. The latter is recommended

	/////using gethostbyname
	//		struct hostent *server;
	//		server = gethostbyname(info.host);
	//		if(server == NULL){
	//			exit_with_error("ERROR: no such server");
	//		}
	//
	//		bzero((char *)(&server_addr), sizeof(server_addr));
	//
	//		struct sockaddr_in *server_addr_in = (struct sockaddr_in*)(&server_addr);
	//		server_addr_in->sin_family = AF_INET;
	//		bcopy(server->h_addr,
	//				&(server_addr_in->sin_addr.s_addr),
	//				server->h_length);
	//		server_addr_in->sin_port = htons(info.port);
	/////// end using gethostbyname


	/////////// using getaddrinfo
	struct addrinfo hints, *servinfo; //the servinfo should be freed in the end...
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char str[5];
	sprintf(str,"%d",info.port); //the port or
	if((rv = getaddrinfo(info.host, str, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s, %s, %d\n", gai_strerror(rv), info.host, info.port);
		exit_with_error("ERROR: not such server");
	}
	server_addr = *(servinfo->ai_addr);
	///////////////end using getaddrinfo

	if(connect(socketfd, servinfo->ai_addr, sizeof(server_addr))<0){
		perror("client connection:");
		exit_with_error("ERROR in connecting");
	}

	char request_buffer[1024];
	memset(request_buffer, 0, sizeof(request_buffer));
	snprintf(request_buffer, 1024, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
			info.path, info.host);

	// it's possible to use write() or send()
	//	int n = write(socketfd, request_buffer, strlen(request_buffer));
	int n = send(socketfd, request_buffer, strlen(request_buffer), 0);

	if(n < 0){
		perror("client writing:");
		exit_with_error("ERROR writing to the socket");
	}

	memset(recv_buf_t, 0, B_SIZE); //the buffer for storing the whole page

	char* temp_recv_buf  = malloc(sizeof(char)*4096); //a temp buffer for each reading
	while(1) { //we need to read until the transmission is finished
		int bytes;

		memset(temp_recv_buf,0, sizeof(char)*4096);

		// it's possible to use read() or recv(). read() is more general, and recv() is more specific to TCP socket
		//		bytes = read(socketfd, temp_recv_buf, 4096);
		bytes = recv(socketfd, temp_recv_buf, 4096, 0);

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
	close(socketfd);

	//END_SOLUTION
	return recv_buf_t;
}

void write_data(const char *path, const char * data)
{
	//BEGIN_SOLUTION
	FILE *fp = fopen(path, "w");
	if (fp==NULL){
		exit_with_error("ERROR opening file");
	}

	fprintf(fp, "%s", data);

	fclose(fp);
	//END_SOLUTION
}
