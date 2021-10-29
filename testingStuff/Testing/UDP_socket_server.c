
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 512
#define PORT 8000

void die(char *s){
	perror(s);
	exit(1);
}

int main(void){
	struct sockaddr_in serveraddr, client;
	
	int s, i, slen = sizeof(client) , recv_len;
	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		die("socket");
	}
	
	// zero out the structure
	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&serveraddr, sizeof(serveraddr) ) == -1){
		die("bind");
	}
	
	while(1)
	{
		char buf[BUFLEN] = "\0";
		fprintf(stderr, "Waiting for data...");
		
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &client, &slen)) == -1){
			die("recvfrom()");
		}
		
		printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		printf("Data: %s\n" , buf);
		
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &client, slen) == -1){
			die("sendto()");
		}
	}

	close(s);
	return 0;
}