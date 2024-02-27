#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "proactor.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

int serverSocket = -1;

// Function to create a new proactor
void* proactor(void* args)
{
    struct Proactor_args* actualArgs = (struct Proactor_args*)args;
    serverSocket = actualArgs->Socket;

    pthread_t thread;
    pthread_create(&thread, NULL, proactor_accept, args);
    
    void* ret_val;
    pthread_join(thread, &ret_val);
}

// Function to accept new connections
void* proactor_accept(void* args)
{
    struct Proactor_args* actualArgs = (struct Proactor_args*)args;
    int serverSocket = actualArgs->Socket;
    int clientSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            perror("accept");
            exit(1);
        }

        printf("\nNew client accepted.\n");
        printf("Client details:\n");

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("IP Address: %s\n", clientIP);
        printf("Port: %d\n", ntohs(clientAddr.sin_port));
        
        struct Proactor_args* newArgs = (struct Proactor_args*)malloc(sizeof(struct Proactor_args));
        newArgs->Socket = clientSocket;
        newArgs->handle = actualArgs->handle;//does not matter, I will never touch it anyway

        pthread_t thread;
        if(pthread_create(&thread, NULL, *(actualArgs->handle), newArgs) != 0)
        {
            perror("pthread_create");
            continue;
        }

        pthread_detach(thread);
        //sleep(1);
    }
}