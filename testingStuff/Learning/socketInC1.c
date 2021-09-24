#include<stdio.h>
#include<string.h>
#include<sys/socket.h> 
#include<arpa/inet.h>
#include<unistd.h>

int main(int argc , char *argv[]){
	int socket_desc;
    struct sockaddr_in server;
    char *message, server_reply[10000];

    //Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0); // TCP connection
	if (socket_desc == -1) {
        printf("Could not create socket\n");
        return -1;
    }else printf("Socket created\n");
    
    server.sin_addr.s_addr = inet_addr("142.250.186.164");
    printf("%d\n",server.sin_addr.s_addr);
	server.sin_family = AF_INET;
    printf("%d\n", server.sin_family);
	server.sin_port = htons( 80 );
    printf("%d\n",server.sin_port);

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		printf("connect error\n");
		return -1;
	}else printf("Connected\n");
	
	//Send some data
	message = "GET / HTTP/1.1\r\n\r\n";
	if( send(socket_desc , message , strlen(message) , 0) < 0){
		puts("Send failed\n");
	}else puts("Data Send\n");

    //Receive a reply from the server
	if( recv(socket_desc, server_reply , 2000 , 0) < 0){
		puts("recv failed\n");
	} else puts("Reply received\n");
	puts(server_reply);
	
    close(socket_desc);
	return 0;
}

/*
// IPv4 AF_INET sockets:
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;          // load with inet_pton()
};

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};
*/