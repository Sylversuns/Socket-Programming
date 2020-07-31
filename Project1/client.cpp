// By Daniel Lindsay, based on information from GeeksForGeeks and Beej's Guide

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <string>

int main (int argc, char **argv)
{
	// Variables
	int sock = 0, valread;
	int cmdlineport;
	struct sockaddr_in serv_addr;
	char buffer[500] = {0};

	// String variables
	std::string handle = "";
	std::string input = "";
	std::string sendThis = "";
	std::string checkThis = "";

	// Cast 3rd parameter from command line to integer
	std::istringstream iss(argv[2]);
	iss >> cmdlineport;

	// Create client socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	// Initialize server information
		memset(&serv_addr, '0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;

		// cmdlineport is the third parameter put in on the command line, cast as an integer
		serv_addr.sin_port = htons(cmdlineport);

		//Specify server IP address
		if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
		{
			printf("\nInvalid address// Address not supported \n");
			return -1;
		}

	// Connect to server using port number mentioned above(serv_addr.sin_port)
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	// Prompt user to get name
	std::cout << "What is your name?: ";
	std::getline(std::cin,handle);
	std::cout << "Hello, " << handle << std::endl << std::endl;
	
	// Loop forever until either server or client says quit
	while (1)
	{
		// Get user input
		std::cout << handle << "> ";
		std::getline(std::cin,input);

		// Concatenate with handle and send
		sendThis = handle + "> " + input;
		send(sock,sendThis.c_str(),sendThis.length(),0);

		// If client says quit, quit
		if (input == "\\quit")
		{
			break;
		}

		// Read from server
		valread = read(sock,buffer, 500);

		// checkThis is a string, and .find won't work unless you use it instead.
		// buffer is a character array, and C++ will complain...
		checkThis = buffer;

		// If server says quit, quit
		if (checkThis.find("\\quit") != std::string::npos)
		{
			break;
		}

		// Print the thing the server said, and clear the buffer
		printf("%s\n",buffer);
		memset(buffer,0,500);
	}

	return 0;
}
