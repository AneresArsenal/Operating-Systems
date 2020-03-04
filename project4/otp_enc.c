#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>

// global variables
#define maxchars 63999
int countFile;
int countKey;

void error(const char *msg)
{
	perror(msg);
	exit(0);
} // Error function used for reporting issues

void receiveData(int socketFD, char *string);
void sendData(int socketFD, char *string);
int checkLength(char file[], char key[]);
void readFile(char filepath[], char *array);

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent *serverHostInfo;
	char buffer[maxchars];

	int counter;
	// printf("Program Name Is: %s", argv[0]);
	// if (argc == 1)
	// 	printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
	// if (argc >= 2)
	// {
	// 	printf("\nNumber Of Arguments Passed: %d", argc);
	// 	printf("\n----Following Are The Command Line Arguments Passed----\n");
	// 	for (counter = 0; counter < argc; counter++)
	// 		printf("argv[%d]: %s\n", counter, argv[counter]);
	// }

	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
		exit(0);
	}

	// printf("Test: before check length\n");
	// printf("File path is %s, key path is %s", argv[1], argv[2]);

	if (checkLength(argv[1], argv[2]) == -1)
	{
		exit(1);
	}

	char filestring[maxchars], keystring[maxchars];
	memset(filestring, '\0', sizeof(filestring));
	memset(keystring, '\0', sizeof(keystring));

	readFile(argv[1], filestring);
	readFile(argv[2], keystring);

	// printf("File string is \n%s\n", filestring);
	// printf("Key string is \n%s\n", keystring);

	// Set up the server address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]);									 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;							 // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);					 // Store the port number
	serverHostInfo = gethostbyname("localhost");				 // Convert the machine name into a special form of address

	if (serverHostInfo == NULL)
	{
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	memcpy((char *)&serverAddress.sin_addr.s_addr, (char *)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0)
		error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	sendData(socketFD, filestring);
	// memset(buffer, '\0', sizeof(buffer));
	// sprintf(buffer, "%s", filestring);

	// charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	// if (charsWritten < 0)
	// 	error("CLIENT: ERROR writing to socket");
	// if (charsWritten < strlen(buffer))
	// 	error("CLIENT: WARNING: Not all data written to socket!");

	// Get return message from server
	char message[maxchars];
	memset(message, '\0', sizeof(message));
	receiveData(socketFD, message);

	sendData(socketFD, keystring);

	// memset(buffer, '\0', sizeof(buffer));
	// sprintf(buffer, "%s", keystring);

	// charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	// if (charsWritten < 0)
	// 	error("CLIENT: ERROR writing to socket");
	// if (charsWritten < strlen(buffer))
	// 	error("CLIENT: WARNING: Not all data written to socket!");

	// printf("CLIENT: I received this from the server:\n");

	// Get return message from server
	receiveData(socketFD, message);

	close(socketFD); // Close the socket
	return 0;
}

void receiveData(int socketFD, char *string)
{
	int charsReceived;
	char buffer[maxchars];
	memset(buffer, '\0', maxchars);

	// Read the client's message from the socket
	charsReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	if (charsReceived < 0)
	{
		error("ERROR reading from socket");
	}
	// printf("%s", buffer);
	sprintf(string, "%s", buffer);
	printf("CLIENT: I received this from the server: %s", buffer);
	
}

void sendData(int socketFD, char *string)
{
	// char buffer[maxchars];
	// memset(buffer, '\0', maxchars);
	int charsWritten;

	charsWritten = send(socketFD, string, strlen(string), 0);
	if (charsWritten < 0)
		error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(string))
		error("CLIENT: WARNING: Not all data written to socket!");
}

int checkLength(char filepath[], char key[])
{

	countFile = 0;
	countKey = 0;
	int file_descriptor, key_descriptor;
	FILE *file, *keyfile;
	char line[maxchars];
	// file = fopen(filepath, "r");

	// printf("File path is %s, key path is %s\n", filepath, key);

	file_descriptor = open(filepath, O_RDONLY);

	if (file_descriptor == -1)
	{
		printf("Hull breach - open() failed on \"%s\"\n", filepath);
		exit(1);
	}

	lseek(file_descriptor, 0, SEEK_SET);
	file = fopen(filepath, "r");
	while (fgets(line, sizeof(line), file))
	{
		countFile += strlen(line);
	}

	fclose(file);

	key_descriptor = open(key, O_RDONLY);

	if (key_descriptor == -1)
	{
		printf("Hull breach - open() failed on \"%s\"\n", key);
		exit(1);
	}

	lseek(key_descriptor, 0, SEEK_SET);
	keyfile = fopen(key, "r");
	while (fgets(line, sizeof(line), keyfile))
	{
		countKey += strlen(line);
	}

	fclose(keyfile);

	if (countFile <= countKey)
	{
		// printf("key length (%i) is longer or equal to plaintext length (%i)\n", countKey, countFile);
		return 0;
	}
	else
	{
		error("key is too short");
		return -1;
	}

	return 0;
}

void readFile(char filepath[], char array[maxchars])
{
	int i = 0;
	int file_descriptor;
	char line[maxchars];
	ssize_t nread, nwritten;
	char readBuffer[maxchars];
	char *ptr = readBuffer;

	memset(readBuffer, '\0', maxchars);
	memset(line, '\0', maxchars);

	FILE *file;

	file_descriptor = open(filepath, O_RDWR);

	if (file_descriptor == -1)
	{
		printf("Hull breach - open() failed on \"%s\"\n", filepath);
		exit(1);
	}

	lseek(file_descriptor, 0, SEEK_SET);
	file = fopen(filepath, "r");
	while (fgets(line, sizeof(line), file))
	{
		if (i == 0)
		{
			strcpy(array, line);
		}

		else
		{
			strcat(array, line);
		}
		i++;
		// printf("Current line: %s\n", line);
	}

	fclose(file);

	// array = readBuffer;
	// printf("Final string is %s", readBuffer);
	// sprintf(array, "%s", readBuffer);
}