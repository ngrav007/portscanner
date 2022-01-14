#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Network Libraries

#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Prototypes for port_scanner, number_validator, and ip_validator
int port_scanner(char* ip, char* port);
int number_validator(char *str);
int ip_validator(char *ip);

int main(int argc, char *argv[])
{
	// Error checking, ensure correct number of arguments inputed into terminal line
	if(argc < 4)
	{
		printf("Usage: %s ip_address start_port end_port\n", argv[0]);
		return 1;
	}

	// Check for valid ip_address or DNS server name
	char ip_test[INET_ADDRSTRLEN] = {0};
	strcpy(ip_test, argv[1]);
	char dns[5] = "www.";
	if(strncmp(argv[1], dns, 4) != 0)
	{
		if(ip_validator(ip_test) != 0)
		{
			printf("Invalid ip_address: %s\n", argv[1]);
			return 1;
		}
	}

	// Check that port numbers inputted are digits
	if(number_validator(argv[2]) == 1 || number_validator(argv[3]) == 1)
	{
		printf("Port numbers must be numbers\nInvalid port numbers: %s %s\n", argv[2], argv[3]);
		return 1;
	}
	// Check that the starting port number is less than the ending port number
	if(atoi(argv[2]) > atoi(argv[3]))
	{
		printf("Usage: start_port %s must be less than end_port %s!\n", argv[2], argv[3]);
		return 1;	
	}

	// Create char array to hold IP address, here I use the macro defining the size of the largest IPv4 address possible
	// Use strcpy() to copy the first terminal argument to the char array
	char ip[INET_ADDRSTRLEN] = {0};
	strcpy(ip, argv[1]);
	
	// Do the same for the starting and ending port numbers, MAX PORT RANGE = 0-65535
	char port1[6];
	strcpy(port1, argv[2]);
	char port2[6];
	strcpy(port2, argv[3]);

	// Create numerical representations of port number for iPort and ePort
	int iPort = atoi(port1);
	int ePort = atoi(port2);
	
	/* Debugging
	printf("Port1 = %i and Port 2 = %i\n", iPort, ePort);
	*/

	// Start the port scanner and loop through port range
	for(int i = iPort; i <= ePort; i++)
	{
		char port[6];
		sprintf(port, "%i", i);
		printf("Port: %s: ", port);
		port_scanner(ip, port);
	}

	return 0;
}

int port_scanner(char* ip, char* port)
{
	// Initialize necessary structs to be filled out by getaddrinfo()
	struct addrinfo hints; 
	struct addrinfo *res;
	
	// Initialize socket file descriptor
	int sockfd;
	
	// Clear 'hints' structure of memory
	memset(&hints, 0, sizeof(hints));

	// Set some configurations for hints, stick with IPv4 and TCP
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Then we need to load up address structures with getaddrinfo():
	int status = getaddrinfo(ip, port, &hints, &res);

	// Error Check: If getaddrinfo() returns a non-zero, something went wrong, print out error with gai_strerror(), 
	if(status != 0)
	{
		printf("getaddrinfo error: %d\n", status);
		return 1;
	}
	
	/* Debugging
	printf("Success! - getaddrinfo()\n");
	*/

	// Create a socket, get the values in the *res struct corresponding to:
	// ai_family(IPv4/IPv6),ai_socktype(SOCK_STREAM/SOCK_DGRAM), and ai_protocol(TCP/UDP/etc.)
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Error check socket() 
	if(sockfd == -1)
	{
		printf("socket() error: %d\n", sockfd);
		return 2;
	}
	
	/* Debugging
	printf("Success! - socket()\n");
	*/

	// Try to connect!
	int conn = connect(sockfd, res->ai_addr, res->ai_addrlen);
	
	// Close socket
	close(sockfd);
	// Free adderinfo
	freeaddrinfo(res);	

	// Check if connection was successful
	if(conn < 0)
	{
		printf("Port Closed...\n");
	}
	if(conn == 0)
	{
		printf("Port Open!\n");
	}

	return 0;
}

int number_validator(char *str)
{
	// Confirm all characters in the ip_address string are digits, if not, return error
	while(*str)
	{
		if(!isdigit(*str))
		{
			return 1;
		}
		str++; // Point string to next char in string
	}

	return 0;
}

int ip_validator(char *ip)
{
	// Confirm ip_address is a valid ip_address, 'xxx.xxx.xxx.xxx'
	// Check if ip_address exists
	if(ip == NULL)
	{
		return 0;
	}

	// Create character to ignore, use it as strtok's delimiter
	const char delim[2] = ".";

	// Make sure last character in ip_address is not a dot, when using strtok, validation will not notice if last character is a "."
	int ip_l = strlen(ip);
	char *end_char = ip + ip_l - 1;
	if(strcmp(end_char, delim) == 0)
	{
		printf("ip_address cannot end in a dot\n");
		return 1;
	}

	// Create token of first four bytes of ip address 'xxx' leaving out "."
	char *token = strtok(ip, delim);

	// Check if first 4 bytes of ip_address (i.e. token) exists, if not, no ip_address portion to validate, return 0
	if(token == NULL)
	{
		return 0;
	}

	// Create variable to keep track of amount of 'dots' in ip_address
	int dot_count = 0;

	// While a 'token' of the ip_address exists (xxx) 
	while(token != NULL)
	{
		// Check if each character in token is a digit, if not return error
		if(number_validator(token) == 1)
		{
			printf("A valid ip address consists of numbers and dots.\nInvalid ip_address portion: '%s'\n", token);
			return 1;
		}

		// Convert string to number
		int ip_number = atoi(token);

		if(ip_number >= 0 && ip_number <= 255)
		{
			// Create token for next portion of ip_address
			token = strtok(NULL, delim);
			// If token exists (not equal to NULL) add a dot to the dot_count 
			if(token != NULL)
			{
				dot_count++;
			}
		}
		else
		{
			printf("Invalid ip_address portion: %s\nip_address = xxx.xxx.xxx.xxx\nxxx = 0-255\n", token);
			return 1;
		}
	
	}

	// If dot_count is not equal to 3, ip_address is invalid, example of valid ip_address: xxx.xxx.xxx.xxx (3 dots)
	if(dot_count != 3)
	{
		printf("Invalid ip_address: dot_count\n");
		return 1;
	}

	return 0;
}


