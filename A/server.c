#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include "server.h"

#define SERVER_PORT 8080
// struct to pass the arguments to the thread
struct HandleClientArgs {
    int* clientFD;
    pthread_mutex_t* send_mutex;
    pthread_mutex_t* recv_mutex;
};

// Function to check for errors in socket operations
void error_exit(const char* msg) 
{
    perror(msg);
    printf("Exiting...\n");
    exit(EXIT_FAILURE);
}

// Function to check errors (values should not differ)
void check_operation_same(int result, const char* operation_name, int check_equal_to, int ClientSocket) 
{
    if (result != check_equal_to) 
    {
        perror(operation_name);
        if (ClientSocket != -1) 
        {
            char* message = "500 INTERNAL ERROR\r\n\r\n"; 
            send_to(ClientSocket, message);
            close(ClientSocket);
        }
        exit(EXIT_FAILURE);
    }
}

// Function to check for errors in socket operations
void check_socket_operation(int result, const char* operation_name, int check_equal_to, int ClientSocket) 
{
    if (result == check_equal_to) 
    {
        perror(operation_name);
        if (ClientSocket != -1) 
        {
            char* message = "500 INTERNAL ERROR\r\n\r\n"; 
            send_to(ClientSocket, message);
            close(ClientSocket);
        }
        exit(EXIT_FAILURE);
    }
}

//This function send the message to the client
void send_to(int clientSocket, char* message, pthread_mutex_t* send_mutex) 
{
    uint32_t msg_len = strlen(message);
    uint32_t net_msg_len = htonl(msg_len);
    
    pthread_mutex_lock(send_mutex);
    int send_result = send(clientSocket, &net_msg_len, sizeof(net_msg_len), 0);
    check_operation_same(send_result, "send", sizeof(net_msg_len), clientSocket);
    
    send_result = send(clientSocket, message, msg_len, 0);
    check_operation_same(send_result, "send", msg_len, clientSocket);
    pthread_mutex_unlock(send_mutex);
}

void send_as_server(int clientSocket, char* message, pthread_mutex_t* send_mutex) 
{
    char* server_sending = "Server: ";
    strcat(server_sending, message);
    send_to(clientSocket, server_sending, send_mutex);
}

//This function recieve the message from the client
void recieve_from(int clientSocket, char** message_ptr, pthread_mutex_t* recv_mutex) 
{
    uint32_t msg_len;

    pthread_mutex_lock(recv_mutex);
    int recv_result = recv(clientSocket, &msg_len, sizeof(msg_len), 0);
    check_operation_same(recv_result, "recv", sizeof(msg_len), clientSocket);

    msg_len = ntohl(msg_len);
    *(message_ptr) = malloc(msg_len + 1);
    

    recv_result = recv(clientSocket, *(message_ptr), msg_len, 0);
    check_operation_same(recv_result, "recv", msg_len, clientSocket);

    (*(message_ptr))[msg_len] = '\0';
    pthread_mutex_unlock(recv_mutex);

    return;
}

// Funtion to handle the client
void handleClient(void* args) 
{
    // while 1:
    //  recieve from (returns fd of the client that wants to comunicate with. If returns -1, client wants to wait for others to send to it)
    //  if not an fd but "Quit": break from outer loop
    //  if not -1: 
    //      while 1:
    //          recieve from (returns the message that the client wants to send to the other client)
    //          if got message: 
    //              if message is "end": break from inner loop
    //              send to (sends to the client that wants to comunicate with.)
    //          else:
    //              continue
    // close the client socket
    struct HandleClientArgs* actualArgs = (struct HandleClientArgs*)args;
    int clientSocket = *(actualArgs->clientFD);
    pthread_mutex_t* send_mutex = actualArgs->send_mutex;
    pthread_mutex_t* recv_mutex = actualArgs->recv_mutex;
    
    while(1)
    {
        char* message;
        message = "Enter the client socket (fd number) you want to communicate with, or Quit to end the chat:\n";
        send_as_server(clientSocket, message, send_mutex);
        recieve_from(clientSocket, &message, recv_mutex);
        if(strcmp(message, "Quit") == 0)
        {
            free(message);
            break;
        }
        else
        {
            if(!isnumber(message))
            {
                free(message);
                message = "Invalid input.\n";
                send_as_server(clientSocket, message, send_mutex);
                continue;
            }

            int clientSocket2 = atoi(message);

            if (clientSocket2 == -1) // client wants to wait for others to send to it
            {
                free(message);
                continue;
            }
            else if (fcntl(clientSocket2, F_GETFD) == -1) // client is not available
            {
                free(message);
                message = "The client you want to communicate with is not available.\n";
                send_as_server(clientSocket, message, send_mutex);
                continue;
            }
            else if (clientSocket2 == clientSocket) // client cannot communicate with itself
            {
                free(message);
                message = "You cannot communicate with yourself.\n";
                send_as_server(clientSocket, message, send_mutex);
                continue;
            }
            else
            {
                free(message);
                message = "Enter the message you want to send, or End to end the chat:\n";
                send_as_server(clientSocket, message, send_mutex);
                while(1)
                {
                    recieve_from(clientSocket, &message, recv_mutex);
                    if(strcmp(message, "End") == 0)
                    {
                        free(message);
                        break;
                    }
                    else
                    {
                        send_to(clientSocket2, message, send_mutex);
                        free(message);
                    }
                }
            }
        }
    
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

// Accepts incoming connections and handles them in separate threads
// Checks for any errors during accepting connections
void acceptConnections(int serverSocket) 
{
    while (1) 
    {
        struct sockaddr_in client_addr;
        socklen_t addrLen = sizeof(client_addr);
        int clientFD = accept(serverSocket, (struct sockaddr *) &client_addr, &addrLen);
        if (clientFD == -1) {
            perror("accept failed");
            continue;
        }

        printf("New client accepted.\n");
        printf("Client details:\n");

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("IP Address: %s\n", clientIP);
        printf("Port: %d\n", ntohs(client_addr.sin_port));

        // Handle client in a new thread
        pid_t pid;
        struct HandleClientArgs* args = malloc(sizeof(struct HandleClientArgs));
        args->clientFD = &clientFD;
        pthread_mutex_init(args->send_mutex, NULL);
        pthread_mutex_init(args->recv_mutex, NULL);

        pthread_t thread;
        if(pthread_create(&thread, NULL, handleClient, args) != 0) 
        {
            perror("pthread_create");
            close(clientFD);
            free(args);
        }
        pthread_detach(thread);
        free(args);
        close(clientFD);
    }
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
    acceptConnections(serverSocket);

    close(serverSocket);
}
