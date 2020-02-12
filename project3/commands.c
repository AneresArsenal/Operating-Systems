#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>

// global variables
#define maxchars 100
char input[maxchars];
size_t indexOfNullTerminator;

int main(void)
{

	memset(input, '\0', maxchars);
	printf("Enter input");
	fflush(stdout);
	fgets(input, maxchars, stdin);
	indexOfNullTerminator = strlen(input);
	input[indexOfNullTerminator - 1] = '\0';
	char delim[] = ":";
	char *token;
	token = strtok(input, delim);
	printf("%s", token);
}
