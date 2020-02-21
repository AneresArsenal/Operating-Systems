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
pid_t lastFG = -99;
int lastFGExitStatus = 0;
pid_t bgPIDs[100];
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
	char *arguments[maxargs];
	int argCount;
	int inputFlag;
	char input[1][maxchars];
	int outputFlag;
	char output[1][maxchars];
	int backgroundFlag;
};
int fgModeFlag = 0; //disabled at first

char *checkExpansion(char *string);

// size_t indexOfNullTerminator;

int getUserInput(struct userInput *currentInput);
struct userInput *resetStruct(struct userInput *currentInput);
void checkStruct(struct userInput *currentInput);
void exitCommand();
void checkExitStatus(int exitstatus);
void CATCHsigchild(int signal);
void removePID(pid_t pid);
void runCommand(struct userInput *currentInput, int argCount);
pid_t forkChild(struct userInput *currentInput);

void checkPIDs();
void runNonBuilt(struct userInput *currentInput);
void ioRedirect(int inputOrOutput, char *filePath, int flag, mode_t mode);
char input[maxchars];

/**********************************************************************************/
/********************************* sigchld ****************************************/
/**********************************************************************************/

void CATCHsigchild(int signal)
{
	int bgChildExitMethod = -5;
	pid_t statusID = waitpid(-1, &bgChildExitMethod, WNOHANG);

	if (statusID > 0 && WIFEXITED(bgChildExitMethod))
	{
		removePID(statusID);
	}

	// write(STDOUT_FILENO, "\n: ", 2);
}

void removePID(pid_t pid)
{
	int i, j;

	printf("Background pid %d is completed \n", pid);

	for (i = 0; i < pidCount; i++)
	{
		if (bgPIDs[i] == pid)
		{
			break;
		}
	}

	for (j = i; j < pidCount; j++)
	{
		if (j + 1 < pidCount)
		{
			bgPIDs[j] = bgPIDs[j + 1];
		}
	}

	bgPIDs[pidCount - 1] = 0;
	pidCount--;
}

/**********************************************************************************/
/********************************* sigint ****************************************/
/**********************************************************************************/

void CATCHsigint(int signal)
{
	int fgChildExitMethod = -5;
	if (lastFG != -99)
	{
		waitpid(lastFG, &lastFGExitStatus, 0);
	}
	else
	{
		write(STDOUT_FILENO, ": ", 2);
	}
}

/**********************************************************************************/
/********************************* sigtstp ****************************************/
/**********************************************************************************/

void CATCHsigtstp(int signal)
{
	int fgChildExitMethod = -5;
	if (lastFG != -99)
	{
		waitpid(lastFG, &lastFGExitStatus, 0);
	}

	if (fgModeFlag == 0)
	{
		fgModeFlag = 1;
		write(STDOUT_FILENO, "Entering foreground-only mode.\n", 31);
	}

	else
	{
		fgModeFlag = 0;
		write(STDOUT_FILENO, "Exiting foreground-only mode.\n", 31);
	}

	write(STDOUT_FILENO, "\n: ", 3);
	// if (lastFG == -99)
	// {
	// 	write(STDOUT_FILENO, ": ", 2);
	// }
}

/**********************************************************************************/
/*********************************** Main *****************************************/
/**********************************************************************************/

int main(void)
{
	struct sigaction SIGCHLD_action = {{0}};
	struct sigaction SIGINT_action = {{0}};
	struct sigaction SIGTSTP_action = {{0}};

	SIGCHLD_action.sa_handler = CATCHsigchild;
	sigfillset(&SIGCHLD_action.sa_mask);
	SIGCHLD_action.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	SIGINT_action.sa_handler = CATCHsigint;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGINT, &SIGINT_action, NULL);

	SIGTSTP_action.sa_handler = CATCHsigtstp;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	char result[maxchars];
	int argCount, i;
	struct userInput *currentInput;
	currentInput = calloc(1, sizeof(struct userInput));

	while (argCount != -100)
	{

		// printf("Before user input:\n");
		// checkStruct(currentInput);
		// fflush(stdout);
		argCount = getUserInput(currentInput);

		// printf("After user input:\n");
		// fflush(stdout);
		// checkStruct(currentInput);

		// ignore comment blocks
		if (argCount != -1)
		{
			runCommand(currentInput, argCount);
		}

		else
		{
			currentInput = resetStruct(currentInput);
			continue;
		}

		// printf("After command has ran:\n");
		// fflush(stdout);
		// checkStruct(currentInput);
		resetStruct(currentInput);
	}

	// checkPIDs();

	return 0;
}

/**********************************************************************************/
/******************************** getUserInput ************************************/
/**********************************************************************************/

int getUserInput(struct userInput *currentInput)
{

	int i = 0;
	int j;
	int k;
	int count = 0;
	char string[maxchars];
	char inputs[maxargs][maxchars];

	memset(string, '\0', maxchars);
	for (k = 0; k < maxargs; k++)
	{
		memset(inputs[k], '\0', maxchars);
	}

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
		// printf("Blank line or comment found!\n");
		// fflush(stdout);
		return -1;
	}

	for (token = strtok(string, " "); token != NULL; token = strtok(NULL, " "))
	{
		if (i == 0) //store command
		{
			strcpy(currentInput->command, token);
		}

		memset(inputs[i], '\0', maxchars);
		sprintf(inputs[i], "%s", token);

		i++;
	}

	count = i;

	i = 0;
	while ((strcmp(inputs[i], "&") != 0) && (strcmp(inputs[i], "<") != 0) && (strcmp(inputs[i], ">") != 0) && (strlen(inputs[i]) != 0))
	{
		char *finalStr = checkExpansion(inputs[i]);

		currentInput->arguments[i] = finalStr;

		i++;
	}

	currentInput->arguments[i] = NULL;

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

	if (currentInput->backgroundFlag == 1)
	{
		count--;
	}

	currentInput->argCount = count;

	return count;
}

/**********************************************************************************/
/********************************* checkExpansion *********************************/
/**********************************************************************************/

char *checkExpansion(char *string)
{

	// printf("Pre-expansion string: %s\n", string);
	int pidNum = (int)getpid();
	char pidStr[maxchars];
	memset(pidStr, '\0', maxchars);
	sprintf(pidStr, "%d", pidNum);

	char *index;
	char word[] = "$$";
	char *newString;
	newString = malloc(maxchars);
	index = strstr(string, word);

	if (index)
	{
		int ind;
		int newStringPost = 0;

		for (ind = 0; ind < strlen(string); ind++)
		{
			if (string[ind] == '$' && string[ind + 1] == '$')
			{
				// printf("Current index: %i\n", ind);
				if (ind == 0)
				{
					strcpy(newString, pidStr);
				}

				else
				{
					strcat(newString, pidStr);
				}

				ind++; //skip next char
				newStringPost = newStringPost + strlen(pidStr);
			}

			else
			{
				newString[newStringPost] = string[ind];
				newStringPost++;
			}
		}

		// printf("Expanded string: %s\n", newString);
	}

	else
	{
		return string;
	}

	return newString;
}

/**********************************************************************************/
/********************************* runCommand *************************************/
/**********************************************************************************/

void runCommand(struct userInput *currentInput, int argCount)
{
	if (strcmp(currentInput->command, "exit") == 0)
	{

		if (argCount - 1 == 0)
		{
			exitCommand();
			exit(0);
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
		fflush(stdout);

		if (argCount - 1 > 1)
		{
			printf("Error! Maximum allowed argument is 1.\n");
			fflush(stdout);
		}

		if (argCount - 1 == 0)
		{
			memset(pathway, '\0', maxchars);
			strcpy(pathway, getenv("HOME"));
		}

		else if (argCount - 1 == 1)
		{
			memset(pathway, '\0', maxchars);
			strcpy(pathway, currentInput->arguments[1]);
		}

		if (chdir(pathway) == -1)
		{
			fprintf(stderr, "Cannot find %s\n", pathway);
		}
	}

	else if (strcmp(currentInput->command, "status") == 0)
	{
		if (argCount - 1 == 0)
		{
			if (lastFG != 0)
			{
				// printf("Last foreground process is %i\n", lastPid);
				// fflush(stdout);
				checkExitStatus(lastFGExitStatus);
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
		//runNonBuilt(currentInput);
		forkChild(currentInput);
	}
}

/**********************************************************************************/
/******************************** runNonBuilt *************************************/
/**********************************************************************************/

void runNonBuilt(struct userInput *currentInput)
{
	int argCount = currentInput->argCount;
	int sourceFD, targetFD, result, errorflag = 0;

	if (currentInput->backgroundFlag == 1 && fgModeFlag == 0) //block when in background mode
	{
		struct sigaction blockSigInt = {{0}};
		blockSigInt.sa_handler = SIG_IGN;
		sigaction(SIGINT, &blockSigInt, NULL);
	}

	// block in both foreground and background
	// struct sigaction blockSigTSTP = {{0}};
	// blockSigTSTP.sa_handler = SIG_IGN;
	// sigaction(SIGTSTP, &blockSigTSTP, NULL);

	if (currentInput->inputFlag == 1)
	{
		if (currentInput->backgroundFlag == 1 && fgModeFlag == 0) //background process
		{
			ioRedirect(0, "/dev/null", O_RDONLY, 0);
		}

		else
		{
			ioRedirect(0, currentInput->input[0], O_RDONLY, 0);
		}
	}

	if (currentInput->outputFlag == 1)
	{
		if (currentInput->backgroundFlag == 1 && fgModeFlag == 0) //background process
		{
			ioRedirect(1, "/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}
		else
		{
			ioRedirect(1, currentInput->output[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}
	}

	if (currentInput->backgroundFlag == 1  && fgModeFlag == 0) //background process
	{
		ioRedirect(1, "/dev/null", O_WRONLY, 0);
	}

	// no args
	if (argCount == 0 && currentInput->inputFlag == 0 && currentInput->outputFlag == 0)
	{
		// printf("No arguments and flags \n.");
		errorflag = execlp(currentInput->arguments[0], currentInput->arguments[0], NULL);
	}

	// only output and/or input
	else if (argCount == 0)
	{
		// printf("no arg count but flag detected \n.");
		errorflag = execlp(currentInput->arguments[0], currentInput->arguments[0], NULL);
	}

	// contains arguments and/or flags
	else
	{
		errorflag = execvp(currentInput->arguments[0], currentInput->arguments);
	}

	if (errorflag == -1)
	{
		printf("Exec error detected.\n");
		exit(1);
	}
}

/**********************************************************************************/
/********************************* forkChild **************************************/
/**********************************************************************************/

pid_t forkChild(struct userInput *currentInput)
{
	pid_t spawnPid = -5, statusID;
	int bgChildExitMethod = -5;
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
		runNonBuilt(currentInput);
		break;
	}
	default:
	{

		// foreground process
		if (currentInput->backgroundFlag == 0 || fgModeFlag == 1)
		{
			// printf("Executing foreground process %d\n", spawnPid);
			// fflush(stdout);

			lastFG = spawnPid;
			pid_t actualPid = waitpid(spawnPid, &lastFGExitStatus, 0);
		}

		// background process or non fg-mode
		else
		{
			parent = getpid();
			// printf("Executing background process %d\n", spawnPid);
			// only add background processes to the array
			bgPIDs[pidCount] = spawnPid;
			pidCount++;
			waitpid(spawnPid, &bgChildExitMethod, WNOHANG);
		}
	}
	}
	return spawnPid;
}

/**********************************************************************************/
/********************************** checkPID **************************************/
/**********************************************************************************/

void checkPIDs()
{
	int i;

	printf("Current parent is %d\n", parent);
	printf("Pid count is %i\n", pidCount);

	if (pidCount > 0)
	{
		for (i = 0; i < pidCount; i++)
		{
			printf("Child %i is %d\n", i + 1, bgPIDs[i]);
		}
		fflush(stdout);
	}
}

/**********************************************************************************/
/***************************** checkExitStatus ************************************/
/**********************************************************************************/

void checkExitStatus(int exitStatus)
{

	// child process has terminated normally
	if (WIFEXITED(exitStatus))
	{
		int exitStat = WEXITSTATUS(exitStatus);

		printf("Process %d exited with exit status %d\n", lastFG, exitStat);
		fflush(stdout);
	}

	else if (WIFSIGNALED(exitStatus))
	{
		int termSignal = WTERMSIG(exitStatus);
		printf("Process %d was terminated by signal %i\n", lastFG, termSignal);
		fflush(stdout);
	}
}

/**********************************************************************************/
/********************************* exitCommand ************************************/
/**********************************************************************************/

void exitCommand()
{
	int i;
	// kill all child processes
	for (i = 0; i < pidCount; i++)
	{
		kill(bgPIDs[i], SIGKILL);
	}
}

void checkStruct(struct userInput *currentInput)
{

	printf("Command is: %s\n", currentInput->command);
	fflush(stdout);

	int i = 0;

	printf("Arg count is: %i\n", currentInput->argCount);
	fflush(stdout);

	while (i < maxargs)
	{
		printf("Argument %i: %s\n", i, currentInput->arguments[i]);
		fflush(stdout);
		i++;
	}

	if (currentInput->inputFlag == 1)
	{
		printf("Input array item: %s\n", currentInput->input[0]);
		fflush(stdout);
	}

	if (currentInput->outputFlag == 1)
	{
		printf("Output array item: %s\n", currentInput->output[0]);
		fflush(stdout);
	}

	if (currentInput->backgroundFlag == 1)
	{
		printf("Background flag found\n");
		fflush(stdout);
	}
}

/**********************************************************************************/
/********************************* resetStruct ************************************/
/**********************************************************************************/

struct userInput *resetStruct(struct userInput *currentInput)
{
	memset(currentInput->command, '\0', maxchars);
	int i;
	while (currentInput->arguments[i])
	{
		currentInput->arguments[i] = NULL;
		i++;
	}
	memset(currentInput->input[0], '\0', maxchars);
	memset(currentInput->output[0], '\0', maxchars);
	currentInput->argCount = 0;
	currentInput->inputFlag = 0;
	currentInput->outputFlag = 0;
	currentInput->backgroundFlag = 0;

	return currentInput;
}

void ioRedirect(int inputOrOutput, char *filePath, int flag, mode_t mode)
{

	int result;

	if (inputOrOutput == 0) // input
	{
		int sourceFD;
		sourceFD = open(filePath, flag);
		if (sourceFD == -1)
		{
			perror("source open()");
			exit(1);
		}
		// printf("sourceFD == %d\n", sourceFD); // Written to terminal

		result = dup2(sourceFD, 0);
		if (result == -1)
		{
			perror("source dup2()");
			exit(1);
		}

		// close when exec is completed
		fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
	}

	else //output
	{

		int targetFD;
		targetFD = open(filePath, flag, mode);
		if (targetFD == -1)
		{
			perror("source open()");
			exit(1);
		}
		//printf("targetFD == %d\n", targetFD); // Written to terminal

		result = dup2(targetFD, 1);
		if (result == -1)
		{
			perror("target dup2()");
			exit(1);
		}

		// close when exec is completed
		fcntl(targetFD, F_SETFD, FD_CLOEXEC);
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
// https://stackoverflow.com/questions/15539708/passing-an-array-to-execvp-from-the-users-input
// http://www.cplusplus.com/reference/cstring/strstr/
// https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
// https://stackoverflow.com/questions/1242974/write-to-stdout-and-printf-output-not-interleaved
// https://stackoverflow.com/questions/14573000/print-int-from-signal-handler-using-write-or-async-safe-functions/52111436
// https://stackoverflow.com/questions/12953350/ignore-sigint-signal-in-child-process