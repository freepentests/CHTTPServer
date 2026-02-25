#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./utils.h"

void send_file(int *socket, char *filename)
{
	if (strstr(filename, ".."))
	{
		send(*socket, "HTTP/1.1 400 Bad Request\nContent-Size: 30\nContent-Type: text/html\nServer: custom c serv\ndon't try to do path traversal", 118, 0);
		return;

	}

	FILE *fptr = fopen(filename, "r");

	if (fptr == NULL)
	{
		send(*socket, "HTTP/1.1 404 Not Found\nContent-Size: 13\nContent-Type: text/html\nServer: custom c serv\n404 not found", 103, 0);
		return;
	}

	size_t content_length = get_file_length(filename);

	char *content = malloc(content_length);
	if (content == NULL)
	{
		fclose(fptr);
		return;
	}
	fread(content, sizeof(char), content_length, fptr);

	char buffer[4096];
	snprintf(buffer, 4096, "HTTP/1.1 200 OK\nContent-Length: %zu\nContent-Type: %s\nServer: custom c serv\n\n", content_length, get_mime_type(filename));

	send(*socket, buffer, strlen(buffer), 0);
	send(*socket, content, content_length, 0);

	free(content);
	fclose(fptr);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("You must specify a port number in your arguments. Example usage:\n\n./program 8080\n^ Listens on port 80\n");
		return EXIT_FAILURE;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	int addr_len = sizeof(serv_addr);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	bind(sockfd, (struct sockaddr *)&serv_addr, addr_len);
	listen(sockfd, 3);

	while (true)
	{
		int new_socket = accept(sockfd, (struct sockaddr *)&serv_addr, &addr_len);

		char buffer[2048];
		recv(new_socket, buffer, 2048, 0);
		char method[3], path[256], protocol[64];

		sscanf(buffer, "%3s %256s %64s", method, path, protocol);

		char dir[263] = "static";
		if (strcmp(path, "/") == 0)
		{
			strcat(dir, "/index.html");
		} else 
		{
			strncat(dir, path, strlen(path));
		}
		send_file(&new_socket, dir);
		dir[263] = '\0';

		close(new_socket);
	}

	return EXIT_SUCCESS;
}

