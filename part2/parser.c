typedef struct {
    char *host_name;
    char *file;
} Input;

void parse(int argc, char **argv, char *host_name, char *file) {
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

    int j = 0;
    while(addr[i] != '/' && i < strlen(addr) ){
        j++;
        i++;
    }
    i++;

    host_name = (char *) malloc(sizeof(char) * (j));
    strncpy(host_name, addr+(strlen(cmp)), j);

    file = (char *) malloc(sizeof(char) * (strlen(addr) - i));
    strncpy(file, addr+i, strlen(addr) - i);
}