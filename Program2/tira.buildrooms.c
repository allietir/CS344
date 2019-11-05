/************************************************************
Assignment 2
Name: Alexander Tir
onid: tira
Description: This programs build a random path of connected rooms for an
adventure game. The program will output a file for each room that contains
the room name, its connections, and the room type. We were given an outline
of functions to use. These files will be stored in a uniquely named directory.
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NUM_ROOMS 7
#define MIN_CONNECT 3
#define MAX_CONNECT 6

//C89 does not support the tool type, so will emulate it here
//taken from stackoverflow
typedef enum {
	false,
	true
} bool;

//The 3 room types
enum roomType {
	START_ROOM,
	MID_ROOM,
	END_ROOM
};

//Used to store information for a Room
struct Room
{
	int id;
	char* name;
	enum roomType type;
	int connections;
	//I originally implemeneted this as an array of pointers to Room structs, but I realized that
	//only a unique id for each room is necessary to indicate a connection.
	int roomConnections[MAX_CONNECT];
};

//initialize an array of 10 hard-coded room names
//decided to make it global since they are "hard-coded"
char *roomNames[10] = {
	"Night",
	"River",
	"Flipped",
	"Bouncy",
	"Garden",
	"Puppy",
	"Forest",
	"Windy",
	"Comfy",
	"Temple"
};

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull(struct Room *roomList[])  
{	
	int i;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		if (roomList[i]->connections < MIN_CONNECT || roomList[i]->connections > MAX_CONNECT)
		{
			return false;
		}
	}
	return true;
}

// Returns a random Room, does NOT validate if connection can be added
struct Room *GetRandomRoom(struct Room *roomList[])
{
	//i modified this function to return a pointer
	struct Room *random;
	random = roomList[rand() % NUM_ROOMS];
	return random;
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct Room *x) 
{
	if (x->connections < MAX_CONNECT)
	{
		return true;
	}
	return false;
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct Room *x, struct Room *y)
{
	int i;
	//check each room connection for a matching id
	for (i = 0; i < x->connections; i++)
	{
		if(x->roomConnections[i] == y->id)
		{
			return true;
		}
	}
	return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room *x, struct Room *y) 
{
	//the number of connections should indicate the next available index
	x->roomConnections[x->connections] = y->id;
	x->connections++;

	//outbound connections must have matching connections coming back
	y->roomConnections[y->connections] = x->id;
	y->connections++;
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct Room *x, struct Room *y) 
{
	//I think checking id is safest since it should be designed to be unique
	if (x->id == y->id)
	{
		return true;
	}
	return false;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct Room *roomList[])  
{
	struct Room *A;  // Maybe a struct, maybe global arrays of ints
	struct Room *B;

  	while(true)
  	{
		A = GetRandomRoom(roomList);

		if (CanAddConnectionFrom(A) == true)
			break;
  	}

	do
	{
		B = GetRandomRoom(roomList);
	}
	//perform all the checks before connecting rooms
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

	ConnectRoom(A, B);
}

//This function randomly assigns rooms using the first 7 elements of a shuffled array
void AssignRandomRoom(struct Room *roomList[])
{
	int i;

	//first, generate an array of ints from 0 to 10
	int randomIndex[10];
	for (i = 0; i < 10; i++)
	{
		randomIndex[i] = i;
	}

	int j;
	int temp;
	//next, shuffle the array by performing random swaps
	for (i = 0; i < 10; i++)
	{
		j = i + rand() % (10 - i);
		temp = randomIndex[i];
		randomIndex[i] = randomIndex[j];
		randomIndex[j] = temp;
	}

	//finally, randomly assign room names using the first 7 elements in the shuffled array
	for (i = 0; i < NUM_ROOMS; i++)
	{
		strcpy(roomList[i]->name, roomNames[randomIndex[i]]);
		//printf("%s\n", roomList[i]->name);
	}
}

//The main function. The array of pointers to Room structs is allocated here.
//Rooms are randomly assigned using AssignRandomRoom() and connections are then
//randomly added to each room until they are considered full.
//Then, each room's information is stored in a file in a directory created using
//the prefix "tira.buildrooms." + the process id.
int main()
{
	int i;
	int j;
	int k;

	//initialize random seed
	srand(time(NULL));

	//initialize an array of pointers to Room structs
	struct Room *roomList[NUM_ROOMS];

	//allocate memory for each Room in roomList
	for (i = 0; i < NUM_ROOMS; i++)
	{
		//allocate memory for each Room
		roomList[i] = (struct Room*) malloc(sizeof(struct Room));

		//Personal note to help me remember concept: since we are referring to a member of a struct
		//through an address of that struct, we use the arrow operator instead of the dot operator.
		//I always forget this!

		//assign id
		roomList[i]->id = i;

		//Only allocating memory, not assigning a name yet
		roomList[i]->name = calloc(16, sizeof(char));

		//starts at 0, will be incremented when a connection is added
		roomList[i]->connections = 0;

		//Guarantees one START_ROOM and one END_ROOM
		if (i == 0)
		{
			roomList[i]->type = START_ROOM;
		}
		else if (i == NUM_ROOMS - 1)
		{
			roomList[i]->type = END_ROOM;
		}
		else
		{
			roomList[i]->type = MID_ROOM;
		}
	}

	//call function to randomly assign rooms
	AssignRandomRoom(roomList);

	//Randomly create all connections in graph
	while (IsGraphFull(roomList) == false)
	{
		AddRandomConnection(roomList);
	}

	//get process id
	int pid = getpid();
	//define prefix for directory name
	char *prefix = "tira.buildrooms.";
	//create char buffer
	char dirName[32];
	//use sprintf to concat dirName
	sprintf(dirName, "%s%d/", prefix, pid);

	//use stat to check if directory exists first
	struct stat st = {0};
	if(stat(dirName, &st) == -1)
	{
		mkdir(dirName, 0777);
	}

	//initialize filepointer and fileName string
	FILE *fp = NULL;
	char fileName[64];

	//Create files for each room, process one Room at a time
	for (i = 0; i < NUM_ROOMS; i++)
	{
		//following the "name_room" format
		sprintf(fileName, "%s%s_room", dirName, roomList[i]->name);
		fp = fopen(fileName, "w+");

		if (fp == NULL)
		{
			perror("Error opening file: ");
		}

		//write room name to file
		fprintf(fp, "ROOM NAME: %s\n", roomList[i]->name);

		//write connections to file
		for (j = 0; j < roomList[i]->connections; j++)
		{
			//this isn't an ideal way of doing this, but iterate through the room connections until
			//a match is found, then use the matching index print the name of that room
			for (k = 0; k < NUM_ROOMS; k++)
			{
				if (roomList[i]->roomConnections[j] == roomList[k]->id)
				{
					fprintf(fp, "CONNECTION %d: %s\n", j + 1, roomList[k]->name);
				}
			}
		}

		//since I implemented them as enumerated data types, I'll have to use these multiple if statements
		//for the room type...
		if (roomList[i]->type == START_ROOM)
		{
			fprintf(fp, "ROOM TYPE: START_ROOM\n");
		}
		else if (roomList[i]->type == END_ROOM)
		{
			fprintf(fp, "ROOM TYPE: END_ROOM\n");
		}
		else
		{
			fprintf(fp, "ROOM TYPE: MID_ROOM\n");
		}
	}

	//clean up allocated memory
	for (i = 0; i < NUM_ROOMS; i++)
	{
		free(roomList[i]);
	}

	return 0;
}