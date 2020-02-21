#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
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
		printf("CHILD(%d): Sleeping for 1 second\n", getpid());
		sleep(1);
		printf("CHILD(%d): Converting into \'ls -a\'\n", getpid());
		execlp("ls", "ls", "-a", NULL);
		perror("CHILD: exec failure!\n");
		exit(2);
		break;
	}
	default:
	{
		printf("PARENT(%d): Sleeping for 2 seconds\n", getpid());
		sleep(2);
		printf("PARENT(%d): Wait()ing for child(%d) to terminate\n", getpid(), spawnPid);
		pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
		printf("PARENT(%d): Child(%d) terminated, Exiting!\n", getpid(), actualPid);
		exit(0);
		break;
	}
	}
}