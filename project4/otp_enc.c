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

// error text must be output to stderr
void error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void receiveData(int socketFD, char *string, int flag);
void sendData(int socketFD, char *string);
int checkLength(char file[], char key[]);
void readFile(char filepath[], char *array);
void checkString(char *string);
int receiveHandshake(int socketFD);

int main(int argc, char *argv[])
{

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent *serverHostInfo;
	char buffer[maxchars];

	// wrong argument inputs provided
	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
		exit(1);
	}

	// check file length is shorter than key file provided, exit 1 if shorter
	if (checkLength(argv[1], argv[2]) == -1)
	{
		exit(1);
	}

	// initialize file and key string, reset char arrays to remove prior inputs
	char filestring[maxchars], keystring[maxchars];
	memset(filestring, '\0', sizeof(filestring));
	memset(keystring, '\0', sizeof(keystring));

	// read file and key into strings
	readFile(argv[1], filestring);
	checkString(filestring);
	readFile(argv[2], keystring);

	// Set up the server address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]);									 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;							 // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);					 // Store the port number
	serverHostInfo = gethostbyname("localhost");				 // Convert the machine name into a special form of address

	if (serverHostInfo == NULL)
	{
		error("ENC CLIENT: ERROR, no such host");
	}
	memcpy((char *)&serverAddress.sin_addr.s_addr, (char *)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0)
		error("ENC CLIENT: ERROR opening socket");

	// if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	// 	error("setsockopt(SO_REUSEADDR) failed");

	// Connect to server
	if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("ENC CLIENT: ERROR connecting");

	// perform handshake with server
	sendData(socketFD, "This is otp-enc\n");

	if (receiveHandshake(socketFD) < 0)
	{
		fprintf(stderr, "ENC CLIENT: ERROR handshake failed at port %i\n", portNumber);
		exit(2);
	}

	// send file string to server
	sendData(socketFD, filestring);

	// Get return message from server
	char message[maxchars];
	receiveData(socketFD, message, 0);

	// send key string to server
	sendData(socketFD, keystring);

	// Get return message from server
	receiveData(socketFD, message, 0);

	sendData(socketFD, "Waiting for encrypted file now...\n");

	// receive encrypted file
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

	// loop through until null terminator detected which breaks the while loop
	while (1)
	{
		charsReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("ENC CLIENT: ERROR reading from socket");
		}

		// first read, copy buffer to string
		if (i == 0)
		{
			sprintf(string, "%s", buffer);
		}

		// subsequent read, concatenate string
		else
		{
			strcat(string, buffer);
		}

		// get buffer length for null terminator check
		bufferLen = strlen(buffer);

		// break loop when null terminator found
		if ((buffer[bufferLen - 1]) == '\n')
		{
			break;
		}

		// reset buffer after each read
		memset(buffer, '\0', maxbuffer);
		i++;
	}

	// if flag is 1, printf to output to file, 0 to ignore
	if (flag == 1)
	{
		printf("%s", string);
	}
}

int receiveHandshake(int establishedConnectionFD)
{
	int charsReceived;
	char buffer[maxbuffer];
	char string[maxbuffer];
	memset(buffer, '\0', maxbuffer);
	memset(string, '\0', maxbuffer);
	int bufferLen = 0;
	int i = 0;

	// Read the server's message from the socket
	// Same logic as receive data above
	while (1)
	{

		charsReceived = recv(establishedConnectionFD, buffer, sizeof(maxbuffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("ENC CLIENT: ERROR reading from socket");
		}

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
			break;
		}
		memset(buffer, '\0', maxbuffer);
		i++;
	}

	// string compare to make sure it matches expected enc server key
	if (strcmp(string, "This is otp-enc-d\n") != 0)
	{
		// failed, return -1
		return -1;
	}

	return 0;
}

void sendData(int socketFD, char *string)
{
	int charsWritten;

	// send string provided to server
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

	file_descriptor = open(filepath, O_RDONLY);

	if (file_descriptor == -1)
	{
		fprintf(stderr, "ENC CLIENT: Hull breach - open() failed on \"%s\"\n", filepath);
		exit(1);
	}

	lseek(file_descriptor, 0, SEEK_SET);
	file = fopen(filepath, "r");

	// get wc of file
	while (fgets(line, sizeof(line), file))
	{
		countFile += strlen(line);
	}

	fclose(file);

	key_descriptor = open(key, O_RDONLY);

	if (key_descriptor == -1)
	{
		fprintf(stderr, "ENC CLIENT: - open() failed on \"%s\"\n", key);
		exit(1);
	}

	lseek(key_descriptor, 0, SEEK_SET);
	keyfile = fopen(key, "r");

	// get wc of key
	while (fgets(line, sizeof(line), keyfile))
	{
		countKey += strlen(line);
	}

	fclose(keyfile);

	// check file length to make sure it's shorter or equal to key length
	if (countFile <= countKey)
	{
		return 0;
	}
	else
	{
		error("ENC CLIENT: key is too short");
		return -1;
	}

	return 0;
}

void readFile(char filepath[], char array[maxchars])
{
	// read file using filepath provided in argument
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
		fprintf(stderr, "ENC CLIENT: - open() failed on \"%s\"\n", filepath);
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
	}

	fclose(file);
}

void checkString(char *string)
{
	int length = strlen(string) - 1;
	int i;
	char currentChar;

	// throw error if string is empty
	if (strlen(string) == 0)
	{
		error("ENC CLIENT: bad input");
	}

	// check if string is valid with appropriate input (capital letters and space allowed only)
	else
	{
		for (i = 0; i < length; i++)
		{
			currentChar = string[i];

			if ((currentChar < 'A' || currentChar > 'Z') && currentChar != 32)
			{
				error("ENC CLIENT: input contains bad characters");
			}
		}
	}
}
