#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#define BUFSIZE 6
#define WRITEFILE "/var/tmp/aesdsocketdata"

// My own signal handler thats terminates the server gracefully
static void signal_handler (int signal_number) {
    //remove file
    printf("Removing file\n");
    remove(WRITEFILE);
    printf("Caught signal, exiting\n");
    syslog(LOG_DEBUG, "Caught signal, exiting\n");
    exit(0);
}

// Register new signal handler
void register_signal () {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        printf("Error registering for SIGTERM");
    }
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        printf("Error registering for SIGINT");
    }
}

// Populate buf with received packet
// Contents in buf and bufsize mya be modified therefore using pointer to pointer
char* receive(int sockfd){
    char *buf = malloc(sizeof(char) * BUFSIZE);
    memset(buf, 0, BUFSIZE);
    int bufsize = BUFSIZE;
    if(!buf){
        //fprintf(stderr, "Malloc buffer failed\n");
        exit(-1);
    }
    int curr_index = 0;
    buf[0] = 0;
    while (1) {
        // Read 1 byte at a time
        int read_bytes = recv(sockfd, &buf[curr_index], 1, MSG_PEEK);
        read_bytes = recv(sockfd, &buf[curr_index], 1, 0);
        if (read_bytes == -1){
            //fprintf(stderr, "recv error\n");
            exit(-1);
        }
        curr_index++;
        // Check for buffer size
        if (curr_index >= bufsize) {
            buf = realloc(buf, bufsize * 2);
            memset(&buf[curr_index], 0, bufsize);
            bufsize *= 2;
        }
        if (buf[curr_index - 1] == '\n') {
            buf[curr_index] = 0;
            break;
        }
    }
    printf("End receiving\n");
    return buf;
}

// Write received message to /var/tmp/aesdsocketdata
// Create file if not exists
void write_received_data(char *buf) {
    FILE *wf = fopen(WRITEFILE, "a");
    if (!wf) {
        //fprintf(stderr, "Open write file error\n");
        exit(-1);
    }
    printf("Writing %s to "WRITEFILE"\n", buf);
    fprintf(wf, "%s", buf);
    fclose(wf);
    return;
}

void send_back(int sockfd) {
    char *buf = malloc(sizeof(char) * BUFSIZE);
    memset(buf, 0, BUFSIZE);
    int read_file_fd = open(WRITEFILE, O_RDONLY);
    int bufsize = BUFSIZE;
    if(!buf){
        //fprintf(stderr, "Malloc buffer failed\n");
        exit(-1);
    }
    int curr_index = 0;
    while (1) {
        // Read 1 byte at a time
        int read_bytes = read(read_file_fd, &buf[curr_index], 1);
        if (read_bytes == -1){
            //fprintf(stderr, "read error\n");
            exit(-1);
        }
        if (read_bytes == 0){
            buf[curr_index] = 0;
            break;
        }
        curr_index++;
        // Check for buffer size
        if (curr_index >= bufsize) {
            buf = realloc(buf, bufsize * 2);
            memset(&buf[curr_index], 0, bufsize);
            bufsize *= 2;
        }
    }
    close(read_file_fd);
    int bytes_sent = send(sockfd, buf, curr_index,  0);
    printf("Send:\n%s", buf);
    free(buf);
    printf("End sending, %d sent\n", bytes_sent);
    return;
}

int server(int daemon) {
    // Create socket
    int status;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int newfd;
    if (sockfd < 0) {
        fprintf(stderr, "Create socket error\n");
        exit(-1);
    }
    printf("Create socket success\n");
    // Setup socket option
    int opt = 1;
    socklen_t optlen = sizeof(opt);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, optlen);
    // Setup addr
    struct addrinfo *addr_struct;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, "9000", &hints, &addr_struct)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(-1);
    }
    // Assign address to socketsent =i
    if (bind(sockfd, addr_struct->ai_addr, sizeof(struct sockaddr))) {
        fprintf(stderr, "Bind error\n");
        freeaddrinfo(addr_struct);
        exit(-1);
    }
    freeaddrinfo(addr_struct);
    printf("Bind success\n");
    if(daemon) {
        pid_t pid = fork();
        if(pid == -1) {
            printf("Fork failed\n");
            exit(-1);
        }
        if (pid > 0) {
            printf("Kill parent process\n");
            exit(0);
        }
    }

    if (listen(sockfd, 10)) {
        //fprintf(stderr, "Listen error\n");
        exit(-1);
    }

    printf("Start listening on localhost:9000\n");
    while (1) {
        printf("Waiting for connections ...\n");
        struct sockaddr connect_addr;
        socklen_t addr_len = sizeof(struct sockaddr);
        memset(&connect_addr, 0, sizeof(struct sockaddr));
        newfd = accept(sockfd, &connect_addr, &addr_len);
        if (newfd < 0) {
            //fprintf(stderr, "Accept error\n");
            exit(-1);
        }
        struct sockaddr_in *addr_in = (struct sockaddr_in*) &connect_addr;
        // Do actions
        syslog(LOG_DEBUG, "Accepted connection form %s\n", inet_ntoa(addr_in->sin_addr));
        printf("Accepted connection from %s\n", inet_ntoa(addr_in->sin_addr));
        // Receive
        char* read_data = receive(newfd);
        printf("Received : %s", read_data);
        // Write received message to /var/tmp/aesdsocketdata
        write_received_data(read_data);
        free(read_data);
        printf("Data written to "WRITEFILE"\n");
        //Send back contents in /var/tmp/aesdsocketdata
        send_back(newfd);
        printf("Contents of aesdsocketdata sent back to client\n");
        printf("Closed connection from %s\n", inet_ntoa(addr_in->sin_addr));
        syslog(LOG_DEBUG, "Closed connection from %s\n", inet_ntoa(addr_in->sin_addr));
    }
    close(sockfd);
    printf("Server closed successfully\n");
    return 0;
}

int parse_arg(char *str) {
    if (strlen(str) != 2)
        return 0;
    if (str[1] != 'd')
        return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    //register signal handler
    register_signal();
    // syslog
    openlog(NULL, LOG_PID, LOG_USER);
    int daemon = 0;
    //parse argument, to make a daemon process
    //fork the process and kill the parent
    if (argc == 2 && parse_arg(argv[1])) {
        daemon = 1;
    }
    server(daemon);
    return 0;
}
