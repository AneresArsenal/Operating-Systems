// #include <sys/types.h>
// #include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
// #include "child.c"

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
	int argCount;
	int inputFlag;
	char input[1][maxchars];
	int outputFlag;
	char output[1][maxchars];
	int backgroundFlag;
};

// size_t indexOfNullTerminator;

int getUserInput(struct userInput *currentInput);
void checkStruct(struct userInput *currentInput);
void exitCommand();
void checkExitStatus();
void runCommand(struct userInput *currentInput, int argCount);
pid_t forkChild(struct userInput *currentInput);
void checkPIDs();
void runNonBuilt(struct userInput *currentInput);
char input[maxchars];

int main(void)
{

	char result[maxchars];
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

	// checkPIDs();

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
		fflush(stdout);
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

	i = 0;
	while ((strcmp(inputs[i], "<") != 0) && (strcmp(inputs[i], ">") != 0) && (strlen(inputs[i]) != 0))
	{
		strcpy(currentInput->arguments[i], inputs[i]);

		i++;
	}

	sprintf(currentInput->arguments[i + 1], NULL);

	// printf("i is %i\n",i);
	// printf("count is %i\n",count);

	if (i < count)
	{

		for (j = i; j < count; j++)
		{

			if (strcmp(inputs[j], "<") == 0)
			{
				currentInput->inputFlag = 1;
				// copy into input file array
				strcpy(currentInput->input[0], inputs[j + 1]);
			}

			// found output file arg
			if (strcmp(inputs[j], ">") == 0)
			{
				currentInput->outputFlag = 1;
				// copy into output file array
				strcpy(currentInput->output[0], inputs[j + 1]);
			}

			if (strcmp(inputs[j], "&") == 0)
			{
				currentInput->backgroundFlag = 1;
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

	if (currentInput->outputFlag == 1)
	{
		count--;
	}

	currentInput->argCount = count;

	return count;
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
		printf("Pre-call cwd: %s\n", cwd);

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
		printf("Post-call cwd: %s\n", cwd);
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
		runNonBuilt(currentInput);
		//forkChild(currentInput);
	}
}

pid_t forkChild(struct userInput *currentInput)
{
	pid_t spawnPid = -5;
	int childExitMethod = -5;
	int childExitStatus = -5;
	spawnPid = fork();

	switch (spawnPid)
	{
	case -1:
	{
		perror("Hull Breach!\n");
		exit(1);
		break;
	}
	case 0:
	{
		// printf("CHILD(%d): Sleeping for 1 second\n", getpid());
		// sleep(1);
		// printf("CHILD(%d): Converting into \'ls -a\'\n", getpid());
		// execlp("ls", "ls", "-a", NULL);
		runNonBuilt(currentInput);
		perror("CHILD: exec failure!\n");
		exit(2);
		break;
	}
	default:
	{
		// printf("PARENT(%d): Sleeping for 2 seconds\n", getpid());
		// sleep(2);
		// printf("PARENT(%d): Wait()ing for child(%d) to terminate\n", getpid(), spawnPid);
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
		// printf("PARENT(%d): Child(%d) terminated, Exiting!\n", getpid(), actualPid);
		parent = getpid();
		ppids[pidCount] = spawnPid;
		pidCount++;
		return spawnPid;
		// exit(0);
		// break;
	}
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

void checkStruct(struct userInput *currentInput)
{

	printf("Command is: %s\n", currentInput->command);

	int i = 0;

	printf("Arg count is: %i\n", currentInput->argCount);
	while (i < currentInput->argCount)
	{
		printf("Argument %i: %s\n", i, currentInput->arguments[i]);
		i++;
	}

	if (currentInput->inputFlag == 1)
	{
		printf("Input array item: %s\n", currentInput->input[0]);
	}

	if (currentInput->outputFlag == 1)
	{
		printf("Output array item: %s\n", currentInput->output[0]);
	}
}

void runNonBuilt(struct userInput *currentInput)
{
	int size = currentInput->argCount;
	int sourceFD, targetFD, result;
	// printf("in non-Built!\n");
	// checkStruct(currentInput);

	// no args
	if (size == 0)
	{
		execvp(currentInput->command, NULL);
	}

	// only output and/or input
	else if (currentInput->argCount == 0)
	{
		// printf("no arg count but flag detected \n.");

		if (currentInput->inputFlag == 1)
		{
			sourceFD = open(currentInput->input[0], O_RDONLY);
			if (sourceFD == -1)
			{
				perror("source open()");
				exit(1);
			}
			printf("sourceFD == %d\n", sourceFD); // Written to terminal

			result = dup2(sourceFD, 0);
			if (result == -1)
			{
				perror("source dup2()");
				exit(2);
			}
		}

		if (currentInput->outputFlag == 1)
		{
			targetFD = open(currentInput->output[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1)
			{
				perror("source open()");
				exit(1);
			}
			printf("targetFD == %d\n", targetFD); // Written to terminal

			result = dup2(targetFD, 1);
			if (result == -1)
			{
				perror("target dup2()");
				exit(2);
			}
		}

		execvp(currentInput->command, NULL);
	}

	// contains arguments and flags
	else
	{
		if (currentInput->inputFlag == 1)
		{
			printf("Input flag found \n");
			sourceFD = open(currentInput->input[0], O_RDONLY);
			if (sourceFD == -1)
			{
				perror("source open()");
				exit(1);
			}
			printf("sourceFD == %d\n", sourceFD); // Written to terminal

			result = dup2(sourceFD, 0);
			if (result == -1)
			{
				perror("source dup2()");
				exit(2);
			}
		}

		if (currentInput->outputFlag == 1)
		{
			printf("Output flag found \n");
			targetFD = open(currentInput->output[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (targetFD == -1)
			{
				perror("source open()");
				exit(1);
			}
			printf("targetFD == %d\n", targetFD); // Written to terminal

			result = dup2(targetFD, 1);
			if (result == -1)
			{
				perror("target dup2()");
				exit(2);
			}
		}

		// char (*arguments)[maxargs][maxchars] = &currentInput->arguments;

		char *argv[currentInput->argCount + 1];
		int i;
		for (i = 0; i < currentInput->argCount; i++)
		{
			memcpy(argv[i], currentInput->arguments[i], strlen(currentInput->arguments[i]));
		}
		memset(argv[currentInput->argCount], '\0', sizeof(char));

		execvp(currentInput->command, argv);
	}
}

void checkPIDs()
{
	int i;

	printf("Current parent is %d\n", parent);

	for (i = 0; i < pidCount; i++)
	{
		printf("Child %i is %d\n", i + 1, ppids[i]);
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