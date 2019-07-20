//
// Created by 梁俊鹏 on 2019-07-20.
//

#ifndef RANDOMSTRINGASSEMBLY_READ_IO_H
#define RANDOMSTRINGASSEMBLY_READ_IO_H

#include <pthread.h>
#include <string>

using namespace std;

int readIOInit(int currank, int world_size, string filepath, string *filenames, int fileNum);

int getNextRead(string *outread, size_t *readpos);

string getCurrentFilename();

pthread_t requestWriteRead(const string &read);

void *writeReadRunner(void *arg);

#endif //RANDOMSTRINGASSEMBLY_READ_IO_H
