/* File : T1_rx.cpp */
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include "receiver.h"
#define bzero(p, size) (void)memset((p), 0 , (size))
/* Delay to adjust speed of consuming buffer, in milliseconds */
#define DELAY 500
/* Define receive buffer size */
#define RXQSIZE 8

Byte rxbuf[RXQSIZE];
QTYPE rcvq = { 0, 0, 0, RXQSIZE, rxbuf };
QTYPE *rxq = &rcvq;
Byte sent_xonxoff = XON;
bool send_xon = false,
send_xoff = false;
int endFileReceived;

/* Socket */
int sockfd; // listen on sock_fd
struct sockaddr_in adhost;
struct sockaddr_in srcAddr;
unsigned int srcLen = sizeof(srcAddr);

/* Functions declaration */
static Byte *rcvchar(int sockfd, QTYPE *queue);
static Byte *q_get(QTYPE *, Byte *);

void error(char* message);

int main(int argc, char *argv[])
{
 	if(argc<2) {
 		fprintf(stderr, "usage %s port\n",argv[0]);
 		exit(0);	
 	}

 	printf("membuat socket untuk koneksi ke %s\n", argv[0]);
 	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
 		error("ERROR OPENING SOCKET");

 	printf("TEST1\n");

	printf("TES2\n");
 	bzero((char*) &adhost, sizeof(adhost));
 	adhost.sin_family = AF_INET;
 	adhost.sin_port = htons(atoi(argv[1]));
 	adhost.sin_addr.s_addr = INADDR_ANY;

 	printf("TEST3\n");
 	if(bind(sockfd, (struct sockaddr*) &adhost, sizeof(adhost)) < 0)
 		error("error connecting");

 	/* Create child process */
 	pid_t pid;
 	endFileReceived = 0;
 	memset(rxbuf, 0, sizeof(rxbuf));
 	
 	
	/*** IF PARENT PROCESS ***/
	if ((pid = fork()) > 0) {
		Byte c;
	 	while (true) {
 			c = *(rcvchar(sockfd, rxq));
	 		/* Quit on end of file */
 			if (c == Endfile) {
 				exit(0);
 			}
		}
	} else if (pid == 0) {
		/*** ELSE IF CHILD PROCESS ***/
		Byte *data, *current = NULL;
 		do {
 			/* Call q_get */
 			current = q_get(rxq, data);

 			if (current != NULL && endFileReceived) break;
 			/* Can introduce some delay here. */
 			sleep(1);
 		} while (true);
 	} else {
 		error("JANCOK ASU");
 	}
}

void error(char *message) {
	perror(message);
	exit(1);
}

static Byte *rcvchar(int sockfd, QTYPE *queue)
{
 	/* Insert code here. Read a character from socket and put it to the receive buffer.
 	If the number of characters in the receive buffer is above certain level, then send
 	XOFF and set a flag (why?). Return a pointer to the buffer where data is put. */
 	Byte* current;
 	char tempBuf[1];
 	char b[1];

 	if (recvfrom(sockfd, tempBuf, 1, 0, (struct sockaddr *) &srcAddr, &srcLen) < 0)
 		error("FAILED to receive character from socket");

 	current = (Byte *) malloc(sizeof(Byte));
 	*current = tempBuf[0];

 	printf("Menerima byte ke-%d: '%c'\n",queue->count, *current);

 	if (*current == Endfile) endFileReceived = 1;

 	if (queue->count < 8) {
 		(*queue).rear = (queue->count > 0) ? (++(*queue).rear) % 8 : queue->rear;
 		(*queue).data[queue->rear] = *current;
 		(*queue).count++;
 	}

 	if(queue->count >= (MIN_UPPERLIMIT) && sent_xonxoff == XON) {
 		printf("buffer besar dari maksimum upper limit. mengirim XOFF %d\n",queue->count);
 		send_xoff = true;
 		b[0] = sent_xonxoff = XOFF;

 		if(sendto(sockfd, b, 1, 0,(struct sockaddr *) &srcAddr, srcLen) < 0)
 			error("error ngirim XOFF\n");
 	}

 	return current;
}

static Byte *q_get(QTYPE *queue, Byte *data)
/* q_get returns a pointer to the buffer where data is read or NULL if buffer is empty. */
{
 	Byte *current = NULL;
 	char b[1];
 	/* Nothing in the queue */
 	printf("%d\n", queue->count);
 	if (queue->count > 0) {
 		current = (Byte* ) malloc (sizeof(Byte));
 		*current = queue->data[queue->front];
 		if (*current == Endfile) exit(0);

 		queue->front = ((*queue).front++) % 8;
 		(*queue).count--;
 		printf("mengkonsumsi byte ke-%d: '%c'\n",queue->rear, *current);
 	}

 	/* Insert code here.  Retrieve data from buffer, save it to "current" and "data"
 	If the number of characters in the receive buffer is below certain  level, then send
 	XON. Increment front index and check for wraparound. */
 	if(queue->count <= MAX_LOWERLIMIT && sent_xonxoff == XOFF) {
 		send_xon = true;
 		printf("buffer lebih kecil dari minimum lowerlimit. mengirim XON\n");
 		b[0] = sent_xonxoff = XON;
 		if(sendto(sockfd, b, 1, 0, (struct sockaddr *) &srcAddr, srcLen) < 0)
 			error("error ngirim XON\n");
 	}	

 	return current;
}