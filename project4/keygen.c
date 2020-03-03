
/****************************************** KeyGen ***************************************/
// This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods.
// Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation! rand() is just fine.
// The last character keygen outputs should be a newline. All error text must be output to stderr, if any.

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <time.h>

// global variables
#define maxchars 255
struct keyValue
{
	int key;
	char alphabet;
};

struct keyMap
{
	struct keyValue keyValues[27];
};

char **generatekey();
struct keyMap mapKey();
void printMap(struct keyMap map);
void printKey(struct keyMap map, int length);
int checkArr(char array[], char alphabet);
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int i, flag;
	struct keyMap map;
	srand(time(NULL));

	int length = atoi(argv[1]);

	char string[length + 1];

	char alpha;

	memset(string, '\0', length);

	// printf("\n Key length is %d\n", length);
	for (i = 0; i < length; i++)
	{

		flag = rand() % (10);
		// printf("Flag is %i\n", flag);
		if (flag <= 8)
		{
			alpha = rand() % (26) + 65;
		}

		else
		{
			alpha = 32;
		}
		string[i] = alpha;
		// printf("Char added %c\n", alpha);
		// printf("Key %i: char %c code is %i\n", i, alpha, alpha);
	}
	// fflush(stdout);
	// last character keygen outputs should be a newline
	string[length] = '\n';

	// no output file provided, outputs to stdout
	if (argc == 2)
	{
		// printKey(map, length);
		printf("%s", string);
	}

	// output file provided, write to file
	// else if (argc == 4)
	// {
	// 	int file_descriptor;
	// 	ssize_t nread, nwritten;
	// 	char readBuffer[256];

	// 	// attempt to open file
	// 	file_descriptor = open(argv[3], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	// 	// throw error if unsuccessful
	// 	if (file_descriptor == -1)
	// 	{
	// 		error("Hull breach - open() failed \n");
	// 		exit(1);
	// 	}

	// 	nwritten = write(file_descriptor, string, strlen(string) * sizeof(char));
	// 	// clear buffer before reading the file
	// 	memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
	// 	lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the beginning of the file
	// 	nread = read(file_descriptor, readBuffer, sizeof(readBuffer));
	// 	printf("File contents:\n%s\n", readBuffer);
	// }

	return 0;
}

// struct keyMap mapKey()
// {

// 	char keys[27];
// 	memset(keys, '\0', 27);
// 	int alphabet;
// 	int flag = -1;

// 	struct keyMap map;
// 	int i;

// 	for (i = 0; i < 27; i++)
// 	{
// 		map.keyValues[i].key = i;

// 		if (i == 26)
// 		{
// 			alphabet = 32;
// 			map.keyValues[i].alphabet = 32;
// 			// printf("Key %i: char %c code is %i \n", i, alphabet, alphabet);
// 			break;
// 		}

// 		while (flag == -1)
// 		{

// 			alphabet = rand() % (26) + 65;
// 			flag = checkArr(keys, alphabet);
// 		}

// 		map.keyValues[i].alphabet = alphabet;
// 		keys[i] = alphabet;
// 		// printf("Key %i: char %c code is %i \n", i, alphabet, alphabet);
// 		flag = -1;
// 	}

// 	return map;
// }

// int checkArr(char array[], char alphabet)
// {
// 	int i;

// 	for (i = 0; i < 27; i++)
// 	{
// 		if (alphabet == array[i])
// 		{
// 			return -1;
// 		}
// 	}

// 	return 0;
// }

// void printMap(struct keyMap map)
// {
// 	int i;

// 	for (i = 0; i < 27; i++)
// 	{
// 		// printf("Key %i, Char %c \n", map.keyValues[i].key, map.keyValues[i].alphabet);
// 	}
// }

// void printKey(struct keyMap map, int length)
// {
// 	int i;

// 	for (i = 0; i < 27; i++)
// 	{
// 	printf("Key %i, Char %c \n", map.keyValues[i].key, map.keyValues[i].alphabet);
// 	}
// }

// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
// https://stackoverflow.com/questions/17909215/c-random-numbers-between-10-and-30