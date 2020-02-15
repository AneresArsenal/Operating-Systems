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
struct userInput
{
	char command[maxchars];
	char arguments[maxargs][maxchars];
	int inputFlag;
	char input[3][maxchars];
	int outputFlag;
	char output[3][maxchars];
	int backgroundFlag;
};

// size_t indexOfNullTerminator;

int getUserInput(struct userInput *currentInput);
void checkStruct(struct userInput *currentInput);
void exitCommand();
void checkExitStatus();
void runCommand(struct userInput *currentInput, int argCount);
char input[maxchars];

int main(void)
{

	// char result[maxchars];
	// int parent = (int)getpid();
	// int childExitMethod;

	int argCount, i;
	struct userInput *currentInput;
	currentInput = calloc(1, sizeof(struct userInput));

	argCount = getUserInput(currentInput);

	if (argCount == -1)
	{
		return 0;
	}

	// checkStruct(currentInput);

	runCommand(currentInput, argCount);

	return 0;
}

int getUserInput(struct userInput *currentInput)
{

	int i = 0;
	int j;
	int count = 0;
	char string[maxchars];
	char inputs[maxargs][maxchars];

	memset(string, '\0', maxchars);
	printf(": ");
	// fflush(stdout);
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
		if (i == 0)
		{
			strcpy(currentInput->command, token);
		}

		else
		{
			memset(inputs[i - 1], '\0', maxchars);
			sprintf(inputs[i - 1], "%s", token);
		}
		i++;
	}

	count = i - 1;

	// for (i = 0; i < count; i++)
	// {
	// 	printf("Input %i: %s\n", i, inputs[i]);
	// }

	i = 0;
	while ((strcmp(inputs[i], "<") != 0) && (strcmp(inputs[i], ">") != 0) && (strlen(inputs[i]) != 0))
	{
		strcpy(currentInput->arguments[i], inputs[i]);
		// printf("Argument %i: %s\n", i, currentInput->arguments[i]);
		i++;
	}

	sprintf(currentInput->arguments[i + 1], NULL);

	// printf("i is %i\n",i);
	// printf("count is %i\n",count);

	if (i < count)
	{

		for (j = i; j < count; j++)
		{
			// printf("current position is %i\n",j);

			if (strcmp(inputs[j], "<") == 0)
			{
				// printf("Found input! \n");
				currentInput->inputFlag = 1;
				// copy into input file array
				strcpy(currentInput->input[0], inputs[j]);
				strcpy(currentInput->input[1], inputs[j + 1]);
			}

			// found output file arg
			if (strcmp(inputs[j], ">") == 0)
			{
				// printf("Found output! \n");
				currentInput->outputFlag = 1;
				// copy into output file array
				strcpy(currentInput->output[0], inputs[j]);
				strcpy(currentInput->output[1], inputs[j + 1]);
			}
		}
	}

	if (currentInput->inputFlag == 1)
	{
		count = count - 2;
	}

	if (currentInput->outputFlag == 1)
	{
		count = count - 2;
	}

	return count;
}

void checkStruct(struct userInput *currentInput)
{

	printf("Command is: %s\n", currentInput->command);

	int i = 0;

	while (strlen(currentInput->arguments[i]) != 0)
	{
		printf("Argument %i: %s\n", i, currentInput->arguments[i]);
		i++;
	}

	if (currentInput->inputFlag == 1)
	{
		for (i = 0; i < 2; i++)
		{
			printf("Input array item %i: %s\n", i, currentInput->input[i]);
		}
	}

	if (currentInput->outputFlag == 1)
	{
		for (i = 0; i < 2; i++)
		{
			printf("Output array item %i: %s\n", i, currentInput->output[i]);
		}
	}
}

void runCommand(struct userInput *currentInput, int argCount)
{
	if (strcmp(currentInput->command, "exit") == 0)
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

	else if (strcmp(currentInput->command, "cd") == 0)
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
			strcpy(pathway, currentInput->input[0]);
		}

		chdir("..");
		chdir(pathway);
		getcwd(cwd, sizeof(cwd));
		// printf("Post-call cwd: %s\n", cwd);
	}

	else if (strcmp(currentInput->command, "status") == 0)
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
		int size = argCount;
		if (currentInput->inputFlag == 1)
		{
			size = size + 2;
		}
		if (currentInput->outputFlag == 1)
		{
			size = size + 2;
		}

		// no args
		if (size == 0)
		{
			execlp(currentInput->command, currentInput->command, NULL);
		}

		// only output and/or input
		else if (argCount == 0 && size > 0)
		{
			// printf("no arg count but flag detected \n.");
			char args[size + 1][maxchars];
			int j = 0;

			if (currentInput->inputFlag == 1)
			{
				memset(args[j], '\0', maxchars);
				memset(args[j + 1], '\0', maxchars);

				strcpy(args[j], currentInput->input[0]);
				strcpy(args[j + 1], currentInput->input[1]);
				j = j + 2;
			}

			if (currentInput->outputFlag == 1)
			{
				memset(args[j], '\0', maxchars);
				memset(args[j + 1], '\0', maxchars);

				strcpy(args[j], currentInput->output[0]);
				strcpy(args[j + 1], currentInput->output[1]);
				j = j + 2;
			}

			memset(args[j], '\0', maxchars);
			sprintf(args[j], NULL);
			execlp(currentInput->command, currentInput->command, args);
		}

		else
		{
			// printf("Size is %i \n", size);
			// printf("Argcount is %i \n", argCount);

			char args[size + 1][maxchars];
			int j = 0;

			for (j = 0; j < argCount; j++)
			{
				memset(args[j], '\0', maxchars);
				strcpy(args[j], currentInput->arguments[j]);
				j++;
			}

			// checkStruct(currentInput);
			sprintf(args[j], NULL);
			execlp(currentInput->command, currentInput->command, args);

			// if (currentInput->inputFlag == 1)
			// {

			// 	memset(args[j], '\0', maxchars);
			// 	memset(args[j + 1], '\0', maxchars);

			// 	strcpy(args[j], currentInput->input[0]);
			// 	strcpy(args[j + 1], currentInput->input[1]);
			// 	j = j + 2;
			// }

			// if (currentInput->outputFlag == 1)
			// {
			// 	memset(args[j], '\0', maxchars);
			// 	memset(args[j + 1], '\0', maxchars);

			// 	strcpy(args[j], currentInput->output[0]);
			// 	strcpy(args[j + 1], currentInput->output[1]);
			// 	j = j + 2;
			// }

			// memset(args[j], '\0', maxchars);
			// sprintf(args[j], NULL);
			// execlp(currentInput->command, currentInput->command, args);
		}
	}
	// 	int size = argCount + 1;
	// 	if (currentInput->inputFlag == 1)
	// 	{
	// 		size = size + 2;
	// 	}
	// 	if (currentInput->outputFlag == 1)
	// 	{
	// 		size = size + 2;
	// 	}

	// 	char args[size][maxchars];
	// 	int i, j = 0;

	// 	for (i = 0; i < argCount; i++)
	// 	{
	// 		memset(args[i], '\0', maxchars);
	// 		strcpy(args[i], currentInput->arguments[i]);
	// 		j++;
	// 	}

	// 	if (currentInput->inputFlag == 1)
	// 	{

	// 		strcpy(args[j], currentInput->input[0]);
	// 		strcpy(args[j + 1], currentInput->input[1]);
	// 		j = j + 2;
	// 	}

	// 	if (currentInput->outputFlag == 1)
	// 	{

	// 		strcpy(args[j], currentInput->output[0]);
	// 		strcpy(args[j + 1], currentInput->output[1]);
	// 		j = j + 2;
	// 	}

	// 	sprintf(args[j], NULL);

	// 	execlp(currentInput->command, currentInput->command, args);
	// }
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