
/****************************************** otp_enc_d ***************************************/
/** This program will run in the background as a daemon. 
 * output an error if it cannot be run due to a network error, such as the ports being unavailable.
 *  function is to perform the actual encoding
 * This program will listen on a particular port/socket, assigned when it is first ran.
 *  When a connection is made, otp_enc_d must call accept() to generate the socket used for actual communication, and then use a separate process to handle the rest of the transaction (see below), which will occur on the newly accepted socket.
 * child process of otp_enc_d must first check to make sure it is communicating with otp_enc.
 * After verifying that the connection to otp_enc_d is coming from otp_enc, then this child receives from otp_enc plaintext and a key via the communication socket (not the original listen socket).
 * The otp_enc_d child will then write back the ciphertext to the otp_enc process that it is connected to via the same communication socket. Note that the key passed in must be at least as big as the plaintext.
 *  must support up to five concurrent socket connections running at the same time
 * your system must be able to do five separate encryptions at once, using either method you choose.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define maxchars 63999

void error(const char *msg)
{
	perror(msg);
	exit(1);
} // Error function used for reporting issues

void receivedata(int establishedConnectionFD, char *string);

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

	printf("SERVER: Connected client at port %d\n", ntohs(clientAddress.sin_port));

	char filestring[maxchars];
	receivedata(establishedConnectionFD, filestring);

	// memset(buffer, '\0', maxchars);
	// charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Read the client's message from the socket
	// if (charsRead < 0)
	// {
	// 	error("ERROR reading from socket");
	// }
	// // printf("%s", buffer);
	// printf("SERVER: I received this from the client: %s", buffer);

	// Get the message from the client and display it
	// printf("SERVER: I received this from the client:\n");

	// Send a Success message back to the client
	memset(buffer, '\0', maxchars);
	sprintf(buffer, "Success! I am the server, and I got your message");

	charsWritten = send(establishedConnectionFD, buffer, strlen(buffer), 0); // Send success back
	if (charsWritten < 0)
		error("ERROR writing to socket");

	printf("\nSERVER: Waiting for next data package....\n\n");

	memset(buffer, '\0', maxchars);
	charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Read the client's message from the socket
	if (charsRead < 0)
	{
		error("ERROR reading from socket");
	}
	// printf("%s", buffer);
	printf("SERVER: I received this from the client: %s", buffer);

	printf("\nSERVER: Server shutting down...\n\n");

	close(establishedConnectionFD); // Close the existing socket which is connected to the client

	printf("\nSERVER: Socket connecting to client closed\n");

	close(listenSocketFD); // Close the listening socket

	printf("\nSERVER: Close listening socket\n");
	// }

	return 0;
}

void receivedata(int establishedConnectionFD, char *string)
{
	int charsReceived;
	char buffer[maxchars];
	memset(buffer, '\0', maxchars);

	// Read the client's message from the socket
	charsReceived = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);

	if (charsReceived < 0)
	{
		error("ERROR reading from socket");
	}
	// printf("%s", buffer);
	sprintf(string, "%s", buffer);
	printf("SERVER: I received this from the client: %s", buffer);
	printf("SERVER: String saved as: %s", string);
}

// reference
// https://stackoverflow.com/questions/13669474/multiclient-server-using-fork
// https://stackoverflow.com/questions/16007789/keep-socket-open-in-c