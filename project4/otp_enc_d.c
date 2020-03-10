#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

// global variables
#define maxchars 80000
#define maxbuffer 63000
int PIDCount = 0;

// error text must be output to stderr
void error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void receiveData(int establishedConnectionFD, char *string);
void sendSuccessMessage(int establishedConnectionFD);
void sendData(int establishedConnectionFD, char *encryptedFile);
void encryptFile(char *file, char *key, char *encrypted);
int receiveHandshake(int establishedConnectionFD);
void forkNewProcess(socklen_t sizeOfClientInfo, int establishedConnectionFD, int listenSocketFD, struct sockaddr_in clientAddress, int portNumber);
int updatePIDCount(int flag);

void catchSIGCHILD(int signo)
{

	int exitStatus;
	int pid = waitpid(-1, &exitStatus, WNOHANG);
	// a child process terminated, update tally
	if (pid > 0)
	{
		updatePIDCount(-1);
	}

	// child process exited with exit status
	if (WIFEXITED(exitStatus))
	{
		int exitStat = WEXITSTATUS(exitStatus);
	}

	// child process has terminated with a signal
	else if (WIFSIGNALED(exitStatus))
	{
		int termSignal = WTERMSIG(exitStatus);
	}
}

int main(int argc, char *argv[])
{
	// signal handler to catch sigchld
	signal(SIGCHLD, catchSIGCHILD);

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
		error("ENC SERVER: ERROR opening socket");

	// allow socket reuse
	if (setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		error("ENC SERVER: setsockopt(SO_REUSEADDR) failed");

	// Enable the socket to begin listening
	// Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		error("ENC SERVER: ERROR on binding");
	}

	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Get the size of the address for the client that will connect
	sizeOfClientInfo = sizeof(clientAddress);
	pid_t spawnPid = -1;
	int childExitMethod = -1;

	// keep server open in while loop
	while (1)
	{

		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0)
		{
			error("ENC SERVER: ERROR on accept");
		}

		// no request sent to socket, skip the rest of the loop
		else if (establishedConnectionFD == 0)
		{
			close(establishedConnectionFD);
			continue;
		}

		// accept succeeded
		else
		{
			// check count
			updatePIDCount(0);

			// maxed out at 5, skip the rest of the loop
			if (PIDCount == 5)
			{
				continue;
			}

			else
			{
				// update count
				updatePIDCount(1);
				spawnPid = fork();

				switch (spawnPid)
				{

				// fork is unsuccsessful
				case -1:
				{
					error("ENC SERVER: Fork unsuccessful");
					break;
				}

				// fork is successful, child is running
				case 0:
				{
					forkNewProcess(sizeOfClientInfo, establishedConnectionFD, listenSocketFD, clientAddress, portNumber);
					break;
				}

				// handle child case
				default:
				{
					// block while loop until one of the child processes exit since count has maxed out
					if (PIDCount == 5)
					{
						// printf("ENC SERVER: Kinda busy right now...\n");
						waitpid(-1, &childExitMethod, 0);
					}

					// create child process to run without blocking
					else
					{
						waitpid(spawnPid, &childExitMethod, WNOHANG);
					}

					close(establishedConnectionFD);
				}
				}
			}
		}
		close(establishedConnectionFD);
	}

	close(listenSocketFD); // Close the listening socket

	return 0;
}

void forkNewProcess(socklen_t sizeOfClientInfo, int establishedConnectionFD, int listenSocketFD, struct sockaddr_in clientAddress, int portNumber)
{

	// perform handshake with client
	if (receiveHandshake(establishedConnectionFD) < 0)
	{
		fprintf(stderr, "ENC SERVER: ERROR handshake failed at port %i\n", portNumber);
		sendData(establishedConnectionFD, "handshake failed\n");
		exit(2);
	}

	sendData(establishedConnectionFD, "This is otp-enc-d\n");

	// printf("SERVER: Connected client at port %d\n", ntohs(clientAddress.sin_port));

	char filestring[maxchars];
	receiveData(establishedConnectionFD, filestring);

	// Send a Success message back to the client
	sendSuccessMessage(establishedConnectionFD);

	char keystring[maxchars];
	receiveData(establishedConnectionFD, keystring);

	// Send a Success message back to the client
	sendSuccessMessage(establishedConnectionFD);

	char dummy[maxchars];
	receiveData(establishedConnectionFD, dummy);

	char encrypted[maxchars];
	encryptFile(filestring, keystring, encrypted);

	sendData(establishedConnectionFD, encrypted);

	close(establishedConnectionFD); // Close the existing socket which is connected to the client
}

void receiveData(int establishedConnectionFD, char *string)
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
		charsReceived = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("ENC SERVER: ERROR reading from socket");
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

	// Read the client's message from the socket
	// Same logic as receive data above
	while (1)
	{
		charsReceived = recv(establishedConnectionFD, buffer, sizeof(maxbuffer) - 1, 0);

		if (charsReceived < 0)
		{
			error("ENC SERVER: ERROR reading from socket");
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

	// string compare to make sure it matches expected enc client key
	if (strcmp(string, "This is otp-enc\n") != 0)
	{
		// failed, return -1
		return -1;
	}

	return 0;
}

void sendSuccessMessage(int establishedConnectionFD)
{
	char buffer[maxchars];
	memset(buffer, '\0', maxchars);
	int charsWritten;

	// Send a Success message back to the client
	sprintf(buffer, "Success! I am the enc server, and I got your message\n");

	charsWritten = send(establishedConnectionFD, buffer, strlen(buffer), 0); // Send success back
	if (charsWritten < 0)
		error("ENC SERVER: ERROR writing to socket");

}

void sendData(int establishedConnectionFD, char *encryptedFile)
{
	int charsWritten;

	charsWritten = send(establishedConnectionFD, encryptedFile, strlen(encryptedFile), 0); // Send success back
	if (charsWritten < 0)
		error("ENC SERVER: ERROR writing to socket");

}

void encryptFile(char *file, char *key, char *encrypted)
{
	// perform encryption process
	int fileLength = strlen(file) - 1;
	int i;
	int total;
	int result;
	int fileChar;
	int keyChar;
	char currentChar;
	memset(encrypted, '\0', maxchars);

	// loop till file string length
	for (i = 0; i < fileLength; i++)
	{
		// if file char is not a space, minus 64
		if (file[i] != 32)
		{
			fileChar = file[i] - 64;
		}

		// else assign to 0
		else
		{
			fileChar = 0;
		}

		// if key char is not a space, minus 64
		if (key[i] != 32)
		{
			keyChar = key[i] - 64;
		}

		// else assign to 0
		else
		{
			keyChar = 0;
		}

		// add file and key numbers
		total = fileChar + keyChar;

		// if (total > 26)
		// {
		// 	total = total - 26;
		// }

		// get the remainder after modulus 26
		result = total % 26;

		// revert to capital letters
		result = result + 64;

		// 64 is space, change to 32
		if (result == 64)
		{
			currentChar = 32;
		}
		else if (result > 64 && result < 91)
		{
			currentChar = result;
		}

		// bad input detected
		else
		{
			error("ENC SERVER: Bad input");
		}

		encrypted[i] = currentChar;
		// printf("Char %i   ", i);
		// printf("message: %i   ", fileChar);
		// printf("key: %i   ", keyChar);
		// printf("message + key: %i   ", total);
		// printf("ciphertext: %i  \n", encrypted[i]);
	}

	encrypted[i] = '\n';
}

int updatePIDCount(int flag)
{

	// flag 0	 - check
	// flag 1	 - add
	// flag -1	 - minus
	if (flag == 0)
	{
		if (PIDCount == 5)
		{
			//maxed out
			// printf("Maxed out!\n");
			// return -1;
		}
		// return 0;
	}
	else if (flag == 1)
	{
		// printf("ENC SERVER: One child added!\n");
		PIDCount++;
	}

	else
	{
		// printf("ENC SERVER: One cild removed!\n");
		PIDCount--;
	}

	// printf("ENC SERVER: Current PID count is %i\n", PIDCount);

	return 0;
}

// references
// https://stackoverflow.com/questions/16007789/keep-socket-open-in-c
// http://man7.org/linux/man-pages/man2/accept.2.html
// https://linux.die.net/man/2/waitpid
// https://beej.us/guide/bgnet/html/#setsockoptman
// server and client files provided on canvas