#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "127.0.0.1"
#define BUFLEN 1024	//Max length of buffer
#define PORT 8888	//The port on which to send data

void die(char *s)
{
	perror(s);
	exit(1);
}

int main(void){
    char message[BUFLEN];
	struct sockaddr_in serveraddr;
	int s, i, slen=sizeof(serveraddr);

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		die("socket");
	}

	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
	
	if (inet_aton(SERVER , &serveraddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	while(1){
        
		printf("Enter message : ");
		gets(message);
		
		//send the message
		if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &serveraddr, slen)==-1){
			die("sendto()");
		}
	}

	close(s);
	return 0;
}
