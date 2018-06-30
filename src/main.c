/*	Author:	Matthew Silva
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//


int main(int argc, char** argv)
{
	printf("\n████████╗██╗  ██╗██████╗ ███████╗ █████╗ ██████╗    ██████╗  █████╗  ██████╗███████╗██████╗");
    printf("\n╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗██╔══██╗   ██╔══██╗██╔══██╗██╔════╝██╔════╝██╔══██╗");
    printf("\n   ██║   ███████║██████╔╝█████╗  ███████║██║  ██║   ██████╔╝███████║██║     █████╗  ██████╔╝");
    printf("\n   ██║   ██╔══██║██╔══██╗██╔══╝  ██╔══██║██║  ██║   ██╔══██╗██╔══██║██║     ██╔══╝  ██╔══██╗");
    printf("\n   ██║   ██║  ██║██║  ██║███████╗██║  ██║██████╔╝   ██║  ██║██║  ██║╚██████╗███████╗██║  ██║");
    printf("\n   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝    ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚═╝  ╚═╝");
	
	printf("\n\n----------------------------------------------------------------------------------------------\n");
	
	printf("\nWelcome to Thread Racer!");
	
	printf("\n\n Please select a difficulty level (1=Easy,2=Medium,3=Hard):");
	
	int diffNum;
	scanf("%d", &diffNum);
	//diffNum = 1;
	
	// TODO Add the stuff to the ThreadRacer file
	
	system("cp userSortSample.c userSort.c");
  
	printf("\nPlease select a text editor to write your implementation of the sorting algorithm");
	printf("\nEditor('vi', 'nano', or 'mousepad'");

	char* command = (char*) malloc(256*sizeof(char));
	char* editor = (char*) malloc(50*sizeof(char));
	scanf("%s", editor);
	sprintf(command, "%s userSort.c", editor);
	system(command);

	printf("\nEnter 'DONE' to continue once you are done editing the text file.");
	char* doneMsg = (char*) malloc(50*sizeof(char));
	scanf("%s", doneMsg);
	while (strcmp(doneMsg, "DONE") != 0) {
		printf("Please try again...");
		scanf("%s", doneMsg);
	}

	FILE *sortFile;
	char* buffer = (char*) malloc(1000*sizeof(char));
	sortFile = fopen("userSort.c", "r");
	char* c = (char*) malloc(1*sizeof(char));
	*c = fgetc(sortFile);
	strcpy(buffer, "");
	while (*c != EOF) {
		strcat(buffer, c);
		*c = fgetc(sortFile);
		
	}
	fclose(sortFile);
	
	FILE *racerFile;
	system("cp threadracer.c userThreadRacer.c");
	racerFile = fopen("userThreadRacer.c", "a");
	
	fprintf(racerFile, buffer);



	fclose(racerFile);
	
	system("chmod +x racer.sh");
	
	
	sprintf(command, "sh racer.sh %d", diffNum);
	system(command);
	
	return 0;
}













