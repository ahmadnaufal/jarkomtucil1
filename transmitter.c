 /* File 	: transmitter.c */
#include "transmitter.h"

void *child(void * i);
void errorr(char *message) {
	perror(message);
	exit(0);
}

int main(int argc, char *argv[]) {
	int sockfd, port;		// sock file descriptor and port number
	pid_t pid;			// process id for forking
	struct hostent *server;

	if (argc < 4) {
		// case if arguments are less than specified
		printf("[TRANSMITTER] Please use the program with arguments: transmitter <target-ip> <socket> <filename>\n");
		return 1;
	} else {
		port = atoi(argv[2]);
		if ((server = gethostbyname(argv[1])) == NULL)
			errorr("SERVER NULL\n");

		// creating IPv4 data stream socket
		printf("[TRANSMITTER] Creating socket to %s Port %s...\n", argv[1], argv[2]);
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			printf("ERROR: Create socket failed.\n");
			return 1;
		}

		// flag set to 1 (connection is established)
		isSocketOpen = 1;

		// initializing the socket host information
		memset(&receiverAddr, 0, sizeof(receiverAddr));
		receiverAddr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&receiverAddr.sin_addr.s_addr, server->h_length);
		receiverAddr.sin_port = htons(port);

		// open the text file
		tFile = fopen(argv[argc-1], "r");
		if (tFile == NULL) {
			printf("ERROR: File %s not Found.\n", argv[argc-1]);
			return 1;
		}
		pthread_t thread[1];
		int i=0;
		int rc = pthread_create(&thread[i],NULL,child,(void *)i);
		if(rc) 
			printf("gagal maning!!\n");
		// this is the parent process
		// use as char transmitter from the text file
		// connect to receiver, and read the file per character
		int counter = 1;
		while ((buf[0] = fgetc(tFile)) != EOF) {
			if (isXON) {
				if (sendto(sockfd, buf, BUFMAX, 0, (const struct sockaddr *) &receiverAddr, receiverAddrLen) != BUFMAX) {
					printf("ERROR: sendto() sent buffer with size more than expected.\n");
					return 1;
				}
				printf("[TRANSMITTER] Mengirim byte ke-%d: \'%c\'\n", counter++, buf[0]);
			} else {
				printf("Waiting for XON...\n");
			}
			usleep(1500);
		}
		// sending endfile to receiver, marking the end of data transfer
		buf[0] = Endfile;
		sendto(sockfd, buf, BUFMAX, 0, (const struct sockaddr *) &receiverAddr, receiverAddrLen);
		fclose(tFile);
		printf("Byte sending done! Closing sockets...\n");
		close(sockfd);
		isSocketOpen = 0;
		printf("Socket Closed!\n");

		// finishing program and closing
		printf("[TRANSMITTER] Finished!\n");
		return 0;
	}

}


void *child(void *threadid) {
	printf("CHILD RECEIVING HAHA\n");
	// child process
	// read if there is XON/XOFF sent by receiver using recvfrom()
	struct sockaddr_in srcAddr;
	int srcLen = sizeof(srcAddr);
	while (isSocketOpen) {
		if (recvfrom(sockfd, xbuf, BUFMAX, 0, (struct sockaddr *) &srcAddr, &srcLen) != BUFMAX) {
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
}