/************************************************************
Program 3 
Name: Alexander Tir
onid: tira
Description: This is a custom shell written in C that is similar to bash.
Supported built-in commands are cd, status, and exit.
This program uses a while loop to run the shell until the user exits.
When commands are executed, the user can choose to run processes in the background
or the foreground.
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LENGTH 2048		//Max supported length for command lines
#define MAX_ARGUMENTS 512	//Max support number of arguments

//Function prototypes
void exitStatus(int);
void catchSIGTSTP(int);
void catchSIGINT(int);

//global variable, determines whether process is foreground-only mode
volatile sig_atomic_t foregroundOnly = 0;


/**************************************
This function causes SIGINT to also print the terminating signal
***************************************/
int main()
{
	int pid;			
	int running = 1;		//execution flag for the do-while loop
	int status = 0;
	int inFile = -1;
	int outFile = -1;
	int i, j;
	int parseIndex = 0;
	int background;			//flag to run process in background
	pid_t spawnpid = -5;	//pid for spawned processes

	//String variables
	char *token;
	char *inputFile = NULL;
	char *outputFile = NULL;
	char userInput[MAX_LENGTH];
	memset(userInput, '\0', MAX_LENGTH);
	char *parsedInput[MAX_ARGUMENTS];
	for (i = 0; i < MAX_ARGUMENTS; i++)
	{
		parsedInput[i] = NULL;
	}

	//Initialize and modify signal handlers for ctrl+z and ctrl+c
	//Taken from lecture 3.3 on signals

	//Handler for ^C
	struct sigaction SIGINT_action = {0};		//initialize empty sigaction struct
	SIGINT_action.sa_handler = SIG_IGN;			//this causes ^C signal to be ignored
	SIGINT_action.sa_flags = SA_RESTART;		//restart system call after signal is handled
	sigfillset(&SIGINT_action.sa_mask);			//block signal while the handler is executing
	sigaction(SIGINT, &SIGINT_action, NULL);	//register the handling function SIGINT signal

	//Handler for ^Z
	struct sigaction SIGTSTP_action = {0};		//initialize empty sigaction struct
	SIGTSTP_action.sa_handler = catchSIGTSTP;	//redirect ^Z to catchSIGHTSTP function
	SIGTSTP_action.sa_flags = SA_RESTART;		//restart system call after signal is handled
	sigfillset(&SIGTSTP_action.sa_mask);		//block signal while the handler is executing
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);	//register the handling function for SIGTSTP signal


	/**************************************
			SHELL LOOP	
	***************************************/
	//The most active part of our shell, this loop repeats until exited	
	do{
		//Reset background process flag
		background = 0;

		//Check if any process has completed, will return 0 if none have
		//Placed before prompt so output is closer to assignment specs, but this could also be placed at the end of the loop.
		//Taken from Lecture 3.1
		while((spawnpid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			printf("background pid %d is done: ", spawnpid);
			fflush(stdout);
			exitStatus(status);
		}

		//Flush output buffers each time we print to avoid errors
		printf(": ");	
		fflush(stdout);

		/**************************************
			GET AND PARSE USER'S INPUT	
		***************************************/

		//use fgets to read a string that has spaces, check if it is the end
		if(fgets(userInput, MAX_LENGTH, stdin) == NULL)
		{
			return 0;
		}

		//reset index of parsedInput array for token parsing
		parseIndex = 0;

		//The lecture material on strtok confused me a little
		//I found the geeksforgeeks example to be more helpful
		token = strtok(userInput, " \n");
		while (token != NULL)
		{
			//Check for input file
			if (strcmp(token, "<") == 0)
			{
				//Parse token by going to next argument and store the input name
				token = strtok(NULL, " \n");
				//strcpy did not work so I used strdup()
				inputFile = strdup(token);
			}
			//Check for output file
			else if (strcmp(token, ">") == 0)
			{
				//Parse token by going to next argument and store the output name
				token = strtok(NULL, " \n");
				outputFile = strdup(token);
			}
			//Check for background command
			else if (strcmp(token, "&") == 0)
			{	
				//If we're not in foreground-only mode
				if (foregroundOnly == 0)
				{
					//Set the background flag
					background = 1;
				}
				else if (foregroundOnly == 1)
				{
					//Ignore it, do not run in background
					background = 0;
				}
			}
			else
			{
				//Store the argument
				parsedInput[parseIndex] = strdup(token);

				//expand $$ into the process ID if found
				for (i = 0; parsedInput[parseIndex][i]; i++)
				{
					if (parsedInput[parseIndex][i] == '$' && parsedInput[parseIndex][i + 1] == '$')
					{
						char expandPid[MAX_LENGTH];
						memset(expandPid, '\0', MAX_LENGTH);
						int length = i;						

						pid = getpid();

						//String manipulation
						strncpy(expandPid, parsedInput[parseIndex], length);
						strcpy(parsedInput[parseIndex], expandPid);
						sprintf(expandPid, "%d", pid);
						strcat(parsedInput[parseIndex], expandPid);
					}
				}

				parseIndex++;
			}

			//Next token
			token = strtok(NULL, " \n");
		}

		/**************************************
			EXECUTE COMMANDS	
		***************************************/

		//If comment or blank is input, do nothing
		if (parsedInput[0] == NULL || *(parsedInput[0]) == '#')
		{

		}
		//Change directory, go to HOME path if no argument
		else if (strcmp(parsedInput[0], "cd") == 0)
		{
			if (parsedInput[1] == NULL)
			{
				chdir(getenv("HOME"));
			}
			else
			{
				chdir(parsedInput[1]);
			}
		}
		//If status is entered, print the exit status
		else if(strcmp(parsedInput[0], "status") == 0)
		{
			exitStatus(status);
		}
		//Exit the shell
		else if (strcmp(parsedInput[0], "exit") == 0)
		{	
			running = 0;
		}
		//Spawn a child process
		else
		{
			//Taken from lecture 3.1
			//pid_t spawnpid = -5;
			if (background == 0)
			{
				//Allow the foreground process to be interrupted by SIGINT
				SIGINT_action.sa_handler = catchSIGINT;
				sigaction(SIGINT, &SIGINT_action, NULL);
			}
			else if (background == 1)
			{
				//Do not allow the process to be interrupted by SIGINT
				SIGINT_action.sa_handler = SIG_IGN;
				sigaction(SIGINT, &SIGINT_action, NULL);
			}

			spawnpid = fork();
			switch(spawnpid)
			{
				//If something went wrong
				case -1:
					perror("fork error");
					status = 1;
					break;

				//fork returns 0 in the child process
				case 0:
					//If not a background process, then it is foreground
					if (background == 0)
					{
						//Allow the foreground process to be interrupted
						SIGINT_action.sa_handler = catchSIGINT;	//by changing SIG_IGN to SIG_DFL
						sigaction(SIGINT, &SIGINT_action, NULL);
					}

					//If an input file was specified
					if (inputFile != NULL)
					{
						//Use O_RDONLY for read only, taken from linux man page
						inFile = open(inputFile, O_RDONLY);
						if (inFile == -1)
						{	
							//Print error for opening file
							printf("cannot open %s for input\n", inputFile);
							fflush(stdout);
							exit(1);
						}
						//Attempt to redirect with dup2()
						if (dup2(inFile, 0) == -1)
						{
							//Redirection error
							perror("dup2 error");
							exit(1);
						}
						//Close filestream
						close(inFile);
					}
					//If background, stdin will be redirected from /dev/null according to assignment specs
					else if (background == 1)
					{
						inFile = open("/dev/null", O_RDONLY);
						if (inFile == -1)
						{
							perror("open /dev/null error");
							exit(1);
						}
						if(dup2(inFile, 0) == -1)
						{
							perror("dup2 /dev/null error");
							exit(1);
						}
					}

					if (outputFile != NULL)
					{
						//O_WRONLY opens a file in write only
						//O_CREAT will create the file, but has no effect if it already exists
						//O_TRUNC will truncate the length of the file to 0
						outFile = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
						if (outFile == -1)
						{
							//Print file open error
							printf("cannot open %s for output\n", outputFile);
							fflush(stdout);
							exit(1);
						}
						//Attempt to redirect
						if(dup2(outFile, 1) == -1)
						{
							perror("dup2 error");
							exit(1);
						}
						//close filestream
						close(outFile);
					}

					//Execute command, or print error if not recognized
					if (execvp(parsedInput[0], parsedInput))
					{
						printf("%s: no such file or directory\n", parsedInput[0]);
						fflush(stdout);
						exit(1);
					}
					break;

				//fork returns the process id of the child process that was created in the parent process
				default:
					//If not in foreground only mode, then allow to execute in background
					if (background == 1 && foregroundOnly == 0)
					{
						waitpid(spawnpid, &status, WNOHANG);
						printf("background pid is %d\n", spawnpid);
						fflush(stdout);
					}
					//Otherwise, wait on child to finish executing
					else
					{
						waitpid(spawnpid, &status, 0);
					}
			}

		}

		//Cleanup/reset memory
		for (i = 0; i < MAX_ARGUMENTS; i++)
		{
			parsedInput[i] = NULL;
		}
		free(inputFile);
		inputFile = NULL;
		free(outputFile);
		outputFile = NULL;
		memset(userInput, '\0', sizeof(userInput));

	} while (running);

	//Kill all the zombies
	while((spawnpid = waitpid(-1, &status, WNOHANG)) == 0)
	{
		kill(spawnpid, SIGTERM);
	}

	return 0;
}

/**************************************
This function is the target of the redirection for the signal handler
It essentially allows the user to turn foreground-only mode on/off
Taken from Lecture 3.5
***************************************/
void catchSIGTSTP(int signal){
	//If not in foreground-only, enter foreground-only mode
	if (foregroundOnly == 0)
	{
		char *sigStatus = "\nEntering foreground-only mode (& is now ignored)\n: ";
		//Cannot use printf during a signal handler
		write(1, sigStatus, 52);
		//Not sure if we should flush to be safe, so I will anyway
		fflush(stdout);
		//Enter foreground-only mode
		foregroundOnly = 1;
	}
	//exit foreground-only mode
	else
	{
		char *sigStatus = "\nExiting foreground-only mode\n: ";
		//Cannot use printf during a signal handler
		write(1, sigStatus, 32);
		//Not sure if we should flush to be safe, so I will anyway
		fflush(stdout);
		//Exit foreground-only mode
		foregroundOnly = 0;
	}
}

/**************************************
This function causes SIGINT to also print the terminating signal
***************************************/
void catchSIGINT(int signal)
{
	printf("termianted by signal %d\n", signal);
	fflush(stdout);
}

/**************************************
Taken from lecture 3.1
This function prints out the exit status or the terminating signal of the last foreground process.
***************************************/
void exitStatus(int childExitMethod)
{
	if(WIFEXITED(childExitMethod) != 0)
	{
		printf("exit value %d\n", WEXITSTATUS(childExitMethod));
		fflush(stdout);
	}
	else if (WIFSIGNALED(childExitMethod) != 0)
	{
		printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
		fflush(stdout);
	}
}