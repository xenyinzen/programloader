#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Wrong parameters.\n");
	} 
	printf("I am a test program %d  %d.\n", atoi(argv[1]), atoi(argv[2]));
	
	return 0;
}
