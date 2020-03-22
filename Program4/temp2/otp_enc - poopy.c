/************************************************
* Author:		Alexander Tir (tira)
* Date:			12/5/2019
* Description:	
************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

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
	struct sockaddr_in serverAddress;	//struct for the server address
	struct hostent* serverHostInfo;
	struct sockaddr_in clientAddress;	//struct for the client address
	int yes = 1;		//used for setsocketopt
	int status;
	int i;
	char auth[] = "otp_enc";
	int messageFile, keyFile, messageSize, keySize;
	int checkFile;
	int fileStatus;

	//the largest plaintext while we are given has about 64,000 characters
	char buffer[70000];
	char key[70000];
	memset(buffer, '\0', sizeof(buffer));
	memset(key, '\0', sizeof(key));

	//check arguments, print the proper usage and exit
	if (argc != 4) 
	{ 
		error("USAGE: otp_enc plaintext key port\n");
		exit(1); 
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); 				// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 		// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
	{
		error("CLIENT: ERROR opening socket\n");
		exit(1);
	}

	//allows program to continue to run the same port
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	//converting localhost into address
	serverHostInfo = gethostbyname("localhost");
	if (serverHostInfo == NULL) 
	{ 
		error("CLIENT: ERROR, no such host\n");
		exit(1); 
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
	{
		fprintf(stderr, "CLIENT: ERROR connecting to server at port %d\n", portNumber);
		exit(2);
	}

	//otp_enc can only connect to otp_enc_d
	//attempt to connect to server using authentication key
	send(socketFD, auth, sizeof(auth), 0);

	//clear buffer before receiving server response
	memset(buffer, '\0', sizeof(buffer));
	recv(socketFD, buffer, sizeof(buffer), 0);

	//terminate if server connection refused
	if(strcmp(buffer, "refused") == 0)
	{
		fprintf(stderr, "CLIENT: ERROR otp_dec_d refused the connection at port %d\n", portNumber);
		exit(2);
	}	

	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

	//validate message file
	checkFile = open(argv[1], O_RDONLY);
	if (checkFile == -1)
	{
		fprintf(stderr, "CLIENT: ERROR could not open file '%s'\n", argv[1]);
		exit(1);
	}

	//store size of message
	messageSize = read(checkFile, buffer, sizeof(buffer))

	//check the message file for any bad characters
	for (i = 0; i < messageSize - 1; i++)
	{
		if (isspace(buffer[0]) || isalpha(buffer[0]))
		{
			//do nothing
		}
		else
		{
			fprintf(stderr, "otp_enc error: plaintext contains bad characters\n");
			exit(1);
		}
	}
	/*while(read(checkFile, buffer, 1) != 0)
	{
		if (isspace(buffer[0]) || isalpha(buffer[0]))
		{
			//do nothing
		}
		else
		{
			fprintf(stderr, "otp_enc error: plaintext contains bad characters\n");
			exit(1);
		}
	}*/
	close(checkFile);

	//validate key file
	checkFile = open(argv[2], O_RDONLY);
	//read 1 character at a time
	while(read(checkFile, buffer, 1) != 0)
	{
		if (isspace(buffer[0]) || isalpha(buffer[0]))
		{
			//do nothing
		}
		else
		{
			fprintf(stderr, "otp_enc error: key contains bad characters\n");
			exit(1);
		}
	}

	/*while (read(checkFile, buffer, 1) != 0)
	{
		//printf("%c\n", buffer[0]);
		if ((int) buffer[0] > 90 || ((int) buffer[0] < 65 && (int) buffer[0] != 32))
		{
			error("otp_enc error: input contains bad characters\n");
			exit(1);
		}
	}*/
	close(checkFile);

	//open the message
	messageFile = open(argv[1], O_RDONLY);
	if (messageFile == -1)
	{
		fprintf(stderr, "CLIENT: ERROR could not open file '%s'\n", argv[1]);
		exit(1);
	}
	//get the file size of the message
	messageSize = lseek(messageFile, 0, SEEK_END);

	//open the key
	keyFile = open(argv[2], O_RDONLY);
	if (keyFile == -1)
	{
		fprintf(stderr, "CLIENT: ERROR could not open file '%s'\n", argv[2]);
		exit(1);
	}
	//get the file size of the key
	keySize = lseek(keyFile, 0, SEEK_END);

	//check that key is long enough
	if (keySize < messageSize)
	{
		fprintf(stderr, "ERROR: key ‘%s’ is too short\n", argv[2]);
		exit(1);
	}

	

	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

	//load the message to send to server
	read(messageFile, buffer, sizeof(messageFile));

	//send it
	fileStatus = send(socketFD, buffer, sizeof(buffer) - 1, 0);

	//error message output for testing
    /*if (sentInfo < inputFileSize - 1)
    {
        printf("File: top_enc_d ERROR! port: %d could not send \n", fileLocation);
        exit(2);
    }*/

	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

    //load the key to send to server
	read(keyFile, buffer, sizeof(keyFile));

	//send the key
	fileStatus = send(socketFD, buffer, sizeof(buffer) - 1, 0);

	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

	//receive the encrypted message
	fileStatus = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	if (fileStatus == -1)
	{
		error("CLIENT: ERROR reading from server\n");
		exit(1);
	}

	printf("%s\n", buffer);

 	close(messageFile);
 	close(keyFile);

	close(socketFD); // Close the socket

	return 0;
}