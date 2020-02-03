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

#define maxchars 100
#define roomCount 7
#define maxconnections 6
char timeFilePath[maxchars];
char dName[maxchars];
char filePaths[roomCount + 1][maxchars];
char roomNames[roomCount + 1][maxchars];
char roomTypes[roomCount + 1][maxchars];
int currentRoom;
char input[maxchars];
char roomConnections[roomCount + 1][maxconnections + 1][maxchars];
char readFilePaths[roomCount + 1][maxchars + 1];
char cwd[maxchars + 1];
char *getcwd(char *buf, size_t size);
int roomCounts[roomCount + 1];
size_t indexOfNullTerminator;
typedef enum _bool Bool;
enum _bool
{
	false = 0,
	true = 1
};
int stepCount;
char pathHistory[maxchars][maxchars];

void getLatestDir();
void getFilePaths(char *str);
void readFiles();
void writeFiles();
void startGame();
void cleanRooms();
void findStartRoom();
Bool checkInput(char *input, int current);
void travelToRoom();
void printRoom(int current);
int getRoom(char *name);
void printEnding();

// time thread functions
int pthread_create(pthread_t *thread,
				   const pthread_attr_t *attr,
				   void *(*start_routine)(void *argv),
				   void *arg);
void *start_routine(void *parm);
void *record_time(void *parm);
pthread_mutex_t lock;
pthread_t myThreadID;
void printTime();

int main(void)
{
	int j, i;

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init has failed\n");
		return 1;
	}

	// get current directory's file path
	memset(cwd, '\0', maxchars);
	getcwd(cwd, sizeof(cwd));

	getLatestDir();
	getFilePaths(dName);

	readFiles();
	cleanRooms();
	findStartRoom();

	while (strcmp(roomTypes[currentRoom], "END_ROOM") != 0)
	{
		travelToRoom();
	}

	printEnding();

	pthread_mutex_destroy(&lock);
	exit(EXIT_SUCCESS);
}

void getLatestDir()
{
	DIR *dirp = opendir(".");

	struct dirent *dir;
	struct tm *time;
	time_t latest = 0;

	char timeConverted[maxchars];

	while (((dir = readdir(dirp)) != NULL) && (dirp != NULL))
	{

		// memset(dName, '\0', sizeof(dName));
		if (strncmp(dir->d_name, "tays.rooms", strlen("tays.rooms")) != 0)
		{
			continue;
		}

		else
		{

			struct stat dStat;
			stat(dir->d_name, &dStat);

			// printf("Current directory: %s\n", dir->d_name);
			// printf("Last file modification:   %s", ctime(&dStat.st_mtime));
			// printf("Pre Lastest time:   %s", ctime(&latest));

			if (dStat.st_mtime > latest)
			{
				// On finding a more recent file switch that to latest
				memset(dName, '\0', sizeof(dName));
				strcpy(dName, dir->d_name);
				latest = dStat.st_mtime;
				// printf("Post Lastest time:   %s", ctime(&latest));
				// printf("Newest directory: %s\n", dName);
			}
			memset(&dStat, 0, sizeof(dStat));
		}

		// check with the latest timestamp
	}

	closedir(dirp);
	printf("The most recently touched directory %s\n", dName);
}

void getFilePaths(char *str)
{
	int i;
	DIR *d;
	struct dirent *dir;

	d = opendir(str);
	if (d)
	{
		i = 0;
		while ((dir = readdir(d)) != NULL)
		{

			memset(filePaths[i], '\0', maxchars);

			if (dir->d_type == DT_REG)
			{
				strcpy(filePaths[i], dir->d_name);
				// printf("File path %s saved! \n", filePaths[i]);
				i++;
			}
		}
		closedir(d);
	}
}
void readFiles()
{
	int i, j, k;
	int file_descriptor;
	char line[maxchars];
	ssize_t nread, nwritten;
	char readBuffer[2000];

	// initialize all int arrays with -1
	for (i = 0; i < roomCount; i++)
	{
		for (j = 0; j < maxconnections; j++)
		{
			memset(roomConnections[i][j], '\0', maxchars);
		}
	}

	FILE *file;

	for (i = 0; i < roomCount; i++)
	{

		sprintf(readFilePaths[i], "%s/%s/%s", cwd, dName, filePaths[i]);
		// printf("%s\n", readFilePaths[i]);

		file_descriptor = open(readFilePaths[i], O_RDWR);

		if (file_descriptor == -1)
		{
			printf("Hull breach - open() failed on \"%s\"\n", readFilePaths[i]);
			exit(1);
		}

		memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
		lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the beginning of the file
													  // nread = read(file_descriptor, readBuffer, sizeof(readBuffer));
													  // printf("File contents:\n%s\n", readBuffer);
		j = 0;

		file = fopen(readFilePaths[i], "r"); /* should check the result */
		char delim[] = ":";
		char *token;

		while (fgets(line, sizeof(line), file))
		{
			// printf("Line %d \n", j);

			k = 0;

			for (token = strtok(line, delim); token; token = strtok(NULL, delim))
			{
				if (k == 1 && j == 0)
				{
					// printf("Room name (in readFile function): %s\n", token);
					sprintf(roomNames[i], "%s", token);
				}

				if (k == 1 && j != 0)
				{
					sprintf(roomConnections[i][j - 1], "%s", token);
					// printf("Room connection (in readFile function): %s", token);
					memset(readBuffer, '\0', sizeof(readBuffer));
					sprintf(readBuffer, "%s", token);
				}

				k++;
			}
			j++;

			// printf("%s", line);
		}

		// memset(roomConnections[i][j - 1], '\0', maxchars);
		roomCounts[i] = j - 2;
		// printf("Connection count: %d \n", roomCounts[i]);

		sprintf(roomTypes[i], "%s", readBuffer);
		// printf("Room Type (in readFile function): %s \n", roomTypes[i]);

		fclose(file);
	}
}

void findStartRoom()
{
	int i, j;

	for (i = 0; i < roomCount; i++)
	{
		// printf("CURRENT LOCATION:%s", roomNames[i]);
		if (strcmp(roomTypes[i], "START_ROOM") == 0)
		{
			// set to current room
			currentRoom = i;
			printRoom(currentRoom);
		}
	}

	Bool flag = false;

	while (flag == false)
	{
		printf("WHERE TO? >");
		memset(input, '\0', maxchars);
		fgets(input, maxchars, stdin);
		indexOfNullTerminator = strlen(input);
		input[indexOfNullTerminator - 1] = '\0';
		if (strcmp(input, "time") == 0)
		{
			pthread_mutex_unlock(&lock);
			pthread_create(&myThreadID, NULL, start_routine, NULL);
			pthread_join(myThreadID, NULL);
			pthread_mutex_lock(&lock);
			printTime();
		}
		else
		{

			flag = checkInput(input, currentRoom);
		}
	}

	// printf("Correct input! %s \n", input);
}

void cleanRooms()
{
	int i, j;

	// clean room names and connections
	for (i = 0; i < roomCount; i++)
	{
		// room names
		indexOfNullTerminator = strlen(roomNames[i]);
		roomNames[i][indexOfNullTerminator - 1] = '\0';
		memmove(roomNames[i], roomNames[i] + 1, strlen(roomNames[i]));

		// printf("%s", roomNames[i]);

		// room types
		// remove space before string
		indexOfNullTerminator = strlen(roomTypes[i]);
		roomTypes[i][indexOfNullTerminator - 1] = '\0';
		memmove(roomTypes[i], roomTypes[i] + 1, strlen(roomTypes[i]));
		// printf("%s", roomTypes[i]);

		for (j = 0; j < roomCounts[i]; j++)
		{
			indexOfNullTerminator = strlen(roomConnections[i][j]);
			roomConnections[i][j][indexOfNullTerminator - 1] = '\0';
			memmove(roomConnections[i][j], roomConnections[i][j] + 1, strlen(roomConnections[i][j]));
		}
	}
}

void travelToRoom()
{
	int j;
	printRoom(currentRoom);
	Bool flag = false;

	while (flag == false)
	{
		printf("WHERE TO? >");
		memset(input, '\0', maxchars);
		fgets(input, maxchars, stdin);
		indexOfNullTerminator = strlen(input);
		input[indexOfNullTerminator - 1] = '\0';
		if (strcmp(input, "time") == 0)
		{
			pthread_mutex_unlock(&lock);
			pthread_create(&myThreadID, NULL, start_routine, NULL);
			pthread_join(myThreadID, NULL);
			pthread_mutex_lock(&lock);
			printTime();
		}
		else
		{

			flag = checkInput(input, currentRoom);
		}
	}
}

Bool checkInput(char *input, int current)
{
	int j;

	for (j = 0; j < roomCounts[current]; j++)
	{
		// printf("Input:%s\n", input);
		// printf("Name check:%s\n", roomConnections[current][j]);
		if (strcmp(roomConnections[current][j], input) == 0)
		{
			// set new currentRoom
			currentRoom = getRoom(input);
			memset(pathHistory[stepCount], '\0', maxchars);
			sprintf(pathHistory[stepCount], "%s", roomNames[currentRoom]);
			stepCount++;

			// printRoom(currentRoom);
			return true;
		}
	}

	printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
	printRoom(currentRoom);

	return false;
}

void printRoom(int current)
{
	int i = current;
	int j;

	printf("CURRENT LOCATION: %s\n", roomNames[i]);
	printf("POSSIBLE CONNECTIONS:");
	for (j = 0; j < roomCounts[i]; j++)
	{
		printf(" %s", roomConnections[i][j]);
		if (j < roomCounts[i] - 1)
		{
			printf(",");
		}
		else
		{
			printf(".");
		}
	}
	printf("\n");
}

int getRoom(char *name)
{

	int j;
	for (j = 0; j < roomCount; j++)
	{
		if (strcmp(name, roomNames[j]) == 0)
		{
			// printf("Found room!\n");
			return j;
		}
	}

	return -1;
}

void printEnding()
{
	int i;
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
	for (i = 0; i <= stepCount; i++)
	{
		printf("%s", pathHistory[i]);
		if (i != stepCount)
		{
			printf("\n");
		}
	}
}

void *start_routine(void *parm)
{
	pthread_mutex_lock(&lock);

	int file_descriptor;
	ssize_t nread, nwritten;
	char readBuffer[2000];
	FILE *file;

	sprintf(timeFilePath, "%s/currentTime.txt", cwd);
	file_descriptor = open(timeFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (file_descriptor == -1)
	{
		printf("Hull breach - open() failed on \"%s\"\n", timeFilePath);
		exit(1);
	}

	lseek(file_descriptor, 0, SEEK_SET);
	file = fopen(timeFilePath, "w");

	char outstr[maxchars];
	char format[] = "%-I:%M%P, %A, %B %d, %Y"; // 1:03pm, Tuesday, September 13, 2016
	time_t now = time(0);
	struct tm *timeInfo;
	timeInfo = localtime(&now);

	// memset(format, '\0', sizeof(format));

	if (timeInfo == NULL)
	{
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(outstr, sizeof(outstr), format, timeInfo) == 0)
	{
		fprintf(stderr, "strftime returned 0");
		exit(EXIT_FAILURE);
	}

	// printf("Result string is %s \n", outstr);
	nwritten = write(file_descriptor, outstr, strlen(outstr) * sizeof(char));
	memset(outstr, '\0', maxchars);

	// return NULL;
	fclose(file);
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void printTime()
{
	int file_descriptor;
	char line[maxchars];
	ssize_t nread, nwritten;
	char readBuffer[2000];
	FILE *file;

	sprintf(timeFilePath, "%s/currentTime.txt", cwd);
	file_descriptor = open(timeFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	if (file_descriptor == -1)
	{
		printf("Hull breach - open() failed on \"%s\"\n", timeFilePath);
		exit(1);
	}

	lseek(file_descriptor, 0, SEEK_SET);
	file = fopen(timeFilePath, "r");
	printf("\n");
	while (fgets(line, sizeof(line), file))
	{
		printf("%s\n", line);
	}
	printf("\n");
	// return NULL;
	fclose(file);
}

// void writeFiles()
// {
// 	int i, j;
// 	int file_descriptor;
// 	char room[maxchars];
// 	char type[maxchars];
// 	char path[maxchars];
// 	char filePaths[roomCount][maxchars];
// 	ssize_t nread, nwritten;
// 	char readBuffer[2000];

// 	// create files in directory
// 	for (i = 0; i < roomCount; i++)
// 	{
// 		// reset char array
// 		memset(path, '\0', maxchars);
// 		sprintf(path, "%s/%d.c", cwd, i);
// 		file_descriptor = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

// 		memset(filePaths[i], '\0', maxchars);
// 		sprintf(filePaths[i], "%s", path);

// 		if (file_descriptor == -1)
// 		{
// 			printf("Hull breach - open() failed on \"%s\"\n", filePaths[i]);
// 			exit(1);
// 		}

// 		// sprintf(room, "%s", roomNames[i]);
// 		// nwritten = write(file_descriptor, room, strlen(room) * sizeof(char));

// 		sprintf(type, "%s", roomTypes[i]);
// 		nwritten = write(file_descriptor, type, strlen(type) * sizeof(char));

// 		memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
// 		lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the
// 	}
// }

// Reference:
// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
// https://stackoverflow.com/questions/2925241/how-to-open-a-text-file-thats-not-in-the-same-folder
// https://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c
// https://stackoverflow.com/questions/5711490/c-remove-the-first-character-of-an-array
// https://stackoverflow.com/questions/20056587/trying-to-remove-the-last-character-in-a-char-array-in-c
// https://stackoverflow.com/questions/23595397/use-of-regular-expressions-in-c-for-strcmp-function
// https://stackoverflow.com/questions/10446526/get-last-modified-time-of-file-in-linux
// https://stackoverflow.com/questions/26306644/how-to-display-st-atime-and-st-mtime/26307281
// https://stackoverflow.com/questions/26070059/c-control-reaches-end-of-non-void-function
// https://stackoverflow.com/questions/6990888/c-how-to-create-thread-using-pthread-create-function
// https://linux.die.net/man/3/strftime
// https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
// https://www.programiz.com/python-programming/datetime/strftime