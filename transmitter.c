#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFMAX 1

int isXON = 1;
int isSocketClosed;
struct sockaddr_in receiverAddr;
int receiverAddrLen = sizeof(receiverAddr);
FILE *tFile;
char *receiverIP;	// buffer for Host IP address
char buf[BUFMAX];
char xbuf[BUFMAX+1];

int main(int argc, char *argv[]) {
	int sockfd, port;		// sock file descriptor and port number
	pid_t pid;

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

		isSocketClosed = 1;

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

			// connect to receiver, and read the file
			int i = 1;
			while ((buf[0] = fgetc(tFile)) != EOF) {
				if (isXON) {
					 if (sendto(sockfd, buf, BUFMAX, 0, &receiverAddr, receiverAddrLen) != BUFMAX) {
					 	printf("ERROR: sendto() sent buffer with size more than expected.\n");
					 	return 1;
					 }

					 printf("[TRANSMITTER] Mengirim byte ke-%d: \'%c\'\n", i++, buf[0]);
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