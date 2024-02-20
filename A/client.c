#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_PORT 8090
#define SERVER_IP "127.0.0.1"
#define MAX_MESSAGE_SIZE 1024

int clientSocket = -1;

struct thread_args {
    int clientSocket;
};

// Function to check errors (values should not differ)
void check_operation_same(int result, const char* operation_name, int check_equal_to) 
{
    if (result != check_equal_to) 
    {
        fprintf(stderr, "%s failed\n", operation_name);
        cleanup_and_exit(0);
    }
}

// Function to check errors (values should not be same)
void check_operation_differ(int result, const char* operation_name, int check_equal_to) 
{
    if (result == check_equal_to) 
    {
        perror(operation_name);
        cleanup_and_exit(0);
    }
}

// Function to open a client socket
int openClient()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    check_operation_differ(sock, "socket", -1);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) 
    {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server!\n");

    return sock;
}

//Sends a message to the server
void send_to(int clientSocket, char* message) 
{
    uint32_t msg_len = strlen(message);
    uint32_t net_msg_len = htonl(msg_len);

    int send_result = send(clientSocket, &net_msg_len, sizeof(net_msg_len), 0);
    check_operation_same(send_result, "send", sizeof(net_msg_len));
    
    send_result = send(clientSocket, message, msg_len, 0);
    check_operation_same(send_result, "send", msg_len);
}

//Receives a message from the server
void recieve_from(int clientSocket, char** message_ptr) 
{
    uint32_t msg_len;
    int recv_result = recv(clientSocket, &msg_len, sizeof(msg_len), 0);
    check_operation_same(recv_result, "recv", sizeof(msg_len));

    msg_len = ntohl(msg_len);
    *(message_ptr) = malloc(msg_len + 1);
    

    recv_result = recv(clientSocket, *(message_ptr), msg_len, 0);
    check_operation_same(recv_result, "recv", msg_len);

    (*(message_ptr))[msg_len] = '\0';

    return;
}

// Function to listen to keyboard input (with thread)
void listen_to_keyboard(void* args) 
{
    int clientSocket = ((struct thread_args*)args)->clientSocket;
    char* message = malloc(MAX_MESSAGE_SIZE * sizeof(char));
    while (1) 
    {
        fgets(message, MAX_MESSAGE_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;
        if (strcmp(message, "exit") == 0)
        {
            cleanup_and_exit(0);
        }
        send_to(clientSocket, message);
    }
}

// Function to listen to server (with thread)
void listen_to_server(void* args) 
{
    int clientSocket = ((struct thread_args*)args)->clientSocket;
    char* message;
    while (1) 
    {
        recieve_from(clientSocket, &message);
        printf("%s\n", message);
        free(message);
    }
}

// Function to cleanup and exit (terminate gracefully)
void cleanup_and_exit(int signo) 
{
    if(clientSocket == -1) 
    {
        exit(signo);
    }
    char* message = "exit";
    send_to(clientSocket, message);
    printf("\nSystem: Said bye bye to server, now exiting\n");
    exit(signo);
}

int main()
{
    signal(SIGINT, cleanup_and_exit);
    clientSocket = openClient();
    pthread_t keyboard_thread;
    pthread_t server_thread;
    struct thread_args args;
    args.clientSocket = clientSocket;
    pthread_create(&keyboard_thread, NULL, (void*)listen_to_keyboard, (void*) &args);
    pthread_create(&server_thread, NULL, (void*)listen_to_server, (void*) &args);

    pthread_join(keyboard_thread, NULL);
    pthread_join(server_thread, NULL);

    exit(0);
}

