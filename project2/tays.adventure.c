#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st = {0};

int main()
{

	if (stat("/tays.rooms", &st) == -1)
	{
		mkdir("/tays.rooms", 0700);
	}
}

// Reference:
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c