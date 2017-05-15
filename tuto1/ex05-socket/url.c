/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"url.h"

/**
 * parse a URL and store the information in info.
 */

void parse_url(char* url, url_info *info)
{
	// url format: [http://]<hostname>[:<port>]/<path>
	// e.g. https://www.polytechnique.edu:80/index.php
	info->url = url;

	char* separator = strstr(url, "://");
	if(!separator || strlen(separator) < 3 || separator[0] != ':' || separator[1] != '/' || separator[2] != '/')
		exit_with_error("bad separator between protocol and host");
	separator = &separator[1];

	char* tokens = strtok(url, ":/");
	if(!strlen(tokens)) exit_with_error("protocol str empty");
	//cannot contain : or / because of strtok way of working
	info->protocol = tokens;

	separator = strstr(separator, ":");
	if(!separator || strlen(separator) < 1 || separator[0] != ':')
		exit_with_error("bad separator between host and port");
	separator = &separator[1];

	tokens = strtok(NULL, ":/");
	if(!strlen(tokens)) exit_with_error("host str empty");
	//cannot contain : or / because of strtok way of working
	info->host = tokens;

	separator = strstr(separator, "/");
	if(!separator || strlen(separator) < 1 || separator[0] != '/')
		exit_with_error("bad separator between port and path");
	separator = &separator[1];

	tokens = strtok(NULL, ":/");
	unsigned long int tmpport = strtoul(tokens, NULL, 10);
	if(tmpport == 0L)
		exit_with_error("port is not an int");
	info->port = (int)tmpport;

	info->path = (char*) malloc(sizeof(url));
	tokens = strtok(NULL, "/");
	strcpy(info->path, (tokens?tokens:"/"));
	tokens = strtok(NULL, "/");
	while(tokens) {
		strcat(info->path, "/");
		strcat(info->path, tokens);
		tokens = strtok(NULL, "/");
	}
}

/**
 * print the url info to std output
 */
void print_url_info(url_info info){
	printf("The URL contains following information: \n");
	printf("Protocol type:\t%s\n", info.protocol);
	printf("Host name:\t%s\n", info.host);
	printf("Port No.:\t%d\n", info.port);
	printf("Path:\t\t%s\n", info.path);
}

/**
 * exit with an error message
 */

void exit_with_error(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}
