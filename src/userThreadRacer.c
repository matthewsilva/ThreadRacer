/*	Author:	Matthew Silva
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
#include <sys/time.h>
#include <stdint.h>
//
// TAKEN FROM tiff.h TIFF LIBRARY
#define TIFFGetR(abgr)   ((abgr) & 0xff)
#define TIFFGetG(abgr)   (((abgr) >> 8) & 0xff)
#define TIFFGetB(abgr)   (((abgr) >> 16) & 0xff)
#define TIFFGetA(abgr)   (((abgr) >> 24) & 0xff)

enum DifficultyLevel {Easy, Medium, Hard};
typedef enum Direction {UP, RIGHT, DOWN, LEFT};

typedef struct ThreadInfo
{
	// Order in which thread was created
	int threadIndex;
	
	int* arr;
	int arrSize;
	int row;
	int col;
	int movesLeft;
	pthread_barrier_t* raceBarrier;
	float execTime;
	enum Direction direction;
	enum DifficultyLevel diff;
	
} ThreadInfo;

typedef struct UserThreadInfo
{
	// Order in which thread was created
	int threadIndex;
	
	int* arr;
	int arrSize;
	int row;
	int col;
	int movesLeft;
	pthread_barrier_t* raceBarrier;
	float execTime;
	enum Direction direction;
	
} UserThreadInfo;

typedef struct KeyboardThreadInfo
{
	// Order in which thread was created
	int threadIndex;
	
	ThreadInfo* thread;
	UserThreadInfo* userThread;
	pthread_mutex_t* directionLock;
	pthread_mutex_t* userDirectionLock;
	
} KeyboardThreadInfo;

uint64_t get_posix_clock_time ()
{
    struct timespec ts;

    if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
    else
        return 0;
}

// PROTOTYPES ------------------------------------------------------------

void* threadFunc(void* arg);

void* userThreadFunc(void* arg);

void* keyboardThreadFunc(void* arg);

void swap(int* arr, int index1, int index2);

void sort(int* arr, int arrLen);

void moveThread(void* info, int numRows, int numCols, int trackTop, int trackBottom, int trackLeft, int trackRight);

int same(int* arr1, int* arr2, int arrLen);

enum Direction randDirection(void);

int isValidMove(int row, int col, enum Direction direction, int numRows, int numCols, 
				int trackTop, int trackBottom, int trackLeft, int trackRight);

int isInside(int row, int col, int numRows, int numCols, int trackTop, int trackBottom, int trackLeft, int trackRight);

void moveThread(void* threadArg, int numRows, int numCols, int trackTop, int trackBottom, int trackLeft, int trackRight);

int getProgress(int topFinishLine, int leftFinishLine, int bottomFinishLine, int rightFinishLine);

void userSort(int* arr, int arrLen);

//------------------------------------------------------------------------


// THREAD DESIGN:
// Threads keep track of their own movement and array correctness locally
// All the main thread does is synchronize their movements, render the grid, and supply new data


// Array of threads to create the difference image
pthread_t* thread;
// Array of parameter structs for the threadFunc
ThreadInfo* threadInfo;
	
// Barrier to synchronize the computation of the difference image by threads
pthread_barrier_t diffBarrier;

static int ARR_SIZE = 1000;

char* grid;

int main(int argc, char** argv)
{
	int diffNum;
	
	
	sscanf(argv[1], "%d", &diffNum);
	
	enum DifficultyLevel difficulty;
	
	switch(diffNum)
	{
		case(1):
			difficulty = Easy;
			break;
		case(2):
			difficulty = Medium;
			break;
		case(3):
			difficulty = Hard;
			break;
	}
	
	int timedout = 0;
	
	// TODO Set up timeoutThread
	
	pthread_barrier_t raceBarrier;
	pthread_barrier_init(&raceBarrier, NULL, 3);
	
	ThreadInfo params;
	UserThreadInfo userParams;
	KeyboardThreadInfo keyboardParams;
			
	
	int* arr = (int*) malloc(ARR_SIZE*sizeof(int));
	int* userArr = (int*) malloc(ARR_SIZE*sizeof(int));
	int* ansArr = (int*) malloc(ARR_SIZE*sizeof(int));
	
	initscr();
	clear();
	refresh();
	int numRows,numCols;
    getmaxyx(stdscr,numRows,numCols);
    
    
    
    // ROW MAJOR ORDER
    grid = (char*) malloc(numRows*numCols*sizeof(int*));
	
	params.arr = arr;
	userParams.arr = userArr;
	params.arrSize = userParams.arrSize = ARR_SIZE;
	params.raceBarrier = userParams.raceBarrier = &raceBarrier;
	
	params.diff = difficulty;
	
	// TODO Create 2nd keyboardThread that outputs into a 2nd keyPress variable
    char keyPress = '~';
    keyboardParams.thread = &params;
    keyboardParams.userThread = &userParams;    
	
	pthread_mutex_t directionLock;
    pthread_mutex_init(&directionLock, NULL);
	pthread_mutex_t userDirectionLock; 
    pthread_mutex_init(&userDirectionLock, NULL);
    
   	keyboardParams.directionLock = &directionLock;
   	keyboardParams.userDirectionLock = &userDirectionLock;
	
	pthread_t thread;
	pthread_t userThread;
	pthread_t keyboardThread;
	
	pthread_create(&thread, NULL, threadFunc, &params);
	pthread_create(&userThread, NULL, userThreadFunc, &userParams);
	pthread_create(&keyboardThread, NULL, keyboardThreadFunc, &keyboardParams);
	
/*    initscr();
    int row,col;				
	refresh();
	printw("THREAD PROGRESS:");
		
	for (int i = 0; i < 10; i++) {
        printw("#");
    }
    for (int i = 0; i < 0; i++) {
        printw("=");
    }
    
	printw("\n");
    printw("USER THREAD PROGRESS:");
    for (int i = 0; i < 10; i++) {
        printw("#");
    }
    for (int i = 0; i < 0; i++) {
        printw("=");
    }
	getch(); 
*/	char* screen = (char*) malloc(10000*sizeof(char));
	initscr();
	clear();
	
	
	char* message = (char*) malloc(1000*sizeof(char));
	char* nextLine = (char*) malloc(1000*sizeof(char));
	
	noecho();
	//keypad(stdstr, TRUE)
	
	refresh();
	printw("\n _____ _                        _  ______                    ");
	printw("\n|_   _| |                      | | | ___ \\                   ");              
	printw("\n  | | | |__  _ __ ___  __ _  __| | | |_/ /__ _  ___ ___ _ __ ");
	printw("\n  | | | '_ \\| '__/ _ \\/ _` |/ _` | |    // _` |/ __/ _ \\ '__|");
	printw("\n  | | | | | | | |  __/ (_| | (_| | | |\\ \\ (_| | (_|  __/ |   ");
	printw("\n  \\_/ |_| |_|_|  \\___|\\__,_|\\__,_| \\_| \\_\\__,_|\\___\\___|_|   ");
	                                                     
	printw("\n");
	for (int i = 0; i < numRows-8; i++) {
		for (int k = 0; k < numCols/2; k++) {
			if (i%2 == 0) {
				printw("-");
				printw("+");
			}
			if (i%2 != 0) {
				printw("+");
				printw("-");
			}
		}
		printw("\n");
	}
	refresh();
	
	
	sleep(2);
	clear();
	refresh();
	
	int gameRows = numRows-8;
	int gameCols = numCols;
	
	int finishLine1;
    
    int trackTop = gameRows/4;
    int trackBottom = 3*(gameRows/4);
    int trackLeft = gameCols/4;
    int trackRight = 3*(gameCols/4);
    
    int horizontalFinishLines = gameRows/2;
    int verticalFinishLines = gameCols/2;
    
    int topFinishLine = 0;
    int leftFinishLine = 0;
    int bottomFinishLine = 0;
    int rightFinishLine = 0;
    
    int userTopFinishLine = 0;
    int userLeftFinishLine = 0;
    int userBottomFinishLine = 0;
    int userRightFinishLine = 0;
	
	int gameFinished = 0;
	int threadWon = 0;
	int userWon = 0;
	while (gameFinished != 1) {

		// Create a new random array and generate the answer, storing both inside params and userParams
		for (int i = 0; i < ARR_SIZE; i++) {
			arr[i] = userArr[i] = ansArr[i] = rand()%5000;
		}
		// ALREADY STORED INSIDE PARAMS AND USERPARAMS SINCE ITS A POINTER
		
		sort(ansArr, ARR_SIZE);//TODO
		
		// Tell threads to do one computation
		pthread_barrier_wait(&raceBarrier);
		// Wait for threads to finish computation
		pthread_barrier_wait(&raceBarrier);
		
		if (same(arr, ansArr, ARR_SIZE) != 1) {
			clear();
			refresh();
			printw("Thread code did not correctly sort the array. Exiting program, sorry.");
		    printw("\n\n\n");
			printw("\nPress any key to exit the game...");
			getch();
			endwin();
		}
		if (same(userArr, ansArr, ARR_SIZE) != 1) {
			clear();
			refresh();
			printw("Your (user) thread code did not correctly sort the array. Exiting program, try again.");
		    printw("\n\n\n");
			printw("\nPress any key to exit the game...");
			getch();
			endwin();
		}
		
		// TODO change this to something that makes the thread 2x faster for being 2x faster
		// BUT ONly AFTER I'VE ADDED MERGESORT OR SOME OTHER FUNCTION THAT WILL ACTUALLY BE 2x FASTER
		float speedup = params.execTime / userParams.execTime;
		//float pctFaster = speedup*100.0f - 100.f;
		int moves = (5.0f*speedup);
		if (moves < 5) {
			moves = 5;
		}
		
		params.movesLeft = 5;
		userParams.movesLeft = moves;
		
		while (params.movesLeft > 0 || userParams.movesLeft > 0) {
			
			pthread_mutex_lock(&directionLock);
			moveThread(&params, gameRows, gameCols, trackTop, trackBottom, trackLeft, trackRight);
			pthread_mutex_unlock(&directionLock);
			
			if (params.col == verticalFinishLines && params.row > -1 && params.row < trackTop) {
				topFinishLine = 1;
			}
			else if (params.row == horizontalFinishLines && params.col > -1 && params.col < trackLeft) {
				leftFinishLine = 1;
			}
			else if (params.col == verticalFinishLines && params.row >= trackBottom && params.row < gameRows) {
				bottomFinishLine = 1;
			}
			else if (params.row == horizontalFinishLines && params.col >= trackRight && params.row < gameCols) {
				rightFinishLine = 1;
			}
			
			pthread_mutex_lock(&userDirectionLock);
			moveThread(&userParams, gameRows, gameCols, trackTop, trackBottom, trackLeft, trackRight);
			pthread_mutex_unlock(&userDirectionLock);
			
			if (userParams.col == verticalFinishLines && userParams.row > -1 && userParams.row < trackTop) {
				userTopFinishLine = 1;
			}
			else if (userParams.row == horizontalFinishLines && userParams.col > -1 && userParams.col < trackLeft) {
				userLeftFinishLine = 1;
			}
			else if (userParams.col == verticalFinishLines && userParams.row >= trackBottom && userParams.row < gameRows) {
				userBottomFinishLine = 1;
			}
			else if (userParams.row == horizontalFinishLines && userParams.col >= trackRight && userParams.row < gameCols) {
				userRightFinishLine = 1;
			}
			
			// TODO
			//
			// DISPLAY PCT FASTER ON TOP ROW, must subtract 1 from screen size to find numRows and 
			// then do a manual print of the top row every frame here
		
			strcpy(screen, "");
		
			clear();
			
			
			sprintf(nextLine,"Thread Row = %d, Thread Col = %d, Thread Moves Left = %d",params.row, params.col, params.movesLeft);
			strcpy(message,nextLine);
			sprintf(nextLine,"\nU Thread Row = %d, U Thread Col = %d, U Thread Moves Left = %d", userParams.row, userParams.col, userParams.movesLeft);
			strcat(message,nextLine);
			sprintf(nextLine,"\nThread Exec Time = %f, U Thread Exec Time = %f, speedup = %f", params.execTime, userParams.execTime, speedup);
			strcat(message,nextLine);
			//vv CAUSES GAME TO FREEZE================================================================================
			sprintf(nextLine,"\n     Thread Lap Progress:");
			int threadProgBarLen = gameCols - strlen(nextLine);
			int threadProgress = getProgress(topFinishLine, leftFinishLine, bottomFinishLine, rightFinishLine);
			if (threadProgress == 4) {
				gameFinished = 1;
				threadWon = 1;
			}
			for (int i = 0; i < ((float) threadProgress / 4.0f)*(float) threadProgBarLen; i++) {
				if (i+1 >= ((float) threadProgress / 4.0f)*(float) threadProgBarLen) {
					strcat(nextLine, ">");
				}
				else {
					strcat(nextLine, "=");
				}
			}
			strcat(message,nextLine);
			
			sprintf(nextLine,"\nUser Thread Lap Progress:");
			int userThreadProgBarLen = gameCols - strlen(nextLine);
			int userThreadProgress = getProgress(userTopFinishLine, userLeftFinishLine, userBottomFinishLine, userRightFinishLine);
			if (userThreadProgress == 4) {
				gameFinished = 1;
				userWon = 1;
			}
			for (int i = 0; i < ((float) userThreadProgress / 4.0f)*(float) userThreadProgBarLen; i++) {
				if (i+1 >= ((float) userThreadProgress / 4.0f)*(float) userThreadProgBarLen) {
					strcat(nextLine, ">");
				}
				else {
					strcat(nextLine, "=");
				}
			}
			strcat(message,nextLine);
			
			strcat(screen, message);
			strcat(screen,"\n\n");
			for (int i = 0; i < gameCols; i++) {
				strcat(screen, "+");
			}
			strcat(screen,"\n");
			
			//^^ CAUSES GAME TO FREEZE================================================================================
			
	 		for (int i = 0; i < gameRows; i++) {
	 			for (int k = 0; k < gameCols; k++) {
	 				if (i > trackTop && i < trackBottom && k > trackLeft && k < trackRight) {
	 					strcat(screen, "X");
	 				}
	 				else if (i == params.row && k == params.col) {
	 					strcat(screen, "T");
	 				}
	 				else if (i == userParams.row && k == userParams.col) {
	 					strcat(screen, "U");
	 				} 
	 				else if (i == horizontalFinishLines && (k <= trackLeft || k >= trackRight)) {
	 					strcat(screen, "-");
	 				}
	 				else if (k == verticalFinishLines && ( i <= trackTop || i >= trackBottom)) {
	 					strcat(screen, "|");
	 				}
	 				else {
	 					strcat(screen, ",");
	 				}
	 			}
	 			printw("\n");
	 		}
	 		
			move(0,0);
			printw(screen);
			refresh();
		
			usleep(100000);
		}
		


	
	}
	
	clear();
	move(0,0);
	
	if (threadWon == 1 && userWon == 1) {
		printw("\n\nTIE GAME! Try again!");
	}
	else if (threadWon == 1) {
		printw("\n\nUSER LOST! Try again!");
	}
	else if (userWon == 1) {
		printw("\n\nUSER WON! Good job!");
	}
	
	refresh();
	
	/*
    if (timedout == 0) {
        if (userParams.execTime < params.execTime) {
            printw("Your implementation was %f seconds faster. Good job!", params.execTime - userParams.execTime);
        }
        else {
            printw("Your implementation was %f seconds worse. Try again!", userParams.execTime - params.execTime);
        }
    }
	
    else {
        printw("Your code did not finish executing. Exiting program, try again.");
        printw("\n\n\n");
		printw("\nPress any key to exit the game...");
		getch();
		endwin();

    }
    */
    
    
    // TODO pthread_join(
	
	printw("\n\n\n");
	printw("\nPress any key to exit the game...");
	getch();
	endwin();
	return 0;
	
}

void swap(int* arr, int index1, int index2) {
	int temp = arr[index1];
	arr[index1] = arr[index2];
	arr[index2] = temp;
}

// Implement merge sort using cool C pointer arithmetic stuff passing calculated array lengths to simulate passing actual smaller arrays
void sort(int* arr, int arrLen) {
	
	for (int numProcessed = 0; numProcessed < arrLen; numProcessed++) {
		int min = arr[numProcessed];
		int minIndex = numProcessed;	
		for (int i = numProcessed; i < arrLen; i++) {
			if (arr[i] < min) {
				min = arr[i];
				minIndex = i;
			}
		}
		swap(arr, minIndex, numProcessed);
	}
}

void merge(int* arr, int* split1Arr, int split1Len, int* split2Arr, int split2Len) {
	int split1Counter = 0;
	int split2Counter = 0;
	for (int i = 0; i < split1Len + split2Len; i++) {
		if (split1Counter == split1Len) {
			arr[i] = split2Arr[split2Counter];
			split2Counter++;
		}
		else if (split2Counter == split2Len) {
			arr[i] = split1Arr[split1Counter];
			split1Counter++;
		}
		
		else if (split1Arr[split1Counter] <= split2Arr[split2Counter]) {
			arr[i] = split1Arr[split1Counter];
			split1Counter++;
		}
		else {
			arr[i] = split2Arr[split2Counter];
			split2Counter++;
		}
	}
}

void mergeSort(int* arr, int arrLen) {
	if (arrLen == 1) {
		return;
	}
	
	// Create a copy for the halves
	int* copyArr = (int*) malloc(arrLen*sizeof(int));
	memcpy(copyArr, arr, arrLen*sizeof(int));
	
	// Split the array
	int* arr1 = copyArr;
	int arr1Len = arrLen / 2;
	int* arr2 = copyArr + arr1Len;
	int arr2Len = arrLen - arr1Len;
	
	// Sort each split array
	mergeSort(arr1, arr1Len);
	mergeSort(arr2, arr2Len);
	
	// Merge them in order
	merge(arr, arr1, arr1Len, arr2, arr2Len);
	
	free(copyArr);
	/*
		EXAMPLE
		====================
		arr = 0
		arrLen = 5
		arr1 = 0
		arr1Len = 2
		arr2 = 2
		arr2Len = 3
		
		EXAMPLE
		====================
		arr = 0
		arrLen = 2
		arr1 = 0
		arr1Len = 1
		arr2 = 1
		arr2Len = 1
	*/
}

int same(int* arr1, int* arr2, int arrLen) {
	for (int i = 0; i < arrLen; i++) {
		if (arr1[i] != arr2[i]) {
			return 0;
		}
	}
	return 1;
}

// Threads use this function to compute the part of the difference image they were assigned to compute
void* threadFunc(void* arg) {
	// Cast the void* parameter struct for actual use
	ThreadInfo* info = ((ThreadInfo*)arg);
	
	info->row = 1;
	info->col = 1;
	
	info->direction = DOWN;
	int speed;
	
	uint64_t prev_time_value, time_value;
	uint64_t time_diff;
	
	while (1) {
		// Wait for signal to perform 1 operation
		pthread_barrier_wait(info->raceBarrier);

		/* Initial time */
		prev_time_value = get_posix_clock_time ();
		
		sort(info->arr, info->arrSize);
		
		/* Final time */
		time_value = get_posix_clock_time ();

		/* Time difference */
		time_diff = time_value - prev_time_value;
		info->execTime = (float) time_diff / 1000000.0f;
	
	
	//	double progress = checkProgress(info->arr, info->ansArr, &(info->doneWithArr));
	//	speed = 1+(progress/25.0);
	//	move(info, speed, direction);
		
		// Signal that we are done with our sort operation and move
		pthread_barrier_wait(info->raceBarrier);
		
	}
	
	// Won't ever get here
	exit(1);
	
	
}

// Threads use this function to compute the part of the difference image they were assigned to compute
void* userThreadFunc(void* arg) {
	// Cast the void* parameter struct for actual use
	UserThreadInfo* info = ((UserThreadInfo*)arg);
	
	info->row = 5;
	info->col = 5;
	
	info->direction = DOWN;
	
	uint64_t prev_time_value, time_value;
	uint64_t time_diff;
	
	while (1) {
		// Wait for signal to perform 1 operation
		pthread_barrier_wait(info->raceBarrier);
		
		/* Initial time */
		prev_time_value = get_posix_clock_time ();
		
		userSort(info->arr, info->arrSize);
		
		/* Final time */
		time_value = get_posix_clock_time ();

		/* Time difference */
		time_diff = time_value - prev_time_value;
		info->execTime = (float) time_diff / 1000000.0f;
	
	
	//	double progress = checkProgress(info->arr, info->ansArr, &(info->doneWithArr));
	//	speed = 1+(progress/25.0);
	//	move(info, speed, direction);
		
		// Signal that we are done with our sort operation and move
		pthread_barrier_wait(info->raceBarrier);
		
	}
	
	// Won't ever get here
	exit(1);
	
	
}

enum Direction randDirection(void) {
	int dirNum = rand()%4;
	enum Direction dir;
	switch(dirNum)
	{
		case(0):
			dir = UP;
			return dir;
		case(1):
			dir = RIGHT;
			return dir;
		case(2):
			dir = DOWN;
			return dir;
		case(3):
			dir = LEFT;
			return dir;
	}
	exit(1);
}

int isInside(int row, int col, int numRows, int numCols, int trackTop, int trackBottom, int trackLeft, int trackRight) {
	if (row > -1 && row < numRows && col > -1 && col < numCols) {
		if (row > trackTop && row < trackBottom && col > trackLeft && col < trackRight) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		return 0;
	}
}

int isValidMove(int row, int col, enum Direction direction, int numRows, int numCols, 
				int trackTop, int trackBottom, int trackLeft, int trackRight) {
				
	switch(direction)
	{
		case(UP):
			return isInside(row-1, col, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight);
			break;
		
		case(RIGHT):
			return isInside(row, col+1, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight);
			break;
			
		case(DOWN):
			return isInside(row+1, col, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight);
			break;
			
		case(LEFT):
			return isInside(row, col-1, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight);
			break;
	}
	exit(1);
}


void moveThread(void* threadArg, int numRows, int numCols, int trackTop, int trackBottom, int trackLeft, int trackRight) {
	ThreadInfo* info = ((ThreadInfo*)threadArg);
	if (info->movesLeft > 0) {
		switch(info->direction)
		{
			case(UP):
				if (isValidMove(info->row, info->col, info->direction, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight) == 1) {
					info->row--;
					info->movesLeft--;
				}
				else {
					info->movesLeft=0;
				}
				break;
		
			
			case(RIGHT):
				if (isValidMove(info->row, info->col, info->direction, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight) == 1) {
					info->col++;
					info->movesLeft--;
				}
				else {
					info->movesLeft=0;
				}
				break;
		
			
			case(DOWN):
				if (isValidMove(info->row, info->col, info->direction, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight) == 1) {
					info->row++;
					info->movesLeft--;
				}
				else {
					info->movesLeft=0;
				}
				break;
		
			
			case(LEFT):
				if (isValidMove(info->row, info->col, info->direction, numRows, numCols, trackTop, trackBottom, trackLeft, trackRight) == 1) {
					info->col--;
					info->movesLeft--;
				}
				else {
					info->movesLeft=0;
				}
				break;
			
		}
	}
	
}

int getProgress(int topFinishLine, int leftFinishLine, int bottomFinishLine, int rightFinishLine) {
	int progress = 0;
	
	if (topFinishLine == 1){
		progress++;
	}
	if (leftFinishLine == 1){
		progress++;
	}
	if (bottomFinishLine == 1){
		progress++;
	}
	if (rightFinishLine == 1){
		progress++;
	}
	
	return progress;
} 

// TODO Add 2nd thread for 2nd player??? Nope
//
// TODO ADD MUTEX for direction and make change immediately to prevent long moves before dir change

void* keyboardThreadFunc(void* arg) {
	// Cast the void* parameter struct for actual use
	KeyboardThreadInfo info = *((KeyboardThreadInfo*)arg);
	
	char keyPress;
	while(1) {
		keyPress = getch();
		if (keyPress == 'w') {
			pthread_mutex_lock(info.directionLock);
			info.thread->direction = UP;
			pthread_mutex_unlock(info.directionLock);
		}
		else if (keyPress == 'a') {
			pthread_mutex_lock(info.directionLock);
			info.thread->direction = LEFT;
			pthread_mutex_unlock(info.directionLock);
		}
		else if (keyPress == 's') {
			pthread_mutex_lock(info.directionLock);
			info.thread->direction = DOWN;
			pthread_mutex_unlock(info.directionLock);
		}
		else if (keyPress == 'd') {
			pthread_mutex_lock(info.directionLock);
			info.thread->direction = RIGHT;
			pthread_mutex_unlock(info.directionLock);
		}
		
		else if (keyPress == '8') {
			pthread_mutex_lock(info.userDirectionLock);
			info.userThread->direction = UP;
			pthread_mutex_unlock(info.userDirectionLock);
		}
		else if (keyPress == '4') {
			pthread_mutex_lock(info.userDirectionLock);
			info.userThread->direction = LEFT;
			pthread_mutex_unlock(info.userDirectionLock);
		}
		else if (keyPress == '5') {
			pthread_mutex_lock(info.userDirectionLock);
			info.userThread->direction = DOWN;
			pthread_mutex_unlock(info.userDirectionLock);
		}
		else if (keyPress == '6') {
			pthread_mutex_lock(info.userDirectionLock);
			info.userThread->direction = RIGHT;
			pthread_mutex_unlock(info.userDirectionLock);
		}
		
	}
	exit(1);
}


	
// Please fill in a function that will sort arr from smallest to largest

// In/Out: arr (array of positive integers from 0 to 5000)
// In:	arrSize (How many integers are in this array

void userSort(int* arr, int arrSize) {
	sort(arr,arrSize);

}
