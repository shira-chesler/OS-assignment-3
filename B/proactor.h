#ifndef PROACTOR_H
#define PROACTOR_H

typedef void* (*Func)(void* args);

typedef struct Proactor_args
{
    int Socket;
    Func handle;
} Proactor_args;

void* proactor(void* args);

void* proactor_accept(void* args);

#endif