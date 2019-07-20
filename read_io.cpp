//
// Created by 梁俊鹏 on 2019-07-20.
//

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "read_io.h"
#include "entity/read.h"

#define BUFFER_SIZE 152

using namespace std;

//block_queue<string> *ioqueue = new block_queue<string>;

static int thisRank;
static int host_num;
static string path;
static string *fnames;
static int fileAmount = 0;
static int curIndex4Fnames;
static fpos_t curSeek;
static char buffer[BUFFER_SIZE];
static FILE *infile;

int moveToNext() {
    fclose(infile);
    curSeek = 0ll;
    if (curIndex4Fnames + 1 >= fileAmount) {
        cout << "All files are read." << endl;
        return 0;
    }
    string fname = path + fnames[++curIndex4Fnames];
    infile = fopen(fname.c_str(), "r");
    if (!infile) {
        cerr << "Unable to open file: " << fname << endl;
        return -1;
    }
    return 1;
}

int readIOInit(int currank, int world_size, string filepath, string *filenames, int fileNum) {
    thisRank = currank;
    host_num = world_size;
    path = filepath;
    fnames = filenames;
    fileAmount = fileNum;
    curIndex4Fnames = -1; // 因为moveToNext需要直接增加
    curSeek = 0ll;
    if (nullptr == filenames) {
        cerr << "No input read file." << endl;
        return -1;
    }
    while (moveToNext() == -1);
    if (curIndex4Fnames > fileAmount) return -1;
    if (createReadDir() == 0) {
        exit(1);
    }
    return 0;
}

int getNextRead(string *outread, size_t *readpos) {
    if (fileAmount < 1) return -1;
    // 先尝试读取，如果不能读取则转向下一个文件
    while (fgets(buffer, BUFFER_SIZE, infile) == NULL) {
        int flag;
        while ((flag = moveToNext()) == -1) ; // 当读到的文件不能打开则马上打开下一个
        if (flag == 0) return -1; // 全部读完则返回-1
    }
    *readpos = curSeek; // 读取前的位置
    fgetpos(infile, &curSeek);

    int len = strlen(buffer) - 1;
    *outread = string(buffer, len);

    return 1;
}

string getCurrentFilename() {
    return fnames[curIndex4Fnames];
}

pthread_t requestWriteRead(const string &read) {
    pthread_t tid;
    if (pthread_create(&tid, NULL, writeReadRunner, (void *)&read) == -1) {
        cerr << "Error: Cannot create thread when requesting read writing." << endl;
        return 0;
    }
    pthread_detach(tid);
    return tid;
}

void *writeReadRunner(void *arg) {
    string read = *((string *) arg);
    if (createRead(read, nullptr) == 0) {
        cerr << "Error: Cannot create file for read \"" << read << "\"" << endl;
        return (void *) -1;
    }
    return (void *)0;
}
