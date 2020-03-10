
/****************************************** KeyGen ***************************************/
// Program requirements:
// This program creates a key file of specified length. The characters in the file generated 
// will be any of the 27 allowed characters, generated using the standard UNIX randomization methods.
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
#define maxchars 63999

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
	srand(time(NULL));

	int length = atoi(argv[1]);

	char string[maxchars];
	char string2[maxchars];

	char alpha;

	// reset char arrays
	memset(string, '\0', maxchars);
	memset(string2, '\0', maxchars);

	// if specified length is over 63999, split generation into two strings

	if (length >= 63999)
	{
		for (i = 0; i < length; i++)
		{

			// flag used to randomly place space
			flag = rand() % (10);
		
			// if flag is below 9, generate alphabet
			if (flag <= 8)
			{
				alpha = rand() % (26) + 65;
			}

			// else place a space
			else
			{
				alpha = 32;
			}

			if (i < 63999)
			{
				string[i] = alpha;
			}

			else
			{
				string2[i - 63999] = alpha;
			}
			
		}

		// last character keygen outputs should be a newline
		string2[length-63999] = '\n';
		printf("%s%s", string, string2);
	}


	// same randomization as above, just with one string as length is below 63999
	else
	{
		for (i = 0; i < length; i++)
		{

			flag = rand() % (10);
			
			if (flag <= 8)
			{
				alpha = rand() % (26) + 65;
			}

			else
			{
				alpha = 32;
			}
			string[i] = alpha;
		}
	

		// last character keygen outputs should be a newline
		string[length] = '\n';
		printf("%s", string);
	}

	return 0;
}

// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
// https://stackoverflow.com/questions/17909215/c-random-numbers-between-10-and-30