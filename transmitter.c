#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

struct sockaddr_in receiverAddr;
FILE *tFile;
char *receiverIP;	// buffer for Host IP address

int main(int argc, char *argv[]) {
	int sockfd, port;		// sock file descriptor and port number
	p_id pid;

	if (argc < 4) {
		// case if arguments are less than specified
		printf("[TRANSMITTER] Please use the program with arguments: transmitter <target-ip> <socket> <filename>\n");
		return 1;
	} else {
		port = atoi(argv[2]);
		receiverIP = argv[1];

		// creating IPv4 data stream socket
		if (sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
			printf("ERROR: Cannot create socket.\n");
			return 1;
		}

		memset(&receiverAddr, 0, sizeof(receiverAddr));
		receiverAddr.sin_family = AF_INET;
		receiverAddr.sin_addr.s_addr = inet_addr(receiverIP);
		receiverAddr.sin_port = htons(port);

		tFile = fopen(argv[argc-1], "r");
		if (tFile == NULL) {
			printf("ERROR: File %s not Found.\n", argv[argc-1]);
			return 1;
		}

		


	}

}