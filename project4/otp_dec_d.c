#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// global variables
#define maxchars 80000
#define maxbuffer 63000

void error(const char *msg)
{
	perror(msg);
	exit(1);
} // Error function used for reporting issues

void receiveData(int establishedConnectionFD, char *string);
void sendSuccessMessage(int establishedConnectionFD);
void sendData(int establishedConnectionFD, char *encryptedFile);
void decryptFile(char *file, char *key, char *encrypted);

int main(int argc, char *argv[])
{

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten;
	socklen_t sizeOfClientInfo;
	char buffer[maxchars];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]);									 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;							 // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);					 // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY;					 // Any address is allowed for connection to this process

	// Set up the socket
	// Create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0)
		error("ERROR opening socket");

	if (setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		error("setsockopt(SO_REUSEADDR) failed");

	// Enable the socket to begin listening
	// Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		// perror("Hull breach: bind()");
		// exit(1);
		error("ERROR on binding");
	}

	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress);																// Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0)
	{
		error("ERROR on accept");
	}

	/* Fork to create a process for this client and perform a test to see
whether we're the parent or the child. */

	// printf("SERVER: Connected client at port %d\n", ntohs(clientAddress.sin_port));

	char filestring[maxchars];
	receiveData(establishedConnectionFD, filestring);

	// Send a Success message back to the client
	sendSuccessMessage(establishedConnectionFD);

	char keystring[maxchars];
	receiveData(establishedConnectionFD, keystring);

	// Send a Success message back to the client
	sendSuccessMessage(establishedConnectionFD);

	// printf("\nSERVER: Server shutting down...\n\n");

	char dummy[maxchars];
	receiveData(establishedConnectionFD, dummy);

	// printf("File string is %s\n", filestring);
	// printf("Key string is %s\n", keystring);

	char encrypted[maxchars];
	decryptFile(filestring, keystring, encrypted);
	// printf("Encrypted file is %s", encrypted);

	sendData(establishedConnectionFD, encrypted);

	close(establishedConnectionFD); // Close the existing socket which is connected to the client

	close(listenSocketFD); // Close the listening socket

	return 0;
}


void receiveData(int establishedConnectionFD, char *string)
{
	int charsReceived;
	char buffer[maxbuffer];
	memset(buffer, '\0', maxbuffer);
	memset(string, '\0', maxchars);
	int bufferLen = 0;
	int i = 0;

	// Read the client's message from the socket
	while (1)
	{
		charsReceived = recv(establishedConnectionFD, buffer, sizeof(maxbuffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("SERVER: ERROR reading from socket");
		}
		// printf("current package is %s\n", buffer);

		if (i == 0)
		{
			strcpy(string, buffer);
		}

		else
		{
			strcat(string, buffer);
		}

		bufferLen = strlen(buffer);
		if ((buffer[bufferLen - 1]) == '\n')
		{
			// printf("Receiving finish!\n");
			break;
		}
		memset(buffer, '\0', maxbuffer);
		i++;
	}
	// printf("SERVER: I received this from the client: %s", buffer);
	// printf("SERVER: String saved as: %s", string);
}


// void receiveData(int establishedConnectionFD, char *string)
// {
// 	int charsReceived;
// 	char buffer[maxchars];
// 	memset(buffer, '\0', maxchars);
// 	memset(string, '\0', maxchars);

// 	// Read the client's message from the socket
// 	charsReceived = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);

// 	if (charsReceived < 0)
// 	{
// 		error("ERROR reading from socket");
// 	}
// 	// printf("%s", buffer);
// 	sprintf(string, "%s", buffer);
// 	printf("SERVER: I received this from the client: %s", buffer);
// 	// printf("SERVER: String saved as: %s", string);
// }

void sendSuccessMessage(int establishedConnectionFD)
{
	char buffer[maxchars];
	memset(buffer, '\0', maxchars);
	int charsWritten;

	// Send a Success message back to the client
	sprintf(buffer, "Success! I am the server, and I got your message\n");

	charsWritten = send(establishedConnectionFD, buffer, strlen(buffer), 0); // Send success back
	if (charsWritten < 0)
		error("ERROR writing to socket");

	printf("\nSERVER: Waiting for next data package....\n\n");
}

void sendData(int establishedConnectionFD, char *encryptedFile)
{
	int charsWritten;

	charsWritten = send(establishedConnectionFD, encryptedFile, strlen(encryptedFile), 0); // Send success back
	if (charsWritten < 0)
		error("ERROR writing to socket");

	printf("\nSERVER: Waiting for next data package....\n\n");
}

void decryptFile(char *file, char *key, char *encrypted)
{

	int fileLength = strlen(file) - 1;
	int i;
	int total;
	int result;
	int fileChar;
	int keyChar;
	char currentChar;

	for (i = 0; i < fileLength; i++)
	{

		if (file[i] != 32)
		{
			fileChar = file[i] - 64;
		}
		else
		{
			fileChar = 0;
		}

		if (key[i] != 32)
		{
			keyChar = key[i] - 64;
		}
		else
		{
			keyChar = 0;
		}

		total = fileChar - keyChar;

		if (total < 0)
		{
			total = total + 26;
		}
		result = total % 26;

		result = result + 64;

		// strncat(temp, &currentChar, sizeof(currentChar));
		if (result == 64)
		{
			currentChar = 32;
		}
		else if (result > 64 && result < 91)
		{
			currentChar = result;
		}

		else
		{
			error("SERVER: Bad input");
		}

		encrypted[i] = currentChar;
		// printf("Char %i   ", i);
		// printf("message: %i   ", fileChar);
		// printf("key: %i   ", keyChar);
		// printf("message + key: %i   ", total);
		// printf("ciphertext: %i  \n", encrypted[i]);
	}

	encrypted[i] = '\n';

	// to see all thevalue in temp array.
	// printf("Final encrpyted string is: [%s]\n", encrypted);
}
// reference
// https://stackoverflow.com/questions/13669474/multiclient-server-using-fork
// https://stackoverflow.com/questions/16007789/keep-socket-open-in-c