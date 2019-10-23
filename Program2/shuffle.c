#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ROOMS 10
int main()
{
	srand(time(NULL));

	int i;
	int randomIndex[NUM_ROOMS];
	for (i = 0; i < NUM_ROOMS; i++)
	{
		randomIndex[i] = i;
	}

	int j;
	int temp;
	for (i = 0; i < NUM_ROOMS; i++)
	{
		j = i + rand() % (NUM_ROOMS - i);
		temp = randomIndex[i];
		randomIndex[i] = randomIndex[j];
		randomIndex[j] = temp;
	}

	for (i = 0; i < 10; i++)
	{
		printf("%d\n", randomIndex[i]);
	}
/*
	int i;
	int randomIndex[10];
	for (i = 0; i < 10; i++)
	{
		randomIndex[i] = i;
	}

	int j;
	int temp;
	for (i = 0; i < 10; i++)
	{
		j = i + rand() % (10-i);
		temp = randomIndex[i];
		randomIndex[i] = randomIndex[j];
		randomIndex[j] = temp;
	}

	for (i = 0; i < 10; i++)
	{
		printf("%d\n", randomIndex[i]);
	}*/

	return 0;
}