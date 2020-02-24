#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// global variables
#define totalRooms 10
#define roomCount 7
#define typeCount 3
#define maxchars 100
#define maxconnections 6
char pathFile[maxchars];
char cwd[maxchars];
char pid[10];
char directory[maxchars];
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

/***************************************** Rooms program *****************************************/
// this program creates the directory and the rooms for the  colossal cave adventure program
// it uses the graph method to connect the rooms to ensure it adheres to the constraints
/*************************************************************************************************/

int main(void)
{
	srand(time(NULL));
	int i, j;

	// get the process id of the current program run
	memset(pid, '\0', 10);
	int pidNum = (int)getpid();
	sprintf(pid, "%d", pidNum);

	// call functions to create the directory and rooms for the game
	// assign the room types to the individual rooms
	createDir();
	createRooms();
	assignRoomTypes();

	// initialize all int arrays with -1 for the connections array to clear it from
	// previous assignment
	for (i = 0; i < roomCount; i++)
	{
		for (j = 0; j < maxconnections; j++)
		{
			connections[i][j] = -1;
		}
	}

	Bool flag = false;

	// Create all connections in graph while the requirements are not met (i.e false)
	while (flag == false)
	{
		flag = IsGraphFull();
		AddRandomConnection();
	}

	// display the current room with the corresponding connections to the console for testing purposes
	// for (i = 0; i < roomCount; i++)
	// {
	// 	printf("Room %d %s %s with the following connections: \n", i, roomNames[roomNums[i]], roomAssignedTypes[i]);
	// 	for (j = 0; j < maxconnections; j++)
	// 	{
	// 		printf("Room %d ", connections[i][j]);
	// 	}

	// 	printf("\n\n");
	// }

	// write the data into the room files
	writeFiles();

	return 0;
}

/***************************************** Create directory *****************************************/
// create a directory that contains all  the 7 room files
/*************************************************************************************************/

void createDir()
{
	// reset char array
	memset(directory, '\0', sizeof(directory));

	// include dir name and pid into the complete directory name
	sprintf(directory, "tays.rooms.%s", pid);

	// if directory does not exist, create it
	if (stat(directory, &st) == -1)
	{
		mkdir(directory, 0700);
		// printf("Directory created! \n");
	}

	// if the noted directory exist, throw an error message
	else
	{
		printf("Directory exists! Error! \n");
	}
}

/********************************* Room name requirements *********************************/
// A room name cannot be assigned to more than one room.
// Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed
// (thus, no numbers, special characters, or spaces).
// This restriction is not extended to the room file's filename.
// You must hard code a list of ten different Room Names into your rooms program and
// have your rooms program randomly assign one of these to each room generated.
// Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.
/******************************************************************************************/

void createRooms()
{
	int i;
	int file_descriptor;
	char *directoryptr = directory;

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

	// choose random room
	memset(roomNums, -1, (roomCount + 1));
	randomRoom(roomNums, roomCount, totalRooms);

	// get current directory's file path
	memset(cwd, '\0', maxchars);
	getcwd(cwd, sizeof(cwd));

	// create files in directory
	for (i = 0; i < roomCount; i++)
	{
		// reset char array after each loop
		memset(pathFile, '\0', maxchars);
		// combile cwd, directory name and room names as well as pid into the file path where the file is to be written
		sprintf(pathFile, "%s/%s/%s", cwd, directoryptr, roomNames[roomNums[i]]);

		// create file
		open(pathFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		// store complete file path to array
		memset(filePaths[i], '\0', maxchars);
		sprintf(filePaths[i], "%s", pathFile);
	}
}

/********************************* Randomized Rooms Requirements ****************************/
// You must hard code a list of ten different Room Names into your rooms program and
// have your rooms program randomly assign one of these to each room generated.
// Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.
// This function serves for two purpose, randomly select 7 rooms out of the 10 available and
// assign room types to the selected rooms
/******************************************************************************************/

void randomRoom(int *array, int size, int max)
{
	int i, k;
	int j = 0;
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
				num = (rand() % max);
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

/********************************* Randomized Rooms Assignments ****************************/
// Assign room types using the random room helper function
/******************************************************************************************/

void assignRoomTypes()
{
	int i;
	// initialize all 3 string arrays with null terminator
	for (i = 0; i < typeCount; i++)
	{
		memset(roomTypes[i], '\0', maxchars);
	}
	// hard code three room types to string arrays
	strcpy(roomTypes[0], "START_ROOM");
	strcpy(roomTypes[1], "END_ROOM");
	strcpy(roomTypes[2], "MID_ROOM");

	// randomized the order of the selected seven rooms
	// this ensure that the rooms assigned to the types are randomized
	memset(roomType, -1, (roomCount + 1));
	randomRoom(roomType, roomCount, roomCount);

	for (i = 0; i < roomCount; i++)
	{
		// assign the first two rooms to start and end rooms
		if (i < 2)
		{
			strcpy(roomAssignedTypes[i], roomTypes[i]);
		}

		// set the rest to mid rooms
		else
		{
			strcpy(roomAssignedTypes[i], roomTypes[2]);
		}
	}
}

/**************************************** Write Room Files *********************************/
// Write data into the room files according the format outlined
// ROOM NAME:
// ROOM TYPE:
// CONNECTIONS:
/******************************************************************************************/

void writeFiles()
{
	int i, j;
	int file_descriptor;
	char room[maxchars] = "ROOM NAME:";
	char type[maxchars] = "ROOM TYPE:";
	char connection[maxchars] = "CONNECTION";
	ssize_t nread, nwritten;
	char readBuffer[2000];

	// loop through all 7 room files
	for (i = 0; i < roomCount; i++)
	{
		// attempt to open file
		file_descriptor = open(filePaths[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		// throw error if unsuccessful
		if (file_descriptor == -1)
		{
			printf("Hull breach - open() failed on \"%s\"\n", filePaths[i]);
			exit(1);
		}

		// concatenate room names into the char array and write into file
		sprintf(room, "%s %s\n", room, roomNames[roomNums[i]]);
		nwritten = write(file_descriptor, room, strlen(room) * sizeof(char));

		// write all the connections for the room
		for (j = 0; j < maxconnections; j++)
		{
			// break loop when an empty spot is found
			if (connections[i][j] == -1)
			{
				break;
			}

			// concatenate room names into the char array and write into file
			sprintf(connection, "%s %d: %s\n", connection, (j + 1), roomNames[roomNums[connections[i][j]]]);
			nwritten = write(file_descriptor, connection, strlen(connection) * sizeof(char));

			// reset char array after each loop
			memset(connection, '\0', maxchars);
			sprintf(connection, "CONNECTION");
		}

		// concatenate room types into the char array and write into file
		sprintf(type, "%s %s\n", type, roomAssignedTypes[i]);
		nwritten = write(file_descriptor, type, strlen(type) * sizeof(char));

		// clear buffer before reading the file
		memset(readBuffer, '\0', sizeof(readBuffer)); // Clear out the array before using it
		lseek(file_descriptor, 0, SEEK_SET);		  // Reset the file pointer to the beginning of the file
		nread = read(file_descriptor, readBuffer, sizeof(readBuffer));
		// printf("File contents:\n%s\n", readBuffer);

		// reset char arrays after each write loop
		memset(room, '\0', maxchars);
		sprintf(room, "ROOM NAME:");
		memset(type, '\0', maxchars);
		sprintf(type, "ROOM TYPE:");
	}
}

/********************************** Check if Graph is Full *******************************/
// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
/******************************************************************************************/

Bool IsGraphFull()
{
	int i, j;

	// loops through all the seven rooms
	for (i = 0; i < roomCount; i++)
	{
		// make sure that at least 3 connections exist in connection array for the room
		// upper bound is handled by the char array size and the CanAddConnectionFrom function below
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

/************************************** Add Room Connection *******************************/
// Adds a random, valid outbound connection from a Room to another Room
/******************************************************************************************/

void AddRandomConnection()
{
	int A;
	int B;
	int i;
	Bool flagA = false;
	Bool flagB = false;
	Bool flagAB = false;

	while (true)
	{
		// get a room randomly
		A = (rand() % roomCount);

		// check if a connection can be added for A (i.e. if it is full)
		flagA = CanAddConnectionFrom(A);

		// if there is a space to add a connection, break from while loop as A
		// is a valid room
		if (flagA == true)
		{
			// printf("A = Room %d selected to add connection \n", A);
			break;
		}
	}

	do
	{
		// get a room randomly
		B = (rand() % roomCount);
		// check if a connection can be added for A (i.e. if it is full)
		flagB = CanAddConnectionFrom(B);
		// check if a connection exist between A and B
		flagAB = ConnectionAlreadyExists(A, B);

		// loop while B selected is full, room A and B are the same room and
		// if a connection exist between the two rooms
	} while (flagB == false || A == B || flagAB == true);

	ConnectRoom(A, B); // TODO: Add this connection to the real variables,
	ConnectRoom(B, A); //  because this A and B will be destroyed when this function terminates
}

/************************************** Can Add Room Connection ? *******************************/
// Returns true if a connection can be added from Room x (< 6 outbound connections),
// false otherwise
/******************************************************************************************/

Bool CanAddConnectionFrom(int x)
{
	int j;

	// loop through connection array of room x and see if a spot exist
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


/*********************************** Does connection exist? *******************************/
// Returns true if a connection from Room x to Room y already exists, false otherwise
/******************************************************************************************/

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


/*********************************** Connect Room *******************************/
// Connects Rooms x and y together, does not check if this connection is valid
/******************************************************************************************/

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

// Reference:
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
// https://www.geeksforgeeks.org/create-directoryfolder-cc-program/
// https://stackoverflow.com/questions/1088622/how-do-i-create-an-array-of-strings-in-c
// http://web.engr.oregonstate.edu/~brewsteb/CS344Slides/2.4%20File%20Access%20in%20C.pdf
// https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
