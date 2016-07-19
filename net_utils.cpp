#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

int make_socket_non_blocking (int sfd) {
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror("fcntl");
		return -1;
	}

	return 0;
}
int create_and_bind(const char *hostname, unsigned int port) {
	int sfd = socket(
				AF_INET, //Ipv4   еще есть AF_INET6 - IPv6, AF_UNIX - unix socket
				SOCK_STREAM, //TCP,  еще есть SOCK_DGRAM for UDP
				IPPROTO_TCP); //   еще есть IPPROTO_UDP; также можно просто указать 0, тогда будет выбираться протокол по умолчанию

	struct sockaddr_in sa;
	sa.sin_family = AF_INET; //
	sa.sin_port = htons(port);
	if(inet_aton(hostname, &(sa.sin_addr)) == -1) {
		perror("inet_aton");
		return -1;
	}
	//sa.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0
	//sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //127.0.0.1
	int enable = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
		return -1;
	}
	if( bind(sfd, (struct sockaddr *)(&sa), sizeof(sa)) != 0) {
		perror("bind");
		return -1;
	}

	return sfd;
}

int create_and_bind(char *port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, retval;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;     // Return IPv4 and IPv6 choices.
	hints.ai_socktype = SOCK_STREAM; // We want a TCP socket.
	hints.ai_flags = AI_PASSIVE;     // All interfaces.

	retval = getaddrinfo(NULL, port, &hints, &result);
	if (retval != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		int enable = 1;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
			perror("setsockopt(SO_REUSEADDR) failed");
			return -1;
		}
		retval = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (retval == 0) {
			// We managed to bind successfully!
			break;
		}

		close(sfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);

	return sfd;
}
