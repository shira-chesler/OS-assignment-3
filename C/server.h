#ifndef SERVER_H
#define SERVER_H

void error_exit(const char* error_message);

int add_into_clientFDs(int clientFD);

void check_operation_same(int result, const char* operation_name, int check_equal_to, int ClientSocket, int placementInServer);

void remove_from_clientFDs(int clientFD, int index,int client_exited);

void close_client_gracefully(void* args, int placementInServer);

void send_to(int clientSocket, char* message, int placementInServer);

void recieve_from(int clientSocket, char** message_ptr, int placementInServer);

void send_all(char* message, int clientSocket, int placementInServer);

void close_every_client();

void* handleClient(void* args) ;

void check_socket_operation(int result, const char* operation_name, int check_equal_to, int ClientSocket) ;

void createServerSocket(int* serverSocket) ;

void setSocketOptions(int serverSocket) ;

void bindAndListen(int serverSocket, struct sockaddr_in addr) ;

void openTcpServer(int argc, char *const *argv) ;

void cleanup_and_exit(int signo);

#endif