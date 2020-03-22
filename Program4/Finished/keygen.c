/************************************************
* Author:		Alexander Tir (tira)
* Date:			12/4/2019
* Description:	Generates a key of length specified as an argument to the program.
				The characters generated are the capital letters A-Z and space.
************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//This main function generates the key using a length specified by the user.
int main(int argc, char* argv[])
{
	//seed the random number generator
	srand(time(NULL));

	int i;			//index

	int keyLength;	//key length
	int randChar;	//holds random character

	//check argument count for correct arguments
	if (argc < 2)
	{
		fprintf(stderr, "must specify key length.\n");
		exit(1);
	}

	//get the key length from command line argument
	keyLength = atoi(argv[1]);

	for (i = 0; i < keyLength; i++)
	{
		randChar = rand() % 27;

		//since the space character is not in the same range as A-Z, make a special case
		if (randChar == 26)
		{
			//32 is space char
			randChar = 32;
		}
		else
		{	
			//65-90 is A-Z
			randChar = randChar + 65;
		} 

		//print random char to stdout
		fprintf(stdout, "%c", randChar);

	}

	//append newline to end of key
	fprintf(stdout, "\n");

	return 0;
}