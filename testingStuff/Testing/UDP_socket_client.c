
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "10.220.27.98"
#define BUFLEN 512	
#define PORT 8000	

void die(char *s){
	perror(s);
	exit(1);
}

int main(void){
	struct sockaddr_in serveraddr;
	int s, i, slen=sizeof(serveraddr);

	if ( (s=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		die("socket");
	}

	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	
	if (inet_aton(SERVER , &serveraddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	while(1){
		char buf[BUFLEN]= "\0";
		char message[BUFLEN]= "\0";
		printf("Enter message : ");
		fgets(message, sizeof(message), stdin);
		
		//send the message

		if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &serveraddr, slen)==-1){
			die("sendto()");
		}
	}

	close(s);
	return 0;
}