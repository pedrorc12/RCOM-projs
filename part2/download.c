#include "ftpAPI.c"
#include "parser.c"

int main(int argc, char **argv) {

    struct hostent *host;
    char *host_name, *file_path;
    // TODO: Not working the parameteres are null
    // parse(argc, argv, host_name, file);

    /* Parser code */
    char *addr = argv[1];
    char cmp[] = "ftp://";
    
    if(argc != 2){
        printf("Wrong number of inputs\n");
        exit(-1);
    }

    int i = 0;
    for(; i < strlen(cmp); i++){
        if(addr[i] != cmp[i]){
            printf("Wrong protocol\n");
            exit(-1);
        }
    }
    printf("addr i = %c\n", addr[i]);
    
    char login[BUFFER_SIZE];
    char pwd[BUFFER_SIZE];

    if(addr[i] == '['){
        i++;
        char buf[512];
        int counter = 0;
        for(; addr[i] != ':'; counter++, i++){
            buf[counter] = addr[i];
        }
        counter++;
        sprintf(login, "user %s\n", buf);
        printf("login = %s\n", login);
        memset(buf, 0, 512);
        counter = 0;
        i++;
        for(; addr[i] != '@'; counter++, i++){
            buf[counter] = addr[i];
        }
        i += 2;
        counter++;
        sprintf(pwd, "pass %s\n", buf);
        
    } else {
        strcpy(login, "user anonymous\n");
        strcpy(pwd, "pass anonymous\n");
    }
    int addressStart = i;

    printf("login = \"%s\"\npwd = \"%s\"\n", login, pwd);

    int j = 0;
    while(addr[i] != '/'){
        if(i >= strlen(addr) - 1) {
            j++;
            break;
        }
        j++;
        i++;
    }
    i++;


    host_name = (char *) malloc(sizeof(char) * (j));
    strncpy(host_name, addr+(addressStart), j);

    file_path = (char *) malloc(sizeof(char) * (strlen(addr) - i));
    strncpy(file_path, addr+i, strlen(addr) - i);


    printf("Input name is : \"%s\"\n", host_name);
    printf("file_path address : \"%s\"\n", file_path);


    if ((host = gethostbyname(host_name)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    printf("the host name is : %s\n", host->h_name);
    printf("the Ip is : %s\n", inet_ntoa(*((struct in_addr *) host->h_addr)));

    int cmd_socket = set_up_ftp(host->h_addr, FTP_PORT);

    char buffer[BUFFER_SIZE];
    int valread;
    
    if((valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE)) == -1){
        printf("Failed to read\n");
        exit(-1);
    }
    printf("return code: %d\n", valread);
    
    /* Authentication */
    
  
    int ret = write(cmd_socket, login, strlen(login));
    valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE);
    printf("return code: %d\n", valread);
    
    if(valread != 331)
        exit(-1);

    ret = write(cmd_socket, pwd, strlen(pwd));
    valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE);
    printf("return code: %d\n", valread);
    
    if(valread != 230)
        exit(-1);

    /* Entering passive mode */
    ret = write(cmd_socket, "pasv\n", strlen("pasv\n"));
    valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE);
    printf("return code: %d\n", valread);

    if(valread != 227)
        exit(-1);

    int port = get_port(buffer);
    printf("port = %d\n", port);
    
    /* Set up the data socket */
    int data_socket = set_up_ftp(host->h_addr, port);

    char *file_name;
    file_name = strrchr(file_path, '/');
    file_name++;
    printf("file name = %s\n", file_name);
    
    char cmd[] = "retr ";
    strcat(cmd, file_path);
    strcat(cmd, "\n");
    printf("cmd = %s", cmd);
    ret = write(cmd_socket, cmd, strlen(cmd));
    valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE);
    printf("return code: %d\n", valread);
    
    if(valread == 150) 
        download(data_socket, file_name);


    // just to end gracefuly
    ret = write(cmd_socket, "quit\n", strlen("quit\n"));
    valread = ftp_read(cmd_socket, buffer, BUFFER_SIZE);
    printf("return code: %d\n", valread);
    return 0;
}