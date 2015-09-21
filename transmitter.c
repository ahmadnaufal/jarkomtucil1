 /*
 	File 	: transmitter.c
 */

#include "transmitter.h"

 /* GLOBAL VARIABLES ARE INSIDE THE HEADER */


int main(int argc, char *argv[]) {
	int sockfd, port;		// sock file descriptor and port number
	pid_t pid;			// process id for forking

	if (argc < 4) {
		// case if arguments are less than specified
		printf("[TRANSMITTER] Please use the program with arguments: transmitter <target-ip> <socket> <filename>\n");
		return 1;
	} else {
		port = atoi(argv[2]);
		receiverIP = argv[1];

		// creating IPv4 data stream socket
		if (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0) {
			printf("ERROR: Cannot create socket.\n");
			return 1;
		}

		isSocketOpen = 1;

		// initializing the socket host information
		memset(&receiverAddr, 0, sizeof(receiverAddr));
		receiverAddr.sin_family = AF_INET;
		receiverAddr.sin_addr.s_addr = inet_addr(receiverIP);
		receiverAddr.sin_port = htons(port);

		if ((pid = fork()) > 0) {
			// this is the parent process
			// use as char transmitter from the text file

			tFile = fopen(argv[argc-1], "r");
			if (tFile == NULL) {
				printf("ERROR: File %s not Found.\n", argv[argc-1]);
				return 1;
			}

			// connect to receiver, and read the file per character
			int counter = 1;
			while ((buf[0] = fgetc(tFile)) != EOF) {
				if (isXON) {
					if (sendto(sockfd, buf, BUFMAX, 0, &receiverAddr, receiverAddrLen) != BUFMAX) {
						printf("ERROR: sendto() sent buffer with size more than expected.\n");
						return 1;
					}

					printf("[TRANSMITTER] Mengirim byte ke-%d: \'%c\'\n", counter++, buf[0]);
				} else {
					printf("Waiting for XON...\n");
				}
			}

			fclose(tFile);
			printf("Byte sending done! Closing sockets...\n");
			close(sockfd);
			printf("Socket Closed!\n");

		} else if (pid == 0) {
			// child process
			// read if there is XON/XOFF sent by receiver by using sendto
			struct sockaddr_in srcAddr;
			int srcLen = sizeof(srcAddr);
			while (!isSocketClosed) {
				if (recvfrom(sockfd, xbuf, BUFMAX, 0, &srcAddr, &srcLen) != BUFMAX) {
					printf("ERROR: recvfrom() receive buffer with size more than expected.\n");
					return 1;
				}

				if (xbuf[0] == XOFF) {
					isXON = 0;
					printf("[TRANSMITTER] Receiving XOFF. Rest a while buddy...\n");
				} else if (xbuf[0] == XON) {
					isXON = 1;
					printf("[TRANSMITTER] Receiving XON. Work again!\n");
				} else {
					printf("What the hell man?\n");
				}
			}


		} else {
			printf("ERROR: Child cannot be created. (MANDUL)\n");
			return 1;
		}

		printf("[TRANSMITTER] Finished!\n");
		return 0;
	}

}
