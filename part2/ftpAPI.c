#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#define BUFFER_SIZE 1024
#define FTP_PORT 21

/* Does the inital connection to the ftp and create a socket */
int set_up_ftp(char *ip, int port){
    int sockfd;
    struct sockaddr_in server_addr;

    // TODO: Find a better way to do this conversions
    unsigned int addr = inet_addr(inet_ntoa(*((struct in_addr *) ip)));
    if(addr == -1){
        printf("inet_addr() error\n");
        printf("ip received: %s\n", ip);
        return -1;
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = addr;    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);                   /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }

    return sockfd;
}

/* Read from a ftp server and return the response code */
int ftp_read(int socket, char *resp, int size){
    int ret; 
    while(1){
        for(int i = 0; i<size; i++){
            ret = read(socket, &resp[i], 1);
            printf("%c", resp[i]);
            if(ret < 1)
                return -1;
            if(resp[i] == '\n'){
                break;
            }
        }
        if(resp[3] == ' ') break;
    }
    return atoi(resp);
}

/* Translate the passive mode return msg into a port */
int get_port(char *buffer){
    int a, b, c, d, pa, pb;
    char *rcv;
    rcv = strchr(buffer, '(');
    sscanf(rcv, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &pa, &pb);
    printf("Received: (%d,%d,%d,%d,%d,%d)\n", a, b, c, d, pa, pb);
    return pa*256 + pb;
}

void download(int socket, char *path){
    char *byte;
    char b;
    int ret;
    FILE *fp = fopen(path, "wb");
    do{
        ret = read(socket, byte, 1);
        b = *byte;
        fwrite(&b, 1, 1, fp);
    }while(ret > 0);
}
