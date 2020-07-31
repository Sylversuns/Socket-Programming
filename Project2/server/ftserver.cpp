// Programmer Name: Daniel Lindsay
// This program is based on information from GeeksForGeeks, 
// 			StackOverflow, and Beej's Guide
// Program Name: FT Server
// Program Description: A server that responds to a client request by either
// 			sending a directory, or a file from that directory
// Course Name: CS372
// Last Modified: 11/25/2018

// Import libraries
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

// Define the maximum allowable transferred data size
#define MAXDATASIZE 2000

int main(int argc, char **argv)
{
	// Define variables
	int errorcheck;
	int errno;
	int listeningSocket;
	int controlSocket;
	int dataSocket;
	socklen_t addrlen = sizeof(struct sockaddr);
	struct addrinfo options;
	struct addrinfo *serverinfo;
	struct sockaddr_storage clientAddress;
	struct sockaddr_in serv_addr;
	socklen_t addressSize;
	char buffer[MAXDATASIZE];
	std::string bufferString;
	std::string command;
	std::string clientDataPortString;
	std::string fileName;
	int clientDataPortInt;
	struct dirent *dirList;		// These are used to get directory information from the local folder
	DIR *thisDir = opendir(".");

	// Clear memory in options first, then specify its members' information
	memset(&options, 0, sizeof options);
	options.ai_family = AF_INET;
	options.ai_socktype = SOCK_STREAM;
	options.ai_flags = AI_PASSIVE;

	// getaddrinfo, return error on failure
	errorcheck = getaddrinfo(NULL, argv[1], &options, &serverinfo);
	if (errorcheck != 0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errorcheck));
		exit(1);
	}
	
	// Set up main socket, return error on failure
	listeningSocket = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
	if (listeningSocket == -1)
	{
		fprintf(stderr, "socket setup error\n");
		exit(1);
	}

	// lose the pesky "Address already in use" error message (from Beej's Guide)
	int yes=1;
	if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
	{
		perror("setsockopt");
        	exit(1);
	} 

	// Bind the socket, return error on failure
	errorcheck = bind(listeningSocket, serverinfo->ai_addr, serverinfo->ai_addrlen);
	if (errorcheck == -1)
	{
		fprintf(stderr, "binding error\n");
		exit(1);
	}

	// Make the socket listen, return error on failure
	errorcheck = listen(listeningSocket, 10);
        if (errorcheck == -1)
        {
                fprintf(stderr, "listening error \n");
                exit(1);
        }	

	// Indicate that the server is listening
	printf("The server is waiting for connections...\n");

	// Keep server alive until it's killed on purpose
	while (1)
	{
		// Accept connection from client, return error on failure
		addressSize = sizeof clientAddress;
		controlSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &addressSize);
        	if (controlSocket == -1)
        	{
        	        fprintf(stderr, "accepting error \n");
        	        exit(1);
        	}	

		// Indicate connection was successful
		printf("Control connection established!\n");

		// Receive and parse the command
		recv(controlSocket, buffer, MAXDATASIZE-1,0);
		bufferString = buffer;
		std::istringstream iss (bufferString);
		iss >> command;
	
		// If the client sent the list command...
		if (command.compare("-l") == 0)
		{
			// Get the client's preferred data port
			// use it to create and connect data socket.
			iss >> clientDataPortString;
			clientDataPortInt = std::stoi(clientDataPortString);
			if ((dataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        		{
                		printf("\n Socket creation error \n");
                		return -1;
        		}
			getpeername(controlSocket, (sockaddr *)&serv_addr, &addrlen);
			serv_addr.sin_port = htons(clientDataPortInt);
			if (connect(dataSocket, (sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
			{
				printf("Connection failed.\n");
				perror("");
				return -1;
			}

			// Indicate that the connection was successful.
			printf("Data connection established!\n");

			// Start with an empty string...
			std::string fileList = "";
			// And add all the files to it.
			while ((dirList = readdir(thisDir)) != NULL)
			{
				fileList.append(dirList->d_name);
				fileList.append("\n");
			}
			// Rewind just in case we need to go through again.
			rewinddir(thisDir);
			// Send the stream back to the client through the data connection
			char *sendThis;
			sendThis = const_cast<char*>(fileList.c_str());
			send(dataSocket, sendThis, strlen(sendThis), 0);
		}

		// If the client sent the file request command...
		else if (command.compare("-g") == 0)
		//else if (command.find("g") != std::string::npos)
		{
                        // Get the client's preferred data port
                        // and use it to create and connect data socket.
			iss >> clientDataPortString;
			clientDataPortInt = std::stoi(clientDataPortString);
			if ((dataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        		{
                		printf("\n Socket creation error \n");
                		return -1;
        		}
			getpeername(controlSocket, (sockaddr *)&serv_addr, &addrlen);
			serv_addr.sin_port = htons(clientDataPortInt);
			if (connect(dataSocket, (sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
			{
				printf("Connection failed.\n");
				perror("");
				return -1;
			}

			// Indicate that the connection was successful.
			printf("Data connection established!\n");

			// Get file name from the received information.
			iss >> fileName;
			std::cout << fileName;

			// Go through the directory and find the file, then send it
			while (1)
			{
				dirList = readdir(thisDir);
				if (dirList == NULL)
				{
					std::string nofileString = "There is no file by that name.";
					char *nofile;
					nofile = const_cast<char*>(nofileString.c_str());
					send(dataSocket, nofile, strlen(nofile), 0);
					break;
				}
				if (dirList->d_name == fileName)
				{
					std::ifstream infile (fileName);
					std::string content((std::istreambuf_iterator<char>(infile)),(std::istreambuf_iterator<char>()));
					int bytes_to_send = content.length();
					int bytes_sent = 0;
					int iResult;
					do
					{
						iResult = send(dataSocket, content.data() + bytes_sent, bytes_to_send, 0);
						if (iResult != -1)
						{
							bytes_to_send -= iResult;
							bytes_sent += iResult;
						}
					} while (iResult != -1 && bytes_to_send > 0);
					break;
				}
			}
			// Rewind in case we need this later.
			rewinddir(thisDir);
			fileName = "";
			bufferString = "";
			memset(&buffer, 0, sizeof buffer);
		}

		// Anything else is considered an error.
		else
		{
			char *errorMessage = (char*)"That is not a valid command.\n";
			send(controlSocket, errorMessage, strlen(errorMessage), 0);
		}
		close(dataSocket);
	}
	// Free up the serverinfo space, and close the current directory.
	freeaddrinfo(serverinfo);
	closedir(thisDir);
	// CLose the listening socket.
	close(listeningSocket);
	return 0;
}
