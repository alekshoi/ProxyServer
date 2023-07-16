/*
 * This is the main program for the proxy, which receives connections for sending and receiving clients
 * both in binary and XML format. Many clients can be connected at the same time. The proxy implements
 * an event loop.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"

#include <string.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#define MAX_CLIENTS 26 // 26 letter in the english alphabet A-Z
#define BUFSIZE 1000000 // this worked for one of the usernames



/* This struct should contain the information that you want
 * keep for one connected client.
 */
struct Client
{
    int socket;
    char id; // id
    char format; // type
};

typedef struct Client Client;

int num_clients = 0;
struct Client *client_list[MAX_CLIENTS];

void usage( char* cmd )
{
    fprintf( stderr, "Usage: %s <port>\n"
                     "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                     "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                     "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                     "\n",
                     cmd );
    exit( -1 );
}

/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleNewClient(int server_sock)
{   
    struct Client *new_client = malloc(sizeof(struct Client));
    if(new_client == NULL){
        perror("Error: unable to malloc \n");
        return;
    }

    int rc, new_socket = tcp_accept(server_sock);
    new_client->socket = new_socket;

    rc = tcp_read(new_socket, &(new_client->format), sizeof(char));    
    // checks if it can read
    if(rc ==-1){
        perror("Error: unable to read clients format: %s\n");
        return;    
    }

    rc = tcp_read(new_socket, &(new_client->id), sizeof(char));
    
    if(rc ==-1){
        perror("Error: unable to read clients id: %s\n");
        tcp_close(new_socket);
        free(new_client);
        return;
    }

    if(new_socket < 0){
        perror("Error: unable to accept connection");
        //fprintf(stderr, "Error: unable to accept connection: %s\n", strerror(errno));
        tcp_close(new_socket);
        free(new_client);
        return;     
    }

    if(num_clients==MAX_CLIENTS){
        perror("Error: Maxumimum number of clients reached");
        //fprintf(stderr, "Error: Maxumimum number of clients reached");
        tcp_close(new_socket);
        free(new_client);
        return;
    }

    else{
        client_list[num_clients] = new_client;
        num_clients++;
        printf("==========================================\n");
        printf("      New Client Connected\n");
        printf("==========================================\n");
        printf("Client ID: %c\n", new_client->id);
        printf("Client Format: %c\n", new_client->format);
        printf("Client Socket: %d\n", new_client->socket);
        printf("==========================================\n");  
    }
    
}

/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void removeClient(Client* client) {
    printf("==========================================\n");
    printf("      Client disconnected\n");
    printf("==========================================\n");
    printf("Client ID: %c\n", client->id);
    printf("Client Format: %c\n", client->format);
    printf("Client Socket: %d\n", client->socket);
    printf("==========================================\n");  

    int i;
    for (i = 0; i < num_clients; i++) {
        if (client_list[i]->socket == client->socket) {
            break;
        }
    }

    // Shift the remaining clients over to fill the gap
    for (int j = i; j < num_clients - 1; j++) {
        client_list[j] = client_list[j + 1];
    }

    tcp_close(client->socket);

    client_list[num_clients - 1] = NULL;

    // Decrement the number of clients
    num_clients--;

    // Clear the socket of the disconnected client
    free(client);
}


/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void forwardMessage(Record* msg)
{
    for (int i = 0; i < num_clients; i++) {
        if (client_list[i]->id == msg->dest) {
            // finds the destination of the message 
            // Convert the record to the format used by the client
            char *buf;
            
            int bytesSent; // bytesSent
            if (client_list[i]->format == 'X') {
                buf = recordToXML(msg, &bytesSent);
            } 
            else {
                buf = recordToBinary(msg, &bytesSent);
            }

            // Send the data to the client
            int ret = tcp_write(client_list[i]->socket, buf, bytesSent);
            
            if (ret < 0) {
                perror("Error: unable to send data to client\n");
            }

            // Free the converted data
            free(buf);

            // Delete the record
            deleteRecord(msg);
            return;
        }
    }

    // If we get here, no matching client was found, so just delete the record
    deleteRecord(msg);
}


/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleClient( Client* client )
{

    char buf[BUFSIZE];
    int rc = tcp_read(client->socket, buf, BUFSIZE);
    int bytesRead = 0; 
    if(rc==0){
        removeClient(client);
        return; 
    }

    else if(rc > 0){
        Record* record;
        
        if(client->format == 'X'){           
            record = XMLtoRecord(buf, BUFSIZE, &bytesRead);
        }
        else{           
            record = BinaryToRecord(buf, BUFSIZE, &bytesRead);
        }

        if(record){
            forwardMessage(record);
        }
        else{
            perror("Error: incorrectly or incomplete formatted message received from client\n");
            //fprintf(stderr, "Error: incorrectly or incomplete formatted message received from client\n");
        }
    }
    else{
        perror("Error: unable to read data from client\n");
        //fprintf(stderr, "Error: unable to read data from client: %s\n", strerror(errno));
    }
}

int main( int argc, char* argv[] )
{
    int port;
    int server_sock;

    if( argc != 2 )
    {
        usage( argv[0] );
    }

    port = atoi( argv[1] );

    server_sock = tcp_create_and_listen( port );
    if( server_sock < 0 ) exit( -1 );


    fd_set fds; 
    

    /* add your initialization code */
    
    /*
     * The following part is the event loop of the proxy. It waits for new connections,
     * new data arriving on existing connection, and events that indicate that a client
     * has disconnected.
     *
     * This function uses handleNewClient() when activity is seen on the server socket
     * and handleClient() when activity is seen on the socket of an existing connection.
     *
     * The loops ends when no clients are connected any more.
     */
    do{   
        FD_ZERO(&fds);
        FD_SET(server_sock, &fds);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_list[i] != 0) {
                FD_SET(client_list[i]->socket, &fds);
                
            }
        }
        
        int activity = tcp_wait(&fds, MAX_CLIENTS);

        if (activity == -1) {
            perror("Error: select");
            break;
        }

        if (FD_ISSET(server_sock, &fds)) {
            handleNewClient(server_sock);
        }
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_list[i] != NULL) {
                if (FD_ISSET(client_list[i]->socket, &fds)) {
                    handleClient(client_list[i]);
                }
            }
        }   
    } 
    
    while (num_clients > 0);

    printf("\n==========================================\n");
    printf("      Program Terminated Successfully\n");
    printf("==========================================\n");
    
    tcp_close(server_sock);

    return 0;
}


