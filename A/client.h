#ifndef CLIENT_H
#define CLIENT_H

int openClient();

void send_to(int clientSocket, char* message) ;

void recieve_from(int clientSocket, char** message_ptr);

void listen_to_keyboard(void* args);

void listen_to_server(void* args);

void cleanup_and_exit(int signo);

void check_operation_same(int result, const char* operation_name, int check_equal_to);

void check_operation_differ(int result, const char* operation_name, int check_equal_to);

#endif