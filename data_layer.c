#include "data_layer.h"

// global variables
struct termios oldtio_sender;
struct termios oldtio_receiver;

struct linkLayer
{
    int package_number;
} data_link;

int newtio_setup(int fd)
{
    struct termios newtio; // Create new termios struct

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; //The CS<number> fields set how many data bits are transmitted per byte across the serial port.
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,... no everything) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* not blocking */

    /* 
        VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
        leitura do(s) proximo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");
}

int llopen(char *port, short int actor)
{
    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */

    int fd = open(port, O_RDWR | O_NOCTTY); //open the serial port device
    if (fd < 0)
    {
        perror(port);
        exit(-1);
    }

    if (actor == SENDER){
        if (tcgetattr(fd, &oldtio_sender) == -1){ 
            /* save current port settings */
            perror("tcgetattr");
            exit(-1);
        }
    }
    else if (actor == RECEIVER){
        if (tcgetattr(fd, &oldtio_sender) == -1){ 
            /* save current port settings */
            perror("tcgetattr");
            exit(-1);
        }
    }
    newtio_setup(fd);

    int tries = ATTEMPTS;
    int res;
    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    unsigned char response[S_FRAME_SIZE];

    data_link.package_number = 0;
    set_up_alarm();
    while (tries)
    {
        if (actor == SENDER)
        {
            //sendig a set_up msg for the receiver
            create_supervision_frame(response, END_SEND, SET);
            res = write(fd, response, S_FRAME_SIZE);
            printf("Sent a SET msg\n");

            //waits and check the response
            parse_frame(fd, frame);
            if (frame->c == UA && frame->type == SUPERVISION)
            {
                printf("Received a UA\n");
                free(frame);

                return fd;
            }
        }
        else if (actor == RECEIVER)
        {
            //Waits for a msg and parse it
            parse_frame(fd, frame);

            //if received a SET respond with UA
            if (frame->c == SET && frame->type == SUPERVISION)
            {
                create_supervision_frame(response, END_SEND, UA);
                printf("response 0x%x %x %x %x %x\n", response[0], response[1], response[2], response[3], response[4]);
                res = write(fd, response, S_FRAME_SIZE);
                free(frame);

                return fd;
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
    int frame_size = create_information_frame(data_link.package_number, buffer, length, END_SEND, frame);

    struct frame *msg = (struct frame *)malloc(sizeof(struct frame));
    
    while (TRUE)
    {

        if (write(fd, frame, frame_size) < 0)
        {
            printf("Error writing the frame. Try again after the timeout\n");
            continue;
        }
        else
        {
            printf("Sent frame with PacketNumber=%d\n", data_link.package_number);
            printf("sent ");
            for (int i = 0; i < frame_size; i++)
            {
                printf(" %x", frame[i]);
            }
            printf("\n");
            parse_frame(fd, msg);
            if (msg->c == RR(0) || msg->c == RR(1))
            {
                data_link.package_number = (++data_link.package_number) % 2;
                printf("postive ACK Received\n");
                break;
            }
        }
    }

}

int llread(int fd, unsigned char *buffer)
{
    unsigned char processed_buffer[MAX_FRAME_SIZE];
    int processed_buffer_size;
    struct frame *information_frame = (struct frame *)malloc(sizeof(struct frame));
    while(parse_frame(fd, information_frame) != TRUE);
        

    processed_buffer_size = byte_clearing(information_frame->data, information_frame->data_length, processed_buffer);
    printf("inside processed buffer: ");
    for (int i = 0; i < processed_buffer_size; i++)
    {
        printf(" %x", processed_buffer[i]);
    }
    printf("\n");

    if (information_frame->bcc2 == create_bcc2(processed_buffer, processed_buffer_size))
    {
        if (information_frame->c >> 7 == data_link.package_number)
            data_link.package_number = (++data_link.package_number) % 2;
        strcpy(buffer, processed_buffer);
        printf("passed the bcc2 checker\n");
        create_supervision_frame(processed_buffer, END_SEND, RR(data_link.package_number));
        write(fd, processed_buffer, S_FRAME_SIZE);
        printf("Sent a UA\n");
    }
    else
    {
        printf("Failed the bcc2\n");
        create_supervision_frame(processed_buffer, END_SEND, REJ(data_link.package_number));
        write(fd, processed_buffer, S_FRAME_SIZE);
        printf("Sent a REJ\n");
    }

    free(information_frame->data);
    free(information_frame);
    return processed_buffer_size;
}

//TODO: attention to edge cases
int llclose(int fd, short int actor)
{
    int tries = ATTEMPTS;
    int res;
    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    unsigned char response[S_FRAME_SIZE];

    data_link.package_number = 0;
    create_supervision_frame(response, END_SEND, DISC);
    set_up_alarm();

    while (tries != 0)
    {
        if (actor == SENDER)
        {
            res = write(fd, response, S_FRAME_SIZE);
            printf("Sent a DISC\n");
            parse_frame(fd, frame);
            if (frame->c == DISC)
            {
                printf("received a DISC senting a UA\n");
                create_supervision_frame(response, END_SEND, UA);
                res = write(fd, response, S_FRAME_SIZE);
                free(frame);

                tcsetattr(fd, TCSANOW, &oldtio_sender);
                close(fd);

                return TRUE;
            }
        }
        else if (actor == RECEIVER)
        {
            parse_frame(fd, frame);
            if (frame->c == DISC)
            {
                printf("received a DISC senting a DISC\n");
                res = write(fd, response, S_FRAME_SIZE);
                parse_frame(fd, frame);
                if (frame->c == UA)
                {
                    free(frame);

                    tcsetattr(fd, TCSANOW, &oldtio_receiver);
                    close(fd);

                    return TRUE;
                }
            }
        }
    }

    free(frame);
    return -1;
}

int create_supervision_frame(unsigned char *frame, unsigned char addressField, unsigned char controlField)
{
    // TODO: The parameters that we will pass here need to be verified in the function call (SET, UA, RR, REJ...)
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
    for (i = 3; i < data_size + 2; i++)
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
    unsigned char res;
    unsigned char *msg = (unsigned char *)malloc(sizeof(unsigned char));
    int counter = 0;
    genericState state = START;

    set_alarm_time(TIMEOUT);

    while ((state != STOP))
    {
        if (timeout)
            return FALSE;

        //TODO: Deal with reads error and weird behaviour (recomendation read the manual entry, >man read)
        res = read(fd, msg, 1) < 1;
        switch (state)
        {
        case START:
            if (*msg == FLAG && errno != EAGAIN)
            {
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
                printf("received a header with C = 0x%x\n", frame->c);
                state = DATA_RCV;
            }
            else
            {
                buffer[counter] = *msg;
                counter++;
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

        strcpy(data, buffer);
    }
    else
        free(data);

    free(msg);
    turn_alarm_off();
    return TRUE;
}

unsigned char create_bcc2(unsigned char *data, int data_lenght)
{
    unsigned char bcc2 = 0;
    printf("information passed to BCC\n");
    for (size_t i = 0; i < data_lenght; i++)
    {   
        printf("%x", data[i]);
        bcc2 ^= data[i];
    }
    printf("\nbcc2 = 0x%x\n\n", bcc2);
    return bcc2;
}