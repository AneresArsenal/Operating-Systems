#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// variables
#define totalRooms 10
#define roomCount 7
#define typeCount 3
#define maxchars 100
#define maxconnections 6
char pathFile[maxchars];
char cwd[maxchars];
char roomNames[totalRooms][9];
char roomTypes[typeCount][maxchars];
char roomAssignedTypes[roomCount][maxchars];
char filePaths[roomCount][maxchars];
char roomNames[totalRooms][9];
int roomNums[roomCount + 1];
int roomType[roomCount + 1];
int connections[roomCount + 1][maxconnections + 1]; //max connections is 6
typedef enum _bool Bool;
enum _bool
{
	false = 0,
	true = 1
};

// function definitions
void createDir();
void createRooms();
void randomRoom(int *array, int size, int max);
void assignRoomTypes();
void writeFiles();
Bool IsGraphFull();
void AddRandomConnection();
Bool CanAddConnectionFrom(int x);
Bool ConnectionAlreadyExists(int x, int y);
void ConnectRoom(int x, int y);
// Bool IsSameRoom(int x, int y);
char *getcwd(char *buf, size_t size);
struct stat st = {0};

/********************************* Room name requirements *********************************/
// A room name cannot be assigned to more than one room.
// Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed
// (thus, no numbers, special characters, or spaces).
// This restriction is not extended to the room file's filename.
// You must hard code a list of ten different Room Names into your rooms program and
// have your rooms program randomly assign one of these to each room generated.
// Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.

int main(void)
{
	srand(time(NULL));
	int i, j;

	createDir();
	createRooms();
	assignRoomTypes();

	// initialize all int arrays with -1
	for (i = 0; i < roomCount; i++)
	{
		for (j = 0; j < maxconnections; j++)
		{
			connections[i][j] = -1;
		}
	}

	// Create all connections in graph
	while (IsGraphFull() == false)
	{
		AddRandomConnection();
	}

	for (i = 0; i < roomCount; i++)
	{
		printf("Room %d %s %s with the following connections: \n", i, roomNames[roomNums[i]], roomAssignedTypes[i]);
		for (j = 0; j < maxconnections; j++)
		{
			printf("Room %d ", connections[i][j]);
		}

		printf("\n\n");
	}

	writeFiles();

	return 0;
}

void createDir()
{
	// if the noted directory does not exist
	if (stat("tays.rooms", &st) == -1)
	{
		mkdir("tays.rooms", 0700);
		printf("Directory created! \n");
	}

// if the noted directory exist, remove old and create new one
	else
	{
		printf("Directory exists! Remove old directory. \n");

		// remove files in old directory
		system("rm -rf tays.rooms/");
		// remove old directory
		rmdir("tays.rooms");

		//create new one
		mkdir("tays.rooms", 0700);
		printf("Neww directory created! \n");

	}
}

void createRooms()
{
	int i;
	// initialize all 10 string arrays with null terminator
	for (i = 0; i < 10; i++)
	{
		memset(roomNames[i], '\0', 9);
	}

	// hard code names to string arrays
	strcpy(roomNames[0], "pine");
	strcpy(roomNames[1], "oak");
	strcpy(roomNames[2], "willow");
	strcpy(roomNames[3], "aspen");
	strcpy(roomNames[4], "juniper");
	strcpy(roomNames[5], "cedar");
	strcpy(roomNames[6], "fir");
	strcpy(roomNames[7], "larch");
	strcpy(roomNames[8], "spruce");
	strcpy(roomNames[9], "birch");

	int file_descriptor;
	char *directory = "tays.rooms";

	int j;

	// choose random room
	memset(roomNums, -1, (roomCount + 1));
	randomRoom(roomNums, roomCount, totalRooms);

	// get current directory's file path
	memset(cwd, '\0', maxchars);
	getcwd(cwd, sizeof(cwd));
	// get parent process id
	char pid[10];
	memset(pid, '\0', 10);
	int pidNum = (int)getppid();
	sprintf(pid, "%d", pidNum);
	// create files in directory
	for (i = 0; i < roomCount; i++)
	{
		// reset char array
		memset(pathFile, '\0', maxchars);
		sprintf(pathFile, "%s/%s/%s.%s.c", cwd, directory, roomNames[roomNums[i]], pid);
		open(pathFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		memset(filePaths[i], '\0', maxchars);
		sprintf(filePaths[i], "%s", pathFile);
	}
}

void randomRoom(int *array, int size, int max)
{
	int i;
	int j = 0;
	int k;
	int num;
	Bool flag;

	for (i = 0; i < size; i++)
	{

		if (i == 0)
		{
			j = 0;
			num = (rand() % max);
			array[i] = num;
			// printf("Number stored in position %d is %d \n", i, array[i]);
		}

		else
		{
			j++;
			do
			{
				// reset flag for each iteration
				flag = false;

				// generate random number
				num = (rand() % 10);
				// printf("Generated number is %d \n", num);
				// printf("Check if number exist.... \n");

				// loop through array to make sure number does not exist (do not want duplicates)
				// if a duplicate is found, dont use it
				for (k = 0; k < j; k++)
				{
					if (array[k] == num)
					{
						// printf("Duplicate found! \n");
						flag = true;
					}
				}

			} while (flag == true);
			array[i] = num;
			// printf("Number stored in position %d is %d \n", i, array[i]);
		}
	}
}

void assignRoomTypes()
{
	int i;
	// initialize all 3 string arrays with null terminator
	for (i = 0; i < typeCount; i++)
	{
		memset(roomTypes[i], '\0', maxchars);
	}
	// hard code room types to string arrays
	strcpy(roomTypes[0], "START_ROOM");
	strcpy(roomTypes[1], "END_ROOM");
	strcpy(roomTypes[2], "MID_ROOM");

	// choose random room
	memset(roomType, -1, (roomCount + 1));
	randomRoom(roomType, roomCount, roomCount);

	for (i = 0; i < roomCount; i++)
	{
		if (i < 2)
		{
			strcpy(roomAssignedTypes[i], roomTypes[i]);
		}

		else
		{
			strcpy(roomAssignedTypes[i], roomTypes[2]);
		}
	}
}

void writeFiles()
{
	int i, j;
	int file_descriptor;
	char room[maxchars] = "ROOM NAME:";
	char type[maxchars] = "ROOM TYPE:";
	char connection[maxchars] = "CONNECTION";
	ssize_t nread, nwritten;
	char readBuffer[2000];

	for (i = 0; i < roomCount; i++)
	{
		// printf("%s \n", filePaths[i]);
		file_descriptor = open(filePaths[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		if (file_descriptor == -1)
		{
			printf("Hull breach - open() failed on \"%s\"\n", filePaths[i]);
			exit(1);
		}

		sprintf(room, "%s %s \n", room, roomNames[roomNums[i]]);
		nwritten = write(file_descriptor, room, strlen(room) * sizeof(char));

		for (j = 0; j < maxconnections; j++)
		{
			if (connections[i][j] == -1)
			{
				break;
			}
			sprintf(connection, "%s %d: %s \n", connection, (j + 1), roomNames[roomNums[connections[i][j]]]);
			nwritten = write(file_descriptor, connection, strlen(connection) * sizeof(char));
			memset(connection, '\0', maxchars);
			sprintf(connection, "CONNECTION");
		}

		sprintf(type, "%s %s \n", type, roomAssignedTypes[i]);
		nwritten = write(file_descriptor, type, strlen(type) * sizeof(char));

		memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
		lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the beginning of the file
		nread = read(file_descriptor, readBuffer, sizeof(readBuffer));
		printf("File contents:\n%s\n", readBuffer);

		memset(room, '\0', maxchars);
		sprintf(room, "ROOM NAME:");
		memset(type, '\0', maxchars);
		sprintf(type, "ROOM TYPE:");
	}
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
Bool IsGraphFull()
{
	int i, j;
	Bool false;

	for (i = 0; i < roomCount; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (connections[i][j] == -1)
			{
				return false;
			}
		}
	}

	return true;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection()
{
	int A; // Maybe a struct, maybe global arrays of ints
	int B;
	int i;

	while (true)
	{
		A = (rand() % roomCount);

		if (CanAddConnectionFrom(A) == true)
		{
			// printf("A = Room %d selected to add connection \n", A);
			break;
		}
	}

	do
	{
		B = (rand() % roomCount);
	} while (CanAddConnectionFrom(B) == false || A == B || ConnectionAlreadyExists(A, B) == true);

	// printf("B = Room %d selected to add connection \n", B);

	// printf("Pre - Connection for A: Room %d (%s) \n", A, roomNames[roomNums[A]]);

	// for (i = 0; i < maxconnections; i++)
	// {
	// 	printf("%d ", connections[A][i]);
	// }

	// printf("\n ");
	// printf("Pre - Connection for B: Room %d (%s) \n", B, roomNames[roomNums[B]]);
	// for (i = 0; i < maxconnections; i++)
	// {
	// 	printf("%d ", connections[B][i]);
	// }
	// printf("\n ");

	ConnectRoom(A, B); // TODO: Add this connection to the real variables,
	ConnectRoom(B, A); //  because this A and B will be destroyed when this function terminates

	// printf("Post - Connection for A: Room %d (%s) \n", A, roomNames[roomNums[A]]);

	// for (i = 0; i < maxconnections; i++)
	// {
	// 	printf("%d ", connections[A][i]);
	// }

	// printf("\n ");
	// printf("Post - Connection for B: Room %d (%s) \n", B, roomNames[roomNums[B]]);
	// for (i = 0; i < maxconnections; i++)
	// {
	// 	printf("%d ", connections[B][i]);
	// }
	// printf("\n ");
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
Bool CanAddConnectionFrom(int x)
{
	int j;
	Bool false;

	for (j = 0; j < maxconnections; j++)
	{
		if (connections[x][j] == -1)
		{
			// spot found
			// printf("Spot found at position %d \n", j);
			return true;
		}
	}

	return false;
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
Bool ConnectionAlreadyExists(int x, int y)
{
	int j;
	for (j = 0; j < maxconnections; j++)
	{
		if (connections[x][j] == y)
		{
			// connection exist
			// printf("Connection exist in position %d \n", j);
			return true;
		}
	}

	return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(int x, int y)
{
	int j;
	for (j = 0; j < maxconnections; j++)
	{
		if (connections[x][j] == -1)
		{
			// spot found! Assigned number to spot
			// printf("Connection formed for A: Room %d (%s) with B: Room %d (%s) at position %d \n", x, roomNames[roomNums[x]], y, roomNames[roomNums[y]], j);
			connections[x][j] = y;
			break;
		}
	}
}

// // Returns true if Rooms x and y are the same Room, false otherwise
// Bool IsSameRoom(int x, int y)
// {
//   if (x==y){
// 	  return true;
//   }

//   else{
// 	  return false;
//   }
// }

// Reference:
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
// https://www.geeksforgeeks.org/create-directoryfolder-cc-program/
// https://stackoverflow.com/questions/1088622/how-do-i-create-an-array-of-strings-in-c
// http://web.engr.oregonstate.edu/~brewsteb/CS344Slides/2.4%20File%20Access%20in%20C.pdf
// https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
