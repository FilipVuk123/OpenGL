
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 1024
#define PORT 8000

void error(char *s){
	perror(s);
	exit(1);
}

int main(void){
	struct sockaddr_in serveraddr, client;
	
	int s, i, slen = sizeof(client) , recv_len;
	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, 0)) < -1){
		error("socket");
	}
	printf("Socket created!\n");
	// zero out the structure
	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&serveraddr, sizeof(serveraddr) ) == -1){
		error("bind");
	}
	printf("Bind done!\n");
	while(1)
	{
		char buf[BUFLEN] = "\0";
		
		if ((recv_len = recv(s, buf, BUFLEN, 0)) == -1){
			error("recvfrom()");
		}
		printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		printf("%s\n", buf);
	}

	close(s);
	return 0;
}