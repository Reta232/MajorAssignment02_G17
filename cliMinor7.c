/*
Joshua Allen
Minor assignment 7
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
//Edit test
//ANOTHER TEST?!?!?!
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	//initialize variables
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int tickets[15] = {0};
	char buffer[256];
	srand ( time(NULL) );
	if (argc < 3)
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);
	//create socket and prepare structs
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		error("ERROR opening socket");
	}
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	//connect
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
		error("ERROR connecting");
	}
	//send 15 requests
	int i;
	for (i = 0; i < 15; i++){
		//set buffer to "BUY" or a ticket number
		//buy 80% of the time
		int buy = rand()%5;
		if (buy){
			printf("[CLIENT]: BUY\n");
			stpcpy(buffer, "BUY");
		} else {
			//return 
			int j;
			int number;
			for (j = 0; j < i; j++){
				if (tickets[j] > 0){
					number = tickets[j];
					tickets[j] = 0;
					break;
				}
			}
			//generate random ticket number if buy fails
			if (j == i){
				number = rand();
				number = number % 90000 + 10000;
			}
			printf("[CLIENT]: RETURN %d\n", number);
			sprintf(buffer, "%d", number);
		}
		//write from buffer
		n = write(sockfd,buffer,strlen(buffer));

		if (n < 0) 
		{
			error("ERROR writing to socket");
		}
		//read to buffer
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
		{
			error("ERROR reading from socket");
		}
		int number;
		int success = sscanf(buffer, "%d", &number);
		//success will be 0 if the server didn't send a number
		if (buy){
			printf("[SERVER]: %s\n",buffer);
			if (success){
				//buy successful
				int j;
				for (j = 0; j <= i; j++){
					if (tickets[j] == 0){
						tickets[j] = number;
						break;
					}
				}
			}
			else
			printf("[CLIENT]: Buy failed\n");
		}
		else{
			if (success){
				//return successful
				printf("[SERVER]: RETURN %s\n[CLIENT]: Ticket %s returned\n",buffer, buffer);
			}
			else
			printf("[SERVER]: %s\n[CLIENT]: Returned failed\n",buffer);
			
			
		}
	}
	//close and print array
	close(sockfd);
	printf("[CLIENT]: Database Table:\n",buffer);
	for (i = 0; i < 15; i++){
		printf("[%d] %d\n", i, tickets[i]);
	}
	return 0;
}
