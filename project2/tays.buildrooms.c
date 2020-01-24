#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void createRooms();


struct stat st = {0};

/********************************* Room name requirements *********************************/
// A room name cannot be assigned to more than one room.
// Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed
// (thus, no numbers, special characters, or spaces).
// This restriction is not extended to the room file's filename.
// You must hard code a list of ten different Room Names into your rooms program and
// have your rooms program randomly assign one of these to each room generated.
// Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.

char roomNames[10][9];



int main(void)
{
	int i;

	createRooms();

	if (stat("tays.rooms", &st) == -1)
	{
		mkdir("tays.rooms", 0700);
		printf("Directory created!.");
	}

	else
	{
		printf("Directory exists! Failed to create.");
	}

	// for (i = 0; i < 10; i++)
	// {
	// 	printf("String %d : %s \n", i, roomNames[i]);
	// }

	return 0;
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
	char *directory = "./tays.rooms";


	// create 10 files with hardcoded strings
	for (i = 0; i < 10; i++)
	{
		// int res = sprintf(readBuffer, "test%s.txt", roomNames[i]);
		char pathFile[50];
		sprintf(pathFile, "%s.%s", directory, roomNames[i]);
		// sprintf(pathFile, "%s.%s", directory, roomNames[i]);
		printf("%s", pathFile);

		// open(roomNames[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	}




}

// Reference:
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
// https://www.geeksforgeeks.org/create-directoryfolder-cc-program/
// https://stackoverflow.com/questions/1088622/how-do-i-create-an-array-of-strings-in-c
// http://web.engr.oregonstate.edu/~brewsteb/CS344Slides/2.4%20File%20Access%20in%20C.pdf