// #include <sys/types.h>
// #include <sys/socket.h>
// #include <stdio.h>
// #include <netinet/in.h>
// #include <signal.h>
// #include <unistd.h>

// int main()
// {
// 	int listenSocketFD, establishedConnectionFD;
// 	int server_len, sizeOfClientInfo;
// 	struct sockaddr_in serverAddress;
// 	struct sockaddr_in clientAddress;

// 	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);

// 	serverAddress.sin_family = AF_INET;
// 	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
// 	serverAddress.sin_port = htons(9734);
// 	server_len = sizeof(serverAddress);
// 	bind(listenSocketFD, (struct sockaddr *)&serverAddress, server_len);

// 	/* Create a connection queue, ignore child exit details and wait for
// clients. */

// 	listen(listenSocketFD, 5);

// 	signal(SIGCHLD, SIG_IGN);

// 	while (1)
// 	{
// 		char ch;

// 		printf("server waiting\n");

// 		/* Accept connection. */

// 		sizeOfClientInfo = sizeof(clientAddress);
// 		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);

// 		/* Fork to create a process for this client and perform a test to see
// whether we're the parent or the child. */

// 		if (fork() == 0)
// 		{

// 			/* If we're the child, we can now read/write to the client on
// establishedConnectionFD.
// The five second delay is just for this demonstration. */

// 			read(establishedConnectionFD, &ch, 1);
// 			sleep(5);
// 			ch++;
// 			write(establishedConnectionFD, &ch, 1);
// 			close(establishedConnectionFD);
// 			exit(0);
// 		}

// 		/* Otherwise, we must be the parent and our work for this client is
// finished. */

// 		else
// 		{
// 			close(establishedConnectionFD);
// 		}
// 	}
// }