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
char filePaths[roomCount+1][maxchars];
char roomNames[roomCount+1][maxchars];
char roomTypes[roomCount+1][maxchars];
char roomConnections[roomCount+1][maxconnections+1][maxchars];
char readFilePaths[roomCount+1][maxchars+1];
char cwd[maxchars+1];
char *getcwd(char *buf, size_t size);
int roomCounts[roomCount+1];

void readFiles();
void cleanLastRow();

int main(void)
{
	int j, i;
	DIR *d;
	struct dirent *dir;
	d = opendir("./tays.rooms");
	if (d)
	{
		i = 0;
		while ((dir = readdir(d)) != NULL)
		{

			memset(filePaths[i], '\0', maxchars);

			if (dir->d_type == DT_REG)
			{
				strcpy(filePaths[i], dir->d_name);
				// printf("%s\n", dir->d_name);
				i++;
			}
		}
		closedir(d);
	}

	readFiles();
	cleanLastRow();

	// for (i = 0; i < roomCount; i++)
	// {
	// 	// printf("%s \n", filePaths[i]);
	// 	printf("Room %d name: %s \n", i, roomNames[i]);

	// 	for (j = 0; j < maxconnections; j++)
	// 	{
	// 		printf("Room Connection %d:%s \n", (j+1), roomConnections[i][j]);
	// 	}
	// 	printf("Room Type: %s \n", roomTypes[i]);
	// }
	return 0;
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

void cleanLastRow()
{
	int i, j;

	for (i = 0; i < roomCount; i++)
	{
		// printf("%s \n", filePaths[i]);
		// printf("Room %d name: %s \n", i, roomNames[i]);

		printf("Room %d name: %s \n", i, roomNames[i]);

		for (j = 0; j < roomCounts[i]; j++)
		{
			printf("Room Connection %d:%s \n", j, roomConnections[i][j]);
		}

		printf("Room %d type: %s \n", i, roomTypes[i]);

	}
}
// Reference:
// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
// https://stackoverflow.com/questions/2925241/how-to-open-a-text-file-thats-not-in-the-same-folder
// https://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c
