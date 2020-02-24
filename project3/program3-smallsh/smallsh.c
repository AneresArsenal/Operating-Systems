
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

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
void forkChild(struct userInput *currentInput);
void checkBGPIDs();
void checkPIDs();
void runNonBuilt(struct userInput *currentInput);
void ioRedirect(int inputOrOutput, char *filePath, int flag, mode_t mode);
char input[maxchars];

/**********************************************************************************/
/********************************* sigint ****************************************/
/**********************************************************************************/

// Requirements:
// SIGINT does not terminate your shell, but only terminates the foreground command if one is running.
// The parent should not attempt to terminate the foreground child process when the parent receives a SIGINT signal
// The foreground child (if any) must terminate itself on receipt of this signal.
// If a child foreground process is killed by a signal, the parent must immediately print out the number of the signal
// that killed it's foreground child process before prompting the user for the next command.
// Background processes should also not be terminated by a SIGINT signal.
// They will terminate themselves, continue running, or be terminated when the shell exits (see below).

void CATCHsigint(int signal)
{
	int fgChildExitMethod = -5;
	// check last foreground process and make sure one exists
	// once process is terminated, print message
	if (lastFG != -99 && waitpid(lastFG, &lastFGExitStatus, 0) == lastFG)
	{
		// write(STDOUT_FILENO, "Process terminated by signal.\n", 31);
	}
	else
	{
		write(STDOUT_FILENO, "\n: ", 3);
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
}

/**********************************************************************************/
/*********************************** Main *****************************************/
/**********************************************************************************/

int main(void)
{
	// initialize signal handlers
	// initialize or reset the signal set to sa_mask
	// set to restart flag to tell system calls to automatically restart
	// specifies an alternative signal handler function to be called

	struct sigaction SIGINT_action = {{0}};
	struct sigaction SIGTSTP_action = {{0}};

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

		// get user input and return arg count which includes command
		argCount = getUserInput(currentInput);
		// check background processes to detect children processes that are done
		checkBGPIDs();

		// ignore comment blocks which are marked with -1 return
		if (argCount != -1)
		{
			// run command line
			// check background processes to detect children processes that are done
			checkBGPIDs();
			runCommand(currentInput, argCount);
		}

		else
		{
			// reset struct
			currentInput = resetStruct(currentInput);
			continue;
		}

		// check background processes to detect children processes that are done
		checkBGPIDs();
		// reset struct
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

	// reset string and input arrays after each use to clear previous input
	memset(string, '\0', maxchars);
	for (k = 0; k < maxargs; k++)
	{
		memset(inputs[k], '\0', maxchars);
	}

	printf(": ");
	fflush(stdout);
	fgets(string, maxchars, stdin);

	// add null terminator to string
	indexOfNullTerminator = strlen(string);
	string[indexOfNullTerminator - 1] = '\0';

	// delimit string with space
	char delim[] = " ";
	char *token;

	// ignore blank or comment lines
	if (strlen(string) == 0 || (string[0] == '#'))
	{
		// printf("Blank line or comment found!\n");
		// fflush(stdout);
		return -1;
	}

	// delimit string with space
	for (token = strtok(string, " "); token != NULL; token = strtok(NULL, " "))
	{
		if (i == 0) //store first string as command
		{
			strcpy(currentInput->command, token);
		}

		// input all strings into inputs array
		memset(inputs[i], '\0', maxchars);
		sprintf(inputs[i], "%s", token);

		i++;
	}

	count = i;

	i = 0;
	// add to argument as long as an i/o flag is not found or & as long as the & char is not the last string in the array count
	while (((strcmp(inputs[i], "&") != 0) || (i < count - 1)) && (strcmp(inputs[i], "<") != 0) && (strcmp(inputs[i], ">") != 0) && (strlen(inputs[i]) != 0))
	{
		// pass each string to the expansion function to expand all $$ before putting it into arguments array in struct
		char *finalStr = checkExpansion(inputs[i]);

		// add to struct
		currentInput->arguments[i] = finalStr;

		i++;
	}

	// assign NULL to final argument for exec function call
	currentInput->arguments[i] = NULL;

	// if i is less than count, it means that there are inputs that belongs to i/o flags or &,
	// process them into appropriate places in struct
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

			if ((strcmp(inputs[j], "&") == 0) && j == count - 1) //only take & that is placed at the final array, ignore rest
			{
				currentInput->backgroundFlag = 1;
			}
		}
	}

	// reduce final argument count if any of the flags are set to 1
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

	// assign count to struct
	currentInput->argCount = count;

	return count;
}

/**********************************************************************************/
/********************************* checkExpansion *********************************/
/**********************************************************************************/

char *checkExpansion(char *string)
{

	// printf("Pre-expansion string: %s\n", string);
	// reset variables used for expansion function
	int pidNum = (int)getpid();
	char pidStr[maxchars];
	memset(pidStr, '\0', maxchars);
	sprintf(pidStr, "%d", pidNum);

	char *index;
	char word[] = "$$";
	char *newString;
	newString = malloc(maxchars);
	index = strstr(string, word);

	// substring found in string
	if (index)
	{
		int ind;
		int newStringPost = 0;

		// iterate through entire string char by char to check if $$ exists
		// there might be more than 1 pair
		for (ind = 0; ind < strlen(string); ind++)
		{
			// check if $ exist in sequence
			if (string[ind] == '$' && string[ind + 1] == '$')
			{
				// printf("Current index: %i\n", ind);
				if (ind == 0)
				{
					// copy during first $$ found
					strcpy(newString, pidStr);
				}

				else
				{
					// concatenate after
					strcat(newString, pidStr);
				}

				ind++; //skip next char to make sure the second $ is read once

				// update position in new string
				newStringPost = newStringPost + strlen(pidStr);
			}

			else
			{
				// use position to add char by char to new string
				newString[newStringPost] = string[ind];
				newStringPost++;
			}
		}

		// printf("Expanded string: %s\n", newString);
	}

	else
	{
		// if no $$ is found, return original string
		return string;
	}

	return newString;
}

/**********************************************************************************/
/********************************* runCommand *************************************/
/**********************************************************************************/

void runCommand(struct userInput *currentInput, int argCount)
{
	// run exit built-in commands when found
	// Command requirements:
	// The exit command exits your shell. It takes no arguments.
	// When this command is run, your shell must kill any other processes or jobs
	// that your shell has started before it terminates itself.

	if (strcmp(currentInput->command, "exit") == 0)
	{

		if (argCount - 1 == 0)
		{
			exitCommand();
			exit(0);
		}

		else // no argument allowed
		{
			printf("Error! No argument is allowed.\n");
			fflush(stdout);
		}
	}

	// run cd built-in commands when found
	// Command requirements:
	// changes the working directory of your shell.
	// With no arguments - it changes to the directory specified in the HOME environment variable
	// With one argument: the path of a directory to change to.
	// support both absolute and relative paths. When smallsh terminates, the original shell it was launched from will still be in its original working directory,
	// despite your use of chdir() in smallsh. Your shell's working directory begins in whatever directory your shell's executible was launched from.

	else if (strcmp(currentInput->command, "cd") == 0)
	{
		char pathway[maxchars];
		memset(cwd, '\0', maxchars);
		getcwd(cwd, sizeof(cwd));
		// printf("Pre-call cwd: %s\n", cwd);
		fflush(stdout);

		if (argCount - 1 > 1) // too many argument provided
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

	// Command requirements:
	// The status command prints out either the exit status or the terminating signal of the last foreground process
	// If this command is run before any foreground command is run, then it should simply return the exit status 0.
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
				printf("No previous foreground command ran. Exit status %i.\n", lastFGExitStatus);
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
		// for all other non-built in commands, fork a child
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

	// if i/o flags are found, initiate redirection accordingly

	// if process is a foreground process
	if (currentInput->backgroundFlag == 0 || fgModeFlag == 1)
	{
		if (currentInput->inputFlag == 1)
		{
			ioRedirect(0, currentInput->input[0], O_RDONLY, 0);
		}

		if (currentInput->outputFlag == 1)
		{
			ioRedirect(1, currentInput->output[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}
	}

	// if process is a background process
	// Requirements are:
	// standard input redirected from /dev/null if the user did not specify some other file to take standard input from.
	// should also not send their standard output to the screen: redirect stdout to /dev/null if no other target is given.

	if (currentInput->backgroundFlag == 1 && fgModeFlag == 0)
	{
		// no redirection was provided
		if (currentInput->inputFlag == 1)
		{
			ioRedirect(0, currentInput->input[0], O_RDONLY, 0);
		}

		// no redirection was provided
		else
		{
			ioRedirect(0, "/dev/null", O_RDONLY, 0);
		}

		// redirection was provided
		if (currentInput->outputFlag == 1)
		{
			ioRedirect(1, currentInput->output[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}

		// no redirection was provided
		else
		{
			ioRedirect(1, "/dev/null", O_WRONLY, 0);
		}
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

// fork child using template from 3.2 Process Management& Zombies
void forkChild(struct userInput *currentInput)
{
	pid_t spawnPid = -5, statusID;
	int bgChildExitMethod = -5;
	spawnPid = fork();

	switch (spawnPid)
	{

	// fork is unsuccsessful
	case -1:
	{
		perror("Hull Breach!\n");
		exit(1);
		break;
	}

	// fork is successful, child is running
	case 0:
	{
		runNonBuilt(currentInput);
		break;
	}

	// handle child case
	default:
	{

		// if running foreground process (or when fg mode is activated), block until child is finish
		if (currentInput->backgroundFlag == 0 || fgModeFlag == 1)
		{
			lastFG = spawnPid;
			waitpid(spawnPid, &lastFGExitStatus, 0);

			// if child exited with a signal, print signal
			if (WIFSIGNALED(lastFGExitStatus))
			{
				checkExitStatus(lastFGExitStatus);
			}
		}

		// if running background process and in non fg-mode
		else
		{
			parent = getpid();
			printf("Executing background process %d\n", spawnPid);
			fflush(stdout);
			// only add background processes to the array to keep track
			bgPIDs[pidCount] = spawnPid;
			pidCount++;
			waitpid(spawnPid, &bgChildExitMethod, WNOHANG);
		}
	}
	}
}

/**********************************************************************************/
/********************************** checkBGPID **************************************/
/**********************************************************************************/

// zombies and background process management, check to make sure terminated bg pids are tracked and removed from tally of running bgs
void checkBGPIDs()
{
	int i;
	int bgChildExitMethod = -5;

	for (i = 0; i < pidCount; i++)
	{
		// if the return id matches the pid found in array when using a waitpid check, it means the it's been terminated
		// pass exit method to see if it was terminated by an exit status or a signal

		pid_t spawnid = waitpid(bgPIDs[i], &bgChildExitMethod, WNOHANG);
		if (spawnid > 0)
		{
			printf("Background process %d is terminated\n", spawnid);
			checkExitStatus(bgChildExitMethod);
			removePID(spawnid);
		}
	}
	fflush(stdout);
}

/**********************************************************************************/
/********************************** checkPID **************************************/
/**********************************************************************************/

// used during testing phase to make sure pids are stored accordingly in array
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
/********************************** removePID **************************************/
/**********************************************************************************/

// remove background pids from array that have been exited or terminated when detected
void removePID(pid_t pid)
{
	int i, j;

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
/***************************** checkExitStatus ************************************/
/**********************************************************************************/

void checkExitStatus(int exitStatus)
{
	// child process has terminated with an exit status
	if (WIFEXITED(exitStatus))
	{
		int exitStat = WEXITSTATUS(exitStatus);

		printf("Process exited with exit status %d\n", exitStat);
		// printf("Process %d exited with exit status %d\n", lastFG, exitStat);
		fflush(stdout);
	}

	// child process has terminated with a signal
	else if (WIFSIGNALED(exitStatus))
	{
		int termSignal = WTERMSIG(exitStatus);

		printf("Process was terminated by signal %i\n", termSignal);
		// printf("Process %d was terminated by signal %i\n", lastFG, termSignal);
		fflush(stdout);
	}
}

/**********************************************************************************/
/********************************* exitCommand ************************************/
/**********************************************************************************/

// when user input exit command
void exitCommand()
{
	int i;
	// kill all child bg processes
	for (i = 0; i < pidCount; i++)
	{
		kill(bgPIDs[i], SIGKILL);
	}

	if (lastFG != -99)
	{
		kill(lastFG, SIGKILL);
	}
}

/**********************************************************************************/
/********************************* check struct ************************************/
/**********************************************************************************/

// use to iterate through the struct content created during testing phase

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

// clear struct after each command has been executed

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

/**********************************************************************************/
/********************************* ioRedirect ************************************/
/**********************************************************************************/

void ioRedirect(int inputOrOutput, char *filePath, int flag, mode_t mode)
{

	int result;

	// for input redirection
	if (inputOrOutput == 0)
	{
		int sourceFD;
		sourceFD = open(filePath, flag);
		// if error detected
		if (sourceFD == -1)
		{
			perror("source open()");
			exit(1);
		}
		// printf("sourceFD == %d\n", sourceFD); // Written to terminal

		result = dup2(sourceFD, 0);
		// if error detected
		if (result == -1)
		{
			perror("source dup2()");
			exit(1);
		}

		// close when exec is completed
		fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
	}

	// for output redirection
	else
	{

		int targetFD;
		targetFD = open(filePath, flag, mode);
		// if error detected
		if (targetFD == -1)
		{
			perror("target open()");
			exit(1);
		}
		//printf("targetFD == %d\n", targetFD); // Written to terminal

		result = dup2(targetFD, 1);

		// if error detected
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