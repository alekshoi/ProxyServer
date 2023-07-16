/*
 * This is a file that implements the operation on TCP sockets that are used by
 * all of the programs used in this assignment.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "connection.h"

// flytte fra connection.h og hit
void check_error(int res, char *msg){
    if(res == -1){
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

int is_valid_ip(char* ip_str) {
    struct in_addr addr;
    int result = inet_pton(AF_INET, ip_str, &addr);
    if (result <= 0) {
        perror("Invalid IP address");
        return -1;
    }
    return 0;
}



int tcp_connect( char* hostname, int port ){
    int moteplass_fd;
    struct sockaddr_in my_addr;

    moteplass_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_error(moteplass_fd, "socket");

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = inet_addr(hostname);

    // checking if it connects
    check_error(connect(moteplass_fd, (struct sockaddr*)&my_addr, sizeof(my_addr)), "connect");

    return moteplass_fd;
}

int tcp_read( int sock, char* buffer, int n )
{
    
    int rc = read(sock, buffer, n);

    if(rc ==-1){
        perror("Error reading from socket");
    }

    return rc;
    
}

int tcp_write(int sock, char* buffer, int bytes) {

    int wc = write(sock, (void*)buffer, bytes);
    if (wc == -1) {
        perror("Error writing to buffer");
        return -1;
    }
    return wc;
}


int tcp_write_loop( int sock, char* buffer, int bytes )
{
    int bytes_sent = 0;
    while (bytes_sent < bytes) {
        bytes_sent += write(sock, &buffer[bytes_sent], bytes - bytes_sent);
        if (bytes_sent == -1) {
            perror("Error writing to buffer");
            return -1;
        }
    }

    return bytes_sent;
}


void tcp_close( int sock )
{
    int rc = close(sock);
    if (rc == -1) {
        perror("Error closing socket");
    }
}


int tcp_create_and_listen( int port )
{
    int moteplass_sock, rc;
    struct sockaddr_in my_addr;

    moteplass_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    check_error(moteplass_sock, "socket");

    // memset(&my_addr, 0, sizeof(my_addr));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(moteplass_sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
    check_error(rc, "bind");

    rc = listen(moteplass_sock, 5); // 10?
    check_error(rc, "listen");

    // Binds the socket to the specified port and address, making it available for accepting connections.

    return moteplass_sock;
}


int tcp_accept( int server_sock )
{
    int new_sock = accept(server_sock, NULL, NULL);

    check_error(new_sock, "accept\n");
    
    return new_sock;
}

int tcp_wait( fd_set* waiting_set, int wait_end )
{
    int rc = select(wait_end, waiting_set, NULL, NULL, NULL);
    if(rc == -1){
        perror("Error select");
        return -1; 
    }
    
    return rc;
}

int tcp_wait_timeout( fd_set* waiting_set, int wait_end, int timeout )
{
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int rc = select(wait_end, waiting_set, NULL, NULL, &tv);


    for (int i = 0; i < FD_SETSIZE-1; i++) {
        if (FD_ISSET(i, waiting_set)) {
            return i;
        }
    }


    check_error(rc, "select error");
    
    return -1; 
}


