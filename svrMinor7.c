/*
Joshua Allen
Minor assignment 7
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int processInput(int ns, int* tickets, int id);

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	//initialize variables
	int maxfd;
	fd_set fds;
	int nread;
	int nready;
	int tickets[20] = {0};
	int i;
	srand ( time(NULL) );
	//fill array with random numbers between 10000 and 99999
	for (i = 0; i < 20; i++){
		tickets[i] = rand() % 90000 + 10000;
		int j;
		//make sure it's unique
		for (j = 0; j < i; j++){
			if (tickets[i] == tickets[j]){
				i--;
				break;
			}
		}
	}
	
	//create socket and prepare structs
	int sockfd, ns, ns2, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	{
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		error("ERROR on binding");
	}
	
	//listen, then accept both connections (blocks until connected)
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	ns = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (ns < 0) 
	{
		error("ERROR on accept");
	}
	ns2 = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	printf("accepted 2\n");
	if (ns2 < 0) 
	{
		error("ERROR on accept");
	}
	maxfd = (ns > ns2 ? ns : ns2) + 1;
	//use select to switch between each socket
	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(ns,&fds);
		FD_SET(ns2,&fds);

		nready = select(maxfd, &fds, (fd_set *) 0, (fd_set *) 0,
		(struct timeval *) 0);
		
		//stop will let me know when both sockets are finished
		int stop = 0;
		if( FD_ISSET(ns, &fds))
		{
			//will return 1 if socket is closed
			stop += processInput(ns, tickets, 1);
		}

		if( FD_ISSET(ns2, &fds))
		{
			//will return 1 if socket is closed
			stop += processInput(ns2, tickets, 2);
		}
		if (stop == 2){
			//both sockets finished
			break;
		}
	} 
	//close all connections
	close(ns);
	close(ns2);
	close(sockfd);

	return 0; 
}

int processInput(int ns, int* tickets, int id){
	char buffer[15] = {0};
	//read to buffer
	int nread = recv(ns, buffer, 15, 0);
	//return 1 if socket is closed or had an error
	if(nread < 1)
		return 1;
	if (strcmp(buffer, "BUY") == 0){
		printf("[CLIENT %d]: BUY\n", id);
		//choose a random starting location for the iteration
		unsigned i = rand();
		unsigned max = i + 20;
		for (;i < max; i++){
			if (tickets[i%20] > 0){
				//send the number to the client
				char str[6];
				sprintf(str, "%d", tickets[i%20]);
				printf("[SERVER X]: client %d buy %d\n", id, tickets[i%20]);
				send( ns, str, 6, 0);
				//represent sold tickets as negative numbers
				tickets[i%20] = - tickets[i%20];
				return 0;
			}
		}
		//database is full
		printf("[SERVER X]: Database full\n");
		send( ns, "Database full", 14, 0);
		return 0;
	}
	//buffer is a number
	printf("[CLIENT %d]: RETURN %s\n", id, buffer);
	int number;
	sscanf(buffer, "%d", &number);
	int i;
	for (i = 0;i < 20; i++){
		//search among sold tickets for the number
		if (tickets[i] == -number){
			//represent the ticket as unsold, and send the number to the client
			tickets[i] = - tickets[i];
			printf("[SERVER X]: client %d cancel %d\n", id, tickets[i]);
			send( ns, buffer, 6, 0);
			return 0;
		}
	}
	//number is not among sold tickets
	char str[10];
	send( ns, "Ticket does not exist", 22, 0);
	return 0;
}


