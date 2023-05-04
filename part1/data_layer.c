#include "data_layer.h"

// global variables

int package_number;
struct termios oldtio;


int llopen(int port, short int actor)
{

    int tries = ATTEMPTS;
    int res;
    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    unsigned char response[S_FRAME_SIZE];

    package_number = 0;
    set_up_alarm();

    while (tries)
    {
        if (actor == SENDER)
        {
            //sendig a set_up msg for the receiver
            create_supervision_frame(response, END_SEND, SET);
            res = write(port, response, S_FRAME_SIZE);
            printf("Sent a SET msg\n");

            //waits and check the response
            if (parse_frame(port, frame) == TRUE) {
                if (frame->c == UA && frame->type == SUPERVISION)
                {
                    printf("UA received\n");
                    free(frame);
                    return TRUE;
                }
            }
            
        }
        else if (actor == RECEIVER)
        {
            //Waits for a msg and parse it
            if (parse_frame(port, frame) == TRUE) {

                //if received a SET respond with UA
                if (frame->c == SET && frame->type == SUPERVISION)
                {
                    create_supervision_frame(response, END_SEND, UA);
                    printf("response 0x%x %x %x %x %x\n", response[0], response[1], response[2], response[3], response[4]);
                    res = write(port, response, S_FRAME_SIZE);

                    free(frame);
                    return TRUE;
                }
            }
        }
        tries--;
    }
    free(frame);
    return -1;
}

int llwrite(int fd, unsigned char *buffer, int length)
{

    if (length > MAX_FRAME_SIZE || length < 0)
        return FALSE;

    unsigned char frame[MAX_FRAME_SIZE];
    int frame_size = create_information_frame(package_number, buffer, length, END_SEND, frame);

    struct frame *msg = (struct frame *)malloc(sizeof(struct frame));

    int tries = ATTEMPTS;
    while (tries)
    {
        printf("sendig:");
        print_buffer(frame, frame_size);
        if (write(fd, frame, frame_size) < 0)
        {
            perror("Error writing the frame. Try again after the timeout\n");
            continue;
        }
        else
        {
            //DEBUG
            //printf("Sent frame with PacketNumber=%d\n", package_number);
            if (parse_frame(fd, msg) == TRUE) {
                if (msg->c == RR(0) || msg->c == RR(1)) {
                    //if (error == 0) continue;
                    package_number = (++package_number) % 2;
                    printf("postive ACK Received\n");
                    break;
                }
            }
            tries--;
        }
    }
    if (tries == 0)
    {
        printf("timeout, giving up...\n");
        exit(-1);
    }
    return frame_size;
}


int read_error = 0;
int p = 2;
int llread(int fd, unsigned char *buffer)
{
    unsigned char processed_buffer[MAX_FRAME_SIZE];
    int processed_buffer_size;
    struct frame *information_frame = (struct frame *)malloc(sizeof(struct frame));
    int tries = ATTEMPTS;
    while (tries)
    {
        if (parse_frame(fd, information_frame) == TRUE)
            break; 

        tries--;
        printf("\n\ntries = %d\n\n", tries);
    }
    if (tries == 0) {
        printf("\ntimeout, giving up...\n");
        exit(-1);
    }

    /*if (read_error == 0) {
        information_frame->bcc2 = 0;
    }*/
    //read_error = (read_error + 1) % p;

    processed_buffer_size = byte_clearing(information_frame->data, information_frame->data_length, processed_buffer);

    //DEBUG
    // printf("inside processed buffer: ");
    // print_buffer(processed_buffer, processed_buffer_size);

    printf("bcc2 in the frame = 0x%x\n", information_frame->bcc2);
    if (information_frame->bcc2 == create_bcc2(processed_buffer, processed_buffer_size))
    {
        printf("package number: %d\n", package_number);
        if (information_frame->c >> 7 == package_number)
        {
            package_number = (++package_number) % 2;
            memcpy(buffer, processed_buffer, processed_buffer_size);
            create_supervision_frame(processed_buffer, END_SEND, RR(package_number));
            //if(read_error == 0){
            //usleep(1000*200);
            //}
            write(fd, processed_buffer, S_FRAME_SIZE);
            printf("Sent a POS ACK\n");
        }
        else {
            create_supervision_frame(processed_buffer, END_SEND, RR(package_number));
            write(fd, processed_buffer, S_FRAME_SIZE);
            printf("Sent a POS ACK\n");
            
            processed_buffer_size = 0;
        }
    }
    else
    {
        printf("Failed the bcc2\n");
        create_supervision_frame(processed_buffer, END_SEND, REJ(package_number));
        write(fd, processed_buffer, S_FRAME_SIZE);
        printf("Sent a REJ\n\n");

        processed_buffer_size = 0;
    }

    free(information_frame->data);
    free(information_frame);
    return processed_buffer_size;
}

int llclose(int fd, short int actor)
{
    int tries = ATTEMPTS;
    int res;
    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    unsigned char response[S_FRAME_SIZE];

    package_number = 0;
    create_supervision_frame(response, END_SEND, DISC);
    set_up_alarm();

    while (tries != 0)
    {
        if (actor == SENDER)
        {   
            res = write(fd, response, S_FRAME_SIZE);
            printf("DISC sent\n");
            if (parse_frame(fd, frame) == TRUE) {
                if (frame->c == DISC)
                {
                    printf("DISC received\n");
                    create_supervision_frame(response, END_SEND, UA);
                    res = write(fd, response, S_FRAME_SIZE);
                    sleep(DELAY);
                    printf("UA sent\n");
                    free(frame);
                    tcsetattr(fd, TCSANOW, &oldtio);
                    close(fd);
                    return TRUE;
                }
            } 
        }
        else if (actor == RECEIVER)
        {
            if(parse_frame(fd, frame) == TRUE) {
                if (frame->c == DISC)
                {
                    printf("DISC received\n");
                    res = write(fd, response, S_FRAME_SIZE);
                    printf("DISC sent\n");
                    if (parse_frame(fd, frame) == TRUE) {
                        if (frame->c == UA)
                        {
                            printf("UA received\n");
                            free(frame);
                            tcsetattr(fd, TCSANOW, &oldtio);
                            close(fd);
                            return TRUE;
                        }
                    }
                }
            }
        }
        tries--;
    }
    free(frame);
    return -1;
}

int create_supervision_frame(unsigned char *frame, unsigned char addressField, unsigned char controlField)
{
    frame[0] = FLAG;
    frame[1] = addressField;
    frame[2] = controlField;
    frame[3] = frame[1] ^ frame[2];
    frame[4] = FLAG;

    return 0;
}

int create_information_frame(unsigned short int package_number, unsigned char *data, int data_size, unsigned char address_field, unsigned char *frame)
{
    unsigned char *frame_info = (unsigned char *)malloc(MAX_FRAME_SIZE);

    frame_info[0] = address_field;

    if (package_number == 1)
        frame_info[1] = BIT(7);
    else
        frame_info[1] = 0;

    frame_info[2] = frame_info[0] ^ frame_info[1]; //BCC1

    int i;
    for (i = 3; i < data_size + 3; i++)
    {
        frame_info[i] = data[i - 3];
    }
    frame_info[i] = create_bcc2(data, data_size); //BCC2
    int length = i + 1;

    int frame_info_bs_length = byte_stuffing(frame_info, length, frame + 1);
    length = frame_info_bs_length + 1;

    frame[0] = FLAG;
    frame[length - 1] = FLAG;

    printf("created frame:\n ");
    for (int i = 0; i < length; i++)
    {
        printf(" %x", frame[i]);
    }
    printf("\n");

    return length;
}

int parse_frame(int fd, struct frame *frame)
{

    unsigned char buffer[MAX_FRAME_SIZE];
    unsigned char *msg = (unsigned char *)malloc(sizeof(unsigned char));
    int counter = 0;
    int res;
    genericState state = START;

    set_alarm_time(TIMEOUT);

    while ((state != STOP))
    {
        if (timeout) {
            printf("parse_frame timeout\n");
            return FALSE;
        }

        res = read(fd, msg, 1);
        if (res < 0) {
            printf("Error read in parse_frame\n");
            sleep(DELAY);
            continue;
        }

        switch (state)
        {
        case START:
            if (*msg == FLAG && errno != EAGAIN)
            {
                counter = 0;
                set_alarm_time(TIMEOUT);
                state = FLAG_RCV;
            }
            break;

        case FLAG_RCV:
            if (*msg == END_SEND || *msg == END_REC)
            {
                frame->a = *msg;
                state = A_RCV;
            }
            else
                state = START;
            break;

        case A_RCV:
            frame->c = *msg;
            state = C_RCV;
            break;

        case C_RCV:
            if (frame->a ^ frame->c == *msg)
            {
                state = BCC_OK;
                frame->bcc1 = *msg;
            }
            else
                state = START;
            break;

        case BCC_OK:
            if (*msg == FLAG)
            {
                state = DATA_RCV;
            }
            else
            {
                if (res > 0) {
                    //DEBUG
                    //printf("buffer[%d] = %x\n", counter, *msg);
                    buffer[counter] = *msg;
                    counter++;
                }

                if (counter >= MAX_FRAME_SIZE) {
                    printf("parse_frame false, counter = %d\n", counter);
                    return FALSE;
                }
            }
            break;

        case DATA_RCV:
            frame->bcc2 = buffer[counter - 1];
            if (counter == 0)
                frame->type = SUPERVISION;
            else
                frame->type = INFORMATION;

            state = STOP;
            break;
        }
    }

    unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * (counter - 1));

    if (frame->type == INFORMATION)
    {
        frame->data = data;
        frame->data_length = counter - 1;

        memcpy(data, buffer, counter - 1);

        //DEBUG
        //printf("inside data:");
        //print_buffer(data, counter - 1);
    }
    else
        free(data);

    free(msg);
    turn_alarm_off();
    return TRUE;
}

int set_up_termios(char *port)
{
    struct termios newtio;
    int fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        perror(port);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* not blocking */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");
    return fd;
}

unsigned char create_bcc2(unsigned char *data, int data_lenght)
{
    unsigned char bcc2 = 0;

    //DEBUG
    //printf("information passed to BCC\n");
    for (size_t i = 0; i < data_lenght; i++)
    {
        //printf("%x ", data[i]);
        bcc2 ^= data[i];
    }
    //printf("\nbcc2 = 0x%x\n", bcc2);

    return bcc2;
}
