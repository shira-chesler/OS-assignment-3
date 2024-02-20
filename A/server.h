#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

void error_exit(const char* msg);

void check_operation_same(int result, const char* operation_name, int check_equal_to, int ClientSocket, int placementInServer);

void check_socket_operation(int result, const char* operation_name, int check_equal_to, int ClientSocket);

void send_to(int clientSocket, char* message, int placementInServer);

void recieve_from(int clientSocket, char** message_ptr, int placementInServer);

int add_into_clientFDs(int clientFD);

void remove_from_clientFDs(int clientFD, int index, int client_exited);

void send_all(char* message, int clientSocket, int placementInServer);

void* handleClient(void* args);

void createServerSocket(int* serverSocket);

void setSocketOptions(int serverSocket);

void bindAndListen(int serverSocket, struct sockaddr_in addr);

void acceptConnections(int serverSocket);

void openTcpServer(int argc, char *const *argv);

void cleanup_and_exit(int signo);

#endif