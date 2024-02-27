#include "proactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>
#include "server.h"
#include <signal.h>

#define SERVER_PORT 8090

pthread_mutex_t send_all_lock;
int* clientFDs;
int clientFDsSize = 10;
int clientFDsAdded = 0;

void error_exit(const char* error_message) 
{
    perror(error_message);
    exit(1);
}

//This function adds the client into the array of clientFDs
int add_into_clientFDs(int clientFD)
{
    for(int i = 0; i < 10; i++)
    {
        if(clientFDs[i] == -1)
        {
            clientFDs[i] = clientFD;
            return i;
        }
    }
    return -1;
}

// Function to check errors (values should not differ)
void check_operation_same(int result, const char* operation_name, int check_equal_to, int ClientSocket, int placementInServer) 
{
    if (result != check_equal_to) 
    {
        if (placementInServer!=-1) perror(operation_name);
        if (ClientSocket != -1) 
        {
            close(ClientSocket);
            remove_from_clientFDs(ClientSocket, placementInServer, 0);
        }
        pthread_exit(NULL);
    }
}

//This function removes the client from the array of clientFDs
void remove_from_clientFDs(int clientFD, int index,int client_exited)
{
    if(clientFDs[index] == clientFD || client_exited == 1)
    {
        clientFDs[index] = -1;
        clientFDsAdded--;
    }
    else perror("index does not match the clientFDs array\n");
}

// Function to close the client gracefully
void close_client_gracefully(void* args, int placementInServer)
{
    struct Proactor_args* actualArgs = (struct Proactor_args*)args;
    int clientSocket = actualArgs->Socket;
    remove_from_clientFDs(clientSocket, placementInServer, 1);
    free(args);
    pthread_exit(EXIT_SUCCESS);
}

//This function send the message to the client
void send_to(int clientSocket, char* message, int placementInServer) 
{
    uint32_t msg_len = strlen(message);
    uint32_t net_msg_len = htonl(msg_len);
    
    int send_result = send(clientSocket, &net_msg_len, sizeof(net_msg_len), 0);
    check_operation_same(send_result, "send", sizeof(net_msg_len), clientSocket, placementInServer);
    
    send_result = send(clientSocket, message, msg_len, 0);
    check_operation_same(send_result, "send", msg_len, clientSocket, placementInServer);
}

//This function recieve the message from the client
void recieve_from(int clientSocket, char** message_ptr, int placementInServer) 
{
    uint32_t msg_len;
    int recv_result = recv(clientSocket, &msg_len, sizeof(msg_len), 0);
    check_operation_same(recv_result, "recv", sizeof(msg_len), clientSocket, placementInServer);

    msg_len = ntohl(msg_len);
    *(message_ptr) = malloc(msg_len + 1);
    

    recv_result = recv(clientSocket, *(message_ptr), msg_len, 0);
    check_operation_same(recv_result, "recv", msg_len, clientSocket, placementInServer);

    (*(message_ptr))[msg_len] = '\0';

    return;
}

// Deals with sending a message to all clients
void send_all(char* message, int clientSocket, int placementInServer)
{
    pthread_mutex_lock(&send_all_lock);
    for(int i = 0; i < clientFDsSize; i++)
    {
        if(clientFDs[i] != -1 && clientFDs[i] != clientSocket)
        {
            send_to(clientFDs[i], message, placementInServer);
        }
    }
    pthread_mutex_unlock(&send_all_lock);
}

// Funtion to handle the client
void* handleClient(void* args) 
{
    struct Proactor_args* actualArgs = (struct Proactor_args*)args;
    int clientSocket = actualArgs->Socket;
    if (clientFDsSize == clientFDsAdded) 
    {
        clientFDs = realloc(clientFDs, clientFDsSize * sizeof(int));
        memset(clientFDs + clientFDsSize, -1, clientFDsSize * sizeof(int));
        clientFDsSize *= 2;
    }
    int placementInServer = add_into_clientFDs(clientSocket);
    clientFDsAdded++;

    char* message;
    message = "Welcome! You have connected to the chat! Now you may send messages!\n";
    send_to(clientSocket, message, placementInServer);

    while (1)
    {
        if (clientSocket == -1) break;
        recieve_from(clientSocket, &message, placementInServer);
        if(strcmp(message, "exit") == 0)
        {
            free(message);
            close_client_gracefully(args, placementInServer);
        }
        else
        {
            int client_size = floor(log10(abs(clientSocket)) + 1);
            char* new_message = malloc(strlen(message) + client_size + 20);
            strcat(new_message, "Client ");
            sprintf(new_message + 7, "%d", clientSocket);
            strcat(new_message, ": ");
            strcat(new_message, message);
            send_all(new_message, clientSocket, placementInServer);
            free(message);
            free(new_message);
        }
    }
    free(args);
    return NULL;
}

// Function to check for errors in socket operations
void check_socket_operation(int result, const char* operation_name, int check_equal_to, int ClientSocket) 
{
    if (result == check_equal_to) 
    {
        perror(operation_name);
        if (ClientSocket != -1) 
        {
            close(ClientSocket);
        }
        error_exit(operation_name);
    }
}

// Creates a server socket and checks for any errors during creation
void createServerSocket(int* serverSocket) 
{
    int domain = AF_INET;
    *serverSocket = socket(domain, SOCK_STREAM, 0);
    check_socket_operation(*serverSocket, "socket", -1, -1);
}

// Sets options for the server socket and checks for any errors
void setSocketOptions(int serverSocket) 
{
    int optVal = 1;
    int setsockopt_result = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    check_socket_operation(setsockopt_result, "setsockopt", -1, -1);
}

// Binds the server socket to an address and starts listening for connections
// Checks for any errors during binding and listening
void bindAndListen(int serverSocket, struct sockaddr_in addr) 
{
    int bind_result = bind(serverSocket, (struct sockaddr *) &addr, sizeof(addr));
    check_socket_operation(bind_result, "bind", -1, -1);

    int listen_result = listen(serverSocket, 5);
    check_socket_operation(listen_result, "listen", -1, -1);
}

// Opens a TCP server: creates a server socket, sets socket options,
// binds the socket to an address, starts listening for connections,
// and accepts incoming connections
void openTcpServer(int argc, char *const *argv) 
{
    int serverSocket;
    createServerSocket(&serverSocket);
    printf("Server socket created\n");
    setSocketOptions(serverSocket);
    printf("Socket options set\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bindAndListen(serverSocket, addr);
    printf("Server socket bound and listening\n");

    struct Proactor_args* args = (struct Proactor_args*)malloc(sizeof(struct Proactor_args));
    args->Socket = serverSocket;
    args->handle = handleClient;

    proactor(args);

    free(clientFDs);
}

// Function to cleanup and exit the server
void cleanup_and_exit(int signo)
{
    char* message = "\nServer is shutting down, bye bye! (recv is going to fail)\n";
    sleep(1);
    for(int i = 0; i < clientFDsSize; i++)
    {
        if(clientFDs[i] != -1)
        {
            send_to(clientFDs[i], message, i);
            close(clientFDs[i]);
        }
    }
    free(clientFDs);
    exit(0);
}

int main()
{
    clientFDs = malloc(clientFDsSize * sizeof(int));
    memset(clientFDs, -1, clientFDsSize * sizeof(int));
    signal(SIGINT, cleanup_and_exit);
    openTcpServer(0, NULL);
    return 0;
}