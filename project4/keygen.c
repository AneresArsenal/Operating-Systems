
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
int checkArr(char array[], char alphabet);

int main(int argc, char *argv[])
{
	int counter;
	struct keyMap map;

	// printf("Program Name Is: %s", argv[0]);
	// if (argc == 1)
	// 	printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
	// if (argc >= 2)
	// {
	// 	printf("\nNumber Of Arguments Passed: %d", argc);
	// 	printf("\n----Following Are The Command Line Arguments Passed----");
	// 	for (counter = 0; counter < argc; counter++)
	// 		printf("\nargv[%d]: %s\n", counter, argv[counter]);
	// }

	int length = atoi(argv[1]);
	// printf("\n Key length is %d\n", length);

	map = mapKey();
	// printMap(map);

	// no output file provided, outputs to stdout
	if (argc == 2)
	{
	}

	// output file provided, write to file
	else if (argc == 3)
	{
	}

	return 0;
}

struct keyMap mapKey()
{

	char keys[27];
	memset(keys, '\0', 27);
	int alphabet;
	int flag = -1;

	struct keyMap map;
	int i;

	for (i = 0; i < 27; i++)
	{
		map.keyValues[i].key = i;

		if (i == 26)
		{
			alphabet = 32;
			map.keyValues[i].alphabet = 32;
			// printf("Key %i: char %c code is %i \n", i, alphabet, alphabet);
			break;
		}

		while (flag == -1)
		{

			alphabet = rand() % (26) + 65;
			flag = checkArr(keys, alphabet);
		}

		map.keyValues[i].alphabet = alphabet;
		keys[i] = alphabet;
		// printf("Key %i: char %c code is %i \n", i, alphabet, alphabet);
		flag = -1;
	}

	return map;
}

int checkArr(char array[], char alphabet)
{
	int i;

	for (i = 0; i < 27; i++)
	{
		if (alphabet == array[i])
		{
			return -1;
		}
	}

	return 0;
}

void printMap(struct keyMap map)
{
	int i;

	for (i = 0; i < 27; i++)
	{
		// printf("Key %i, Char %c \n", map.keyValues[i].key, map.keyValues[i].alphabet);
	}
}

// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
// https://stackoverflow.com/questions/17909215/c-random-numbers-between-10-and-30