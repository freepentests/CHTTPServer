#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct MimeType
{
	char *extension;
	char *mime_type;
} mimetype_t;

const mimetype_t mime_types[] = 
{
	{".txt",  "text/plain"},
	{".html", "text/html"},
	{".htm",  "text/html"},
	{".css",  "text/css"},
	{".js",   "application/javascript"},
	{".jpg",  "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".png",  "image/png"},
	{".gif",  "image/gif"},
	{".bmp",  "image/bmp"},
	{".pdf",  "application/pdf"},
	{".doc",  "application/msword"},
	{".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
	{".xls",  "application/vnd.ms-excel"},
	{".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
	{".zip",  "application/zip"},
	{".gz",   "application/gzip"},
	{".tar",  "application/x-tar"},
	{".mp3",  "audio/mpeg"},
	{".wav",  "audio/wav"},
	{".mp4",  "video/mp4"},
	{".mov",  "video/quicktime"},
	{".avi",  "video/x-msvideo"},
	{NULL, NULL}
};

char *get_extension(char *filename)
{
	char *ext = strrchr(filename, '.');
	if (ext == NULL)
	{
		return NULL;
	}

	return ext;
}

char *get_mime_type(char *filename)
{
	char *ext = get_extension(filename);

	if (ext == NULL)
	{
		return "application/octet-stream";
	}

	for (int i = 0; i < sizeof(mime_types); i++)
	{
		if (strcmp(mime_types[i].extension, ext) == 0)
		{
			return mime_types[i].mime_type;
		}
	}

	return "application/octet-stream";
}

size_t get_file_length(char *filename)
{
    size_t file_length = 0;

    FILE *fptr = fopen(filename, "r");

    while (fgetc(fptr) != EOF) file_length++;

    fclose(fptr);

    return file_length;
}

void send_file(int *socket, char *filename)
{
	printf("%s\n", filename);
	FILE *fptr = fopen(filename, "r");

	if (fptr == NULL)
	{
		send(*socket, "HTTP/1.1 404 Not Found\nContent-Size: 13\nContent-Type: text/html\nServer: custom c serv\n404 not found", 103, 0);
		return;
	}

	size_t content_length = get_file_length(filename);

	char *content = malloc(content_length);
	fread(content, sizeof(char), content_length, fptr);

	char buffer[4096];
	snprintf(buffer, 4096, "HTTP/1.1 200 OK\nContent-Length: %zu\nContent-Type: %s\nServer: custom c serv\n\n", content_length, get_mime_type(filename));

	send(*socket, buffer, strlen(buffer), 0);
	send(*socket, content, content_length, 0);

	free(content);
	fclose(fptr);
}

int main(void)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	int addr_len = sizeof(serv_addr);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(4457);

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
		printf("%s\n", path);
		strncat(dir, path, strlen(path));
		send_file(&new_socket, dir);
		dir[263] = '\0';

		close(new_socket);
	}

	return EXIT_SUCCESS;
}

