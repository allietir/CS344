/*




*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define NUM_ROOMS 7
#define MIN_CONNECT 3
#define MAX_CONNECT 6

//C89 does not support the tool type, so will emulate it here
//taken from stackoverflow
typedef enum {
	false,
	true
} bool;

enum roomType {
	START_ROOM,
	MID_ROOM,
	END_ROOM
};

struct Room
{
	int id;
	char* name;
	enum roomType type;
	int connections;
	struct Room* roomConnections[MAX_CONNECT];
};

//initialize an array of 10 hard-coded room names
//decided to make it global since they are "hard-coded"
char *roomNames[10] = {
	"Night",
	"Waterfall",
	"Upside-down",
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
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

	ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
	ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}

// Returns a random Room, does NOT validate if connection can be added
struct Room *GetRandomRoom(struct Room *roomList[])
{
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
	
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(Room x, Room y) 
{

}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(Room x, Room y) 
{

}

void RoomInit()
{

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

int main()
{
	//initialize random seed
	srand(time(NULL));

	

	//initialize an array of pointers to Room structs
	struct Room *roomList[NUM_ROOMS];

	int i;
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

		//"Upside-down" is the longest room name, so max number of characters needed is 12
		//Only allocating memory, not assigning a name yet
		roomList[i]->name = calloc(12, sizeof(char));

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

	// Randomly create all connections in graph
	//while (IsGraphFull(roomList) == false)
	//{
	//	AddRandomConnection(roomList);
	//}

	//struct Room *A;
	//A = GetRandomRoom(roomList);
	//printf("%d\n%s\n", A->id, A->name);
	//roomList[2]->id = 0;
	//printf("%d\n%s\n", A->id, A->name);



	for (i = 0; i < NUM_ROOMS; i++)
	{
		free(roomList[i]);
	}

	return 0;

}