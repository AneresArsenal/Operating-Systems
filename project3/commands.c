// #include <sys/types.h>
// #include <sys/stat.h>
#include <unistd.h>
// #include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
// #include <time.h>
// #include <dirent.h>
// #include <pthread.h>

// global variables
#define maxchars 2048
#define maxargs 512
#define IDnums 10
pid_t parent;
pid_t ppids[100];
int pidCount = 0;
size_t indexOfNullTerminator;
char cwd[maxchars];
char *getcwd(char *buf, size_t size);
char inputs[maxargs + 2][maxchars];
char inputFile[maxargs][maxchars];
char outputFile[maxargs][maxchars];

// size_t indexOfNullTerminator;

int getUserInput(char *string);
int cleanInputs(int args);
void exitCommand();
void checkExitStatus();
void runCommand(char *string, int argCount);
char input[maxchars];

int main(void)
{

	char result[maxchars];
	int parent = (int)getpid();
	int childExitMethod;

	int argCount, i;

	argCount = getUserInput(input);
	if (argCount == -1)
	{
		return 0;
	}

	// printf("Pre-clean arg count: %i\n", argCount);
	argCount = cleanInputs(argCount);
	// printf("Post-clean arg count: %i\n", argCount);

	for (i = 0; i < argCount; i++)
	{
		printf("Input %i: %s\n", i, inputs[i]);
	}

	// if (strcmp(inputFile[0], "\0") != 0)
	// {
	// 	for (i = 0; i < 2; i++)
	// 	{
	// 		printf("Input file %i: %s\n", i, inputFile[i]);
	// 	}
	// }

	// if (strcmp(outputFile[0], "\0") != 0)
	// {
	// 	for (i = 0; i < 2; i++)
	// 	{
	// 		printf("Output file %i: %s\n", i, outputFile[i]);
	// 	}
	// }

	// runCommand(input, argCount);

	return 0;
}

int getUserInput(char *string)
{

	int i = 0;
	int j;
	int count = 0;

	memset(string, '\0', maxchars);
	printf(": ");
	fflush(stdout);
	fgets(string, maxchars, stdin);

	indexOfNullTerminator = strlen(string);
	string[indexOfNullTerminator - 1] = '\0';

	char delim[] = " ";
	char *token;

	// ignore blank or comment lines
	if (strlen(string) == 0 || (string[0] == '#'))
	{
		printf("Blank line or comment found!\n");
		return -1;
	}

	for (token = strtok(string, " "); token != NULL; token = strtok(NULL, " "))
	{
		if (i != 0)
		{

			sprintf(inputs[i - 1], "%s", token);
			// printf("i is %i: token is %s\n",i, token);
		}
		i++;

		puts(token);
	}

	for (j = 0; j <= i; j++)
	{
		if (strcmp(inputs[j], "") != 0)
		{
			// printf("Input %i: %s\n", j, inputs[j]);
			count++;
		}
	}
	return count;
}

int cleanInputs(int count)
{
	int args = count;
	int i;

	for (i = 0; i < count; i++)
	{

		// found input file arg
		if (strcmp(inputs[i], "<") == 0)
		{
			// copy into input file array
			sprintf(inputFile[0], "%s", inputs[i]);
			sprintf(inputFile[1], "%s", inputs[i + 1]);
			memset(inputs[i], '\0', maxchars);
			memset(inputs[i + 1], '\0', maxchars);
			args--;
			args--;
		}

		// found output file arg
		else if (strcmp(inputs[i], ">") == 0)
		{
			// copy into output file array
			sprintf(outputFile[0], "%s", inputs[i]);
			sprintf(outputFile[1], "%s", inputs[i + 1]);
			memset(inputs[i], '\0', maxchars);
			memset(inputs[i + 1], '\0', maxchars);
			args--;
			args--;
		}
	}

	return args;
}

void runCommand(char *string, int argCount)
{
	if (strcmp(string, "exit") == 0)
	{

		if (argCount == 0)
		{
			exitCommand();
		}

		else
		{
			printf("Error! No argument is allowed.\n");
			fflush(stdout);
		}
	}

	else if (strcmp(string, "cd") == 0)
	{
		char pathway[maxchars];
		memset(cwd, '\0', maxchars);
		getcwd(cwd, sizeof(cwd));
		// printf("Pre-call cwd: %s\n", cwd);

		if (argCount > 1)
		{
			printf("Error! Maximum allowed argument is 1.\n");
			fflush(stdout);
		}

		if (argCount == 0)
		{

			memset(pathway, '\0', maxchars);
			strcpy(pathway, getenv("HOME"));
		}

		else if (argCount == 1)
		{
			memset(pathway, '\0', maxchars);
			strcpy(pathway, inputs[1]);
		}

		chdir("..");
		chdir(pathway);
		getcwd(cwd, sizeof(cwd));
		// printf("Post-call cwd: %s\n", cwd);
	}

	else if (strcmp(string, "status") == 0)
	{
		if (argCount == 0)
		{
			pid_t lastPid = 0;

			if (lastPid != 0)
			{
				printf("pid is %i\n", lastPid);
				fflush(stdout);
				checkExitStatus(lastPid);
			}

			else
			{
				printf("No previous foreground command found.\n");
				fflush(stdout);
			}
		}

		else
		{
			printf("Error! No argument is allowed.\n");
			fflush(stdout);
		}
	}
	else
	{
		execlp("ls", "ls", inputs);
	}
}

void exitCommand()
{
	int i;
	// kill all child processes
	for (i = 0; i < pidCount; i++)
	{
		kill(ppids[i], SIGKILL);
	}

	// kill parent
	kill(parent, SIGKILL);
}

void checkExitStatus(pid_t pid)
{
	int childExitMethod;
	pid = wait(&childExitMethod);
	// if (pid == -1)
	// {
	// 	perror("wait failed");
	// 	exit(1);
	// }

	// child process has terminated normally
	if (WIFEXITED(childExitMethod))
	{
		printf("The process exited normally\n");
		fflush(stdout);
		int exitStatus = WEXITSTATUS(childExitMethod);
		printf("exit status was %d\n", exitStatus);
		fflush(stdout);
	}
	else
	{
		//If the child process was terminated by a signal
		int termSignal = WTERMSIG(childExitMethod);
		printf("Child terminated by signal > %i\n", termSignal);
		fflush(stdout);
	}
}

// https://stackoverflow.com/questions/13273836/how-to-kill-child-of-fork
// https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
// https://stackoverflow.com/questions/39716380/my-cd-function-does-not-go-to-the-home-directory
// http://www0.cs.ucl.ac.uk/staff/ucacbbl/getenv/
// https://stackoverflow.com/questions/7630551/using-a-new-path-with-execve-to-run-ls-command
// https://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits
// https://stackoverflow.com/questions/1908610/how-to-get-pid-of-background-process
// https://www.tldp.org/LDP/abs/html/x9644.html