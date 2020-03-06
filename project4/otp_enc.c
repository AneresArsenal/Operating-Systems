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
#define maxchars 80000
#define maxbuffer 63000
int countFile;
int countKey;

void error(const char *msg)
{
	perror(msg);
	exit(0);
} // Error function used for reporting issues

void receiveData(int socketFD, char *string, int flag);
void sendData(int socketFD, char *string);
int checkLength(char file[], char key[]);
void readFile(char filepath[], char *array, char *array2);
void checkString(char *string);

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent *serverHostInfo;
	char buffer[maxchars];

	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
		exit(0);
	}

	if (checkLength(argv[1], argv[2]) == -1)
	{
		exit(1);
	}

	char filestring[maxchars], keystring[maxchars];
	char filestring2[maxchars], keystring2[maxchars];
	memset(filestring, '\0', sizeof(filestring));
	memset(keystring, '\0', sizeof(keystring));
	memset(filestring2, '\0', sizeof(filestring2));
	memset(keystring2, '\0', sizeof(keystring2));

	readFile(argv[1], filestring, filestring2);

	checkString(filestring);

	readFile(argv[2], keystring, keystring2);

	int fileLen, keyLen;
	fileLen = strlen(filestring);
	keyLen = strlen(keystring);

	// printf("File string length is %i, content is \n%s\n", fileLen, filestring);
	// printf("Key string length is %i, content is \n%s\n", keyLen, keystring);

	// Set up the server address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]);									 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;							 // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);					 // Store the port number
	serverHostInfo = gethostbyname("localhost");				 // Convert the machine name into a special form of address

	if (serverHostInfo == NULL)
	{
		fprintf(stderr, "ENC CLIENT: ERROR, no such host\n");
		exit(0);
	}
	memcpy((char *)&serverAddress.sin_addr.s_addr, (char *)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0)
		error("ENC CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("ENC CLIENT: ERROR connecting");

	sendData(socketFD, filestring);

	// Get return message from server
	
	char message[maxchars];
	receiveData(socketFD, message, 0);

	sendData(socketFD, keystring);

	// Get return message from server
	receiveData(socketFD, message, 0);

	// sendData(socketFD, "Waiting for encrypted file now...\n");

	receiveData(socketFD, message, 1);

	close(socketFD); // Close the socket
	return 0;
}

void receiveData(int socketFD, char *string, int flag)
{
	int charsReceived;
	char buffer[maxbuffer];
	memset(buffer, '\0', maxbuffer);
	memset(string, '\0', maxchars);
	int bufferLen = 0;
	int i = 0;

	while (1)
	{
		charsReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("ENC CLIENT: ERROR reading from socket");
		}
		// printf("current package is %s\n", buffer);

		if (i == 0)
		{
			sprintf(string, "%s", buffer);
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

	if (flag == 1)
	{
		printf("%s", string);
	}
}

// void receiveData(int socketFD, char *string, int flag)
// {
// 	int charsReceived;
// 	char buffer[maxchars];
// 	memset(buffer, '\0', maxchars);
// 	memset(string, '\0', maxchars);
// 	int bufferLen = 0;
// 	int i = 0;

// 	// Read the client's message from the socket
// 	charsReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

// 	if (charsReceived < 0)
// 	{
// 		error("ENC CLIENT: ERROR reading from socket");
// 	}

// 	sprintf(string, "%s", buffer);
// 	if (flag == 1)
// 	{
// 		printf("%s\n", buffer);
// 	}
// }

void sendData(int socketFD, char *string)
{
	// char buffer[maxchars];
	// memset(buffer, '\0', maxchars);
	int charsWritten;

	charsWritten = send(socketFD, string, strlen(string), 0);
	if (charsWritten < 0)
		error("ENC CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(string))
		error("ENC CLIENT: WARNING: Not all data written to socket!");
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
		error("ENC CLIENT: key is too short");
		return -1;
	}

	return 0;
}

void readFile(char filepath[], char *array, char *array2)
{
	int i = 0;
	int file_descriptor;
	char line[maxchars];
	ssize_t nread, nwritten;
	char readBuffer[maxchars];
	// int length = 0;

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

		// length = length + strlen(line);
		i++;
		// printf("Current line: %s\n", line);
	}

	fclose(file);

	// array = readBuffer;
	// printf("Final string is %s", readBuffer);
	// sprintf(array, "%s", readBuffer);
}

void checkString(char *string)
{
	int length = strlen(string) - 1;
	int i;
	char currentChar;

	for (i = 0; i < length; i++)
	{
		currentChar = string[i];

		if ((currentChar < 'A' || currentChar > 'Z') && currentChar != 32)
		{
			printf("Position %i Current char is %c with value %i\n", i, string[i], string[i]);
			// printf("Error found!");

			printf("ENC CLIENT: input contains bad characters\n");
		}
	}
}