/************************************************
* Author:		Alexander Tir (tira)
* Date:			12/5/2019
* Description:	Server file that accepts a connection from the otp_dec client,
				receives an encrypted file, decrypts it, and sends it back to 
				the client.
************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <signal.h>

//Borrowed from examples, print error messages to stderr
void error(const char *msg) 
{ 
	fprintf(stderr, "%s", msg);
}

int main(int argc, char* argv[])
{
	//initialize network variables		
	int socketFD;
	int portNumber;					
	int newConnection;
	socklen_t sizeOfClientInfo;			//size of client info
	struct sockaddr_in serverAddress;	//struct for the server address
	struct sockaddr_in clientAddress;	//struct for the client address
	struct hostent* serverHostInfo;
	int yes = 1;		//used for setsocketopt
	pid_t spawnpid;		//used when spawning child process
	int status;
	int i;

	//the largest plaintext while we are given has about 64,000 characters
	char buffer[80000];
	char key[80000];
	char decrypted[80000];
	memset(buffer, '\0', sizeof(buffer));
	memset(key, '\0', sizeof(key));
	memset(decrypted, '\0', sizeof(decrypted));

	//check arguments, print the proper usage and exit
	if (argc != 2) 
	{ 
		fprintf(stderr,"USAGE: otp_dec_d listening_port\n"); 
		exit(1); 
	}
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); 				// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 		// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Allow any address to connect

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
	{
		error("CLIENT: ERROR opening socket\n");
		exit(1);
	}

	//allows program to continue to run the same port
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	// Enable the socket to begin listening
	if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
	{
		error("ERROR on binding\n");
		exit(1);
	}

	// Flip the socket on, up to 5 processes can queue up
	listen(socketFD, 5);

	//There does not appear to be an exit in the assignment, so assume this process continues to run until interrupted by a stop signal.
	while(1)
	{
		// Get the size of the address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress); 
		// Accept new connection
		newConnection = accept(socketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (newConnection < 0) 
		{
			error("ERROR on accept\n");
		}

		spawnpid = fork();
		switch(spawnpid)
		{
			//If something went wrong
			case -1:
				error("ERROR fork\n");
				break;

			//child process
			case 0: ;
				int numBytes;
				int messageSize;
				int keySize;
				int sent;
				int messageChar;
				int keyChar;
				int cipherText;
				int cipherTemp;
				int currentSize;
				int r;

				char received[1024];
				memset (received, '\0', sizeof(received));

				//clear buffer
				memset(buffer, '\0', sizeof(buffer));

				//validate the client connection, only otp_enc can connect
				recv(newConnection, buffer, sizeof(buffer) - 1, 0);
				if(strcmp(buffer, "otp_dec") != 0)
				{
					char response[] = "refused";
					send(newConnection, response, sizeof(response), 0);
					error("SERVER: ERROR otp_dec_d refused client connection\n");
					exit(2);
				}
				else
				{
					//it doesn't matter what we send back, but we have to send back something that is not "refused"
					char response[] = "success";
					send(newConnection, response, sizeof(response), 0);
				}

				//clear buffer
				memset(buffer, '\0', sizeof(buffer));

				//receive message size from client
				recv(newConnection, &messageSize, sizeof(messageSize), 0);
				if ( messageSize == -1)
				{
					error("ERROR recv from client\n");
					exit(1);
				}
				
				currentSize = 0;
				//this loop strategy was found in the lectures
				while (currentSize <= messageSize)
				{
					//clear buffer
					memset (received, '\0', sizeof(received));

					r = recv(newConnection, received, sizeof(received), 0);

					if (r == -1)
					{
						error("ERROR recv from client\n");
						exit(1);
					}
					else if (r == 0)
					{
						//we could be at the end, check edge case
						if (currentSize < messageSize)
						{
							break;
						}
					}
					else
					{
						//r-1 allows us to get rid of the termination character
						strncat(buffer, received, r - 1);
					}
					currentSize = currentSize + (r - 1);
				}

				//slap on that null terminator
				buffer[messageSize - 1] = '\0';

				//clear key buffer
				memset(key, '\0', sizeof(key));

				//receive key size from client
				//keySize = 0;
				recv(newConnection, &keySize, sizeof(keySize), 0);
				if (keySize == -1)
				{
					error("ERROR could not read key file from client\n");
					exit(1);
				}
				
				currentSize = 0;
				//this loop strategy was found in the lectures
				while (currentSize <= keySize)
				{	
					//clear buffer
					memset (received, '\0', sizeof(received));

					r = recv(newConnection, received, sizeof(received), 0);

					if (r == -1)
					{
						error("ERROR recv from client\n");
						exit(1);
					}
					else if (r == 0)
					{
						break;
					}
					else
					{
						//r-1 allows us to get rid of the termination character
						strncat(key, received, r - 1);
					}
					currentSize = currentSize + (r - 1);
				}

				//slap on that null terminator
				key[keySize - 1] = '\0';

				//this should already be checked client-side
				if (keySize < messageSize)
				{
					error("ERROR key too short\n");
					exit(1);
				}

				memset(decrypted, '\0', sizeof(decrypted));

				//encrypt message according to key
				for (i = 0; i < messageSize; i++)
				{
					//rather than use space, we will use @ since it is within range of A-Z
					if(buffer[i] == ' ')
					{
						buffer[i] = '@';
					}

					if(key[i] == ' ')
					{
						key[i] = '@';
					}

					//cast the characters into integers
					cipherText = (int) buffer[i];
					//messageChar = (int) buffer[i];
					keyChar = (int) key[i];

					//substract the ascii value of @
					cipherText = cipherText - 64;
					//messageChar = messageChar - 64;
					keyChar = keyChar - 64;

					//only some of my text was being decrypted correctly
					//turns out we need to check for a negative number
					cipherTemp = cipherText - keyChar;
					if (cipherTemp < 0)
					{
						cipherTemp = cipherTemp + 27;
					}

					//with the addition of @, we now use mod 27 instead of 26
					messageChar = cipherTemp % 27;
					//cipherText = (messageChar + keyChar) % 27;
					
					//convert back to capital letter ascii value
					messageChar = messageChar + 64;
					//cipherText = cipherText + 64;

					//cast back to character by adding 0
					decrypted[i] = (char) messageChar + 0;

					//check for space
					if(decrypted[i] == '@')
					{
						decrypted[i] = ' ';
					}
				}

				//slap on that null terminator
				decrypted[messageSize - 1] = '\0';

				//send decrypted message back
				currentSize = 0;
				while(currentSize < messageSize)
				{
					//clear sent buffer
					//memset(received, '\0', sizeof(received));
					char payload[1024];

					//sending a part of the message at a time
					strncpy(payload, &decrypted[currentSize], 1023);

					//null terminate the message
					payload[1024] = '\0';

					//send it
					if(send(newConnection, &payload, 1024, 0) == -1)
					{
						error("CLIENT: ERROR no connection to server\n");
						exit(2);
					}
					currentSize = currentSize + 1023;
				}

				close(newConnection);

				break;

			//parent process, wait on child to finish
			default:
				close(newConnection);
				while (spawnpid > 0)
				{
					spawnpid = waitpid(-1, &status, WNOHANG);
				}
		}
	}

	close(socketFD);

	//Kill any zombies
	while((spawnpid = waitpid(-1, &status, WNOHANG)) == 0)
	{
		kill(spawnpid, SIGTERM);
	}

	return 0;
}