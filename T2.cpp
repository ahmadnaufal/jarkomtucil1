/* File : T1_rx.cpp */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <cstring>
#include "dcomm.h"
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
int min_upper=6,max_lower=2;
/* Socket */
int sockfd; // listen on sock_fd
/* Functions declaration */
static Byte *rcvchar(int sockfd, QTYPE *queue);
static Byte *q_get(QTYPE *, Byte *);
void error(char* message);
int main(int argc, char *argv[])
{
	Byte c;
 	/* Insert code here to bind socket to the port number given in argv[1]. */
 	if(argc<3) {
 		fprintf(stderr, "usage %s hostname port\n",argv[0]);
 		exit(0);	
 	}
 	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
 	printf("membuat socket untuk koneksi ke %s\n",argv[0]);
 	if(sockfd<0)
 		error("ERROR OPENING SOCKET");
	struct hostent *server;
 	server = gethostbyname(argv[1]);
 	if(server==NULL) {
 		fprintf(stderr, "ERROR, no such host\n");
 		exit(0);
 	}	
	int portnum;
 	portnum = atoi(argv[2]);
	struct sockaddr_in adhost;
 	//bzero((char*) &adhost,sizeof(adhost));
 	adhost.sin_family = AF_UNIX;
 	adhost.sin_port = htons(portnum);
 	//bcopy((char *)server->h_addr, (char *)&adhost.sin_addr.s_addr, server->h_length);
 	if(connect(sockfd,(struct sockaddr*) &adhost,sizeof(adhost))<0)
 		error("error connecting");

 	/* Create child process */
 	pid_t pid = fork();
	/*** IF PARENT PROCESS ***/
	if(!pid) {
	 	while (true) {
 			c = *(rcvchar(sockfd, rxq));
	 		/* Quit on end of file */
 			if(c == Endfile) {
 				exit(0);
 			}
		}
	}
	/*** ELSE IF CHILD PROCESS ***/
	else {
 		while (true) {
 			Byte *data,*current;
 			/* Call q_get */
 			current = q_get(rxq,data);
 			/* Can introduce some delay here. */
 			usleep(DELAY);
 		}
 	}
}

void error(char *message) {
	perror(message);
	exit(0);
}

static Byte *rcvchar(int sockfd, QTYPE *queue)
{
 	/* Insert code here. Read a character from socket and put it to the receive buffer.
 	If the number of characters in the receive buffer is above certain level, then send
 	XOFF and set a flag (why?). Return a pointer to the buffer where data is put. */
 	char b[1];
 	queue->count = queue->count<queue->rear?queue->rear-queue->count:7-queue->count+queue->rear;
 	if(queue->count>min_upper) {
 		send_xoff = true;
 		printf("buffer besar dari maksimum upper limit. mengirim XOFF\n");
 		sent_xonxoff = XOFF;
 		int n = write(sockfd,(void *)sent_xonxoff,16);
 	}
 	else {
 		send_xoff = false;
 		int n = read(sockfd,b,1);
 		if(n<0)
 			error("error reading from socket\n");
 		else {
 			queue->rear=queue->rear>RXQSIZE-1?0:queue->rear+1;
 			printf("menerima byte ke-%d: '%c'\n",queue->rear,queue->data[queue->rear]);
 			queue->data[queue->rear]=b[1];
 		}
 	}
 	return queue->data;
}

static Byte *q_get(QTYPE *queue, Byte *data)
/* q_get returns a pointer to the buffer where data is read or NULL if buffer is empty. */
{
 	Byte *current;
 	/* Nothing in the queue */
 	if (!queue->count) return (NULL);

 	/* Insert code here.  Retrieve data from buffer, save it to "current" and "data"
 	If the number of characters in the receive buffer is below certain  level, then send
 	XON. Increment front index and check for wraparound. */
 	queue->count = queue->front<queue->rear?queue->rear-queue->front:7-queue->front+queue->rear;
 	if(queue->count<max_lower) {
 		send_xon = true;
 		printf("buffer lebih kecil dari minimum lowerlimit. mengirim XON\n");
 		sent_xonxoff = XON;
 		int n = write(sockfd,(void *)sent_xonxoff,16);
 	}	
 	else {
 		send_xon = false;
 		printf("mengkonsumsi byte ke-%d: '%c'\n",queue->count,queue->data[queue->count]);
 		data = queue->data;
 		current = (Byte *)queue->data[queue->count];  
 		queue->front=queue->front>RXQSIZE-1?0:queue->front+1;
 	}
 	return current;
}