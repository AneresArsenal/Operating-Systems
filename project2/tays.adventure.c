#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define maxchars 100
#define roomCount 7
#define maxconnections 6
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
void readFiles();
void writeFiles();
void startGame();
void cleanRooms();
void findStartRoom();
Bool checkInput(char *input, int current);
void travelToRoom();
void printRoom(int current);
int getRoom(char *name);

int main(void)
{
	int j, i;

	DIR *d;
	struct dirent *dir;
	// d = opendir("./tays.rooms");
	// if (d)
	// {
	// 	i = 0;
	// 	while ((dir = readdir(d)) != NULL)
	// 	{

	// 		memset(filePaths[i], '\0', maxchars);

	// 		if (dir->d_type == DT_REG)
	// 		{
	// 			strcpy(filePaths[i], dir->d_name);
	// 			// printf("%s\n", dir->d_name);
	// 			i++;
	// 		}
	// 	}
	// 	closedir(d);
	// }

	getLatestDir();

	// readFiles();
	// cleanRooms();
	// findStartRoom();

	// while (strcmp(roomTypes[currentRoom], "END_ROOM") != 0)
	// {
	// 	travelToRoom();
	// }

	// printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	// printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
	// for (i = 0; i <= stepCount; i++)
	// {
	// 	printf("%s\n", pathHistory[i]);
	// }
	return 0;
}

void getLatestDir()
{
	DIR *dirp = opendir(".");
	struct dirent *dir;
	struct stat dStat;
	time_t latest = 0;
	char dName[maxchars];

	while ((dir = readdir(dirp)) != NULL)
	{
		memset(&dStat, 0, sizeof(dStat));
		if (stat(dir->d_name, &dStat) < 0)
		{
			printf("Error getting info on file\n");
			continue;
		}
		// If not a directory skip
		if ((dStat.st_mode & S_IFDIR) != S_IFDIR)
		{
			continue;
		}
		// check with the latest timestamp
		if (dStat.st_mtime > latest)
		{
			// On finding a more recent file switch that to latest
			strcpy(dName, dir->d_name);
			latest = dStat.st_mtime;
		}
	}



	closedir(dirp);
	printf("Most recently touched directory %s\n", dName);
}
void readFiles()
{
	int i, j, k;
	int file_descriptor;
	char line[maxchars];
	ssize_t nread, nwritten;
	char readBuffer[2000];

	// get current directory's file path
	memset(cwd, '\0', maxchars);
	getcwd(cwd, sizeof(cwd));

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

		sprintf(readFilePaths[i], "%s/tays.rooms/%s", cwd, filePaths[i]);
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
		flag = checkInput(input, currentRoom);
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
		flag = checkInput(input, currentRoom);
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

void writeFiles()
{
	int i, j;
	int file_descriptor;
	char room[maxchars];
	char type[maxchars];
	char path[maxchars];
	char filePaths[roomCount][maxchars];
	ssize_t nread, nwritten;
	char readBuffer[2000];

	// create files in directory
	for (i = 0; i < roomCount; i++)
	{
		// reset char array
		memset(path, '\0', maxchars);
		sprintf(path, "%s/%d.c", cwd, i);
		file_descriptor = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		memset(filePaths[i], '\0', maxchars);
		sprintf(filePaths[i], "%s", path);

		if (file_descriptor == -1)
		{
			printf("Hull breach - open() failed on \"%s\"\n", filePaths[i]);
			exit(1);
		}

		// sprintf(room, "%s", roomNames[i]);
		// nwritten = write(file_descriptor, room, strlen(room) * sizeof(char));

		sprintf(type, "%s", roomTypes[i]);
		nwritten = write(file_descriptor, type, strlen(type) * sizeof(char));

		memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
		lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the
	}
}

// Reference:
// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
// https://stackoverflow.com/questions/2925241/how-to-open-a-text-file-thats-not-in-the-same-folder
// https://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c
// https://stackoverflow.com/questions/5711490/c-remove-the-first-character-of-an-array
// https://stackoverflow.com/questions/20056587/trying-to-remove-the-last-character-in-a-char-array-in-c