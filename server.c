#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>

typedef struct sockaddr_in t_sockaddr;

int main()
{
	int server_fd = -1;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket");
		return 0;
	}
	printf("socket: %d\n", server_fd);

	t_sockaddr address;
	memset((char *)&address, 0, sizeof(address));
	int addrlen = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		return 0;
	}
	printf("bound successfully\n");
	if (listen(server_fd, 3) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}
	int new_socket = -1;
	long valread;
	while (1)
	{
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}
		char buffer[30000] = {0};
		valread = read( new_socket , buffer, 30000);
		(void)valread;
		printf("%s\n",buffer );
		write(new_socket , "hello" , strlen("hello"));
		close(new_socket);
	}
	return (0);
}