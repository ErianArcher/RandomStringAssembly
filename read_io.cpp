//
// Created by 梁俊鹏 on 2019-07-20.
//

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "read_io.h"
#include "entity/read.h"

#define BUFFER_SIZE 152
#define MB (1024*1024)

using namespace std;

//block_queue<string> *ioqueue = new block_queue<string>;

static int thisRank;
static int host_num;
static string path;
static string *fnames;
static int fileAmount = 0;
static int curIndex4Fnames;
static off_t curSeek;
static off_t stopSeek;
static char buffer[BUFFER_SIZE];
static FILE *infile;

int moveToNext() {
    if (NULL != infile)
        fclose(infile);
    //curSeek.__pos = 0;

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

    fseek(infile, 0, SEEK_END);
    off_t totalSize = ftello(infile);
    stopSeek = (thisRank+1==host_num)?totalSize: totalSize / host_num * (thisRank+1);
    if (thisRank == 0) {
        curSeek = 0;
        fseeko(infile, curSeek, SEEK_SET);
    } else {
        curSeek = totalSize / host_num * thisRank;
        if (curSeek < 1) {
            cerr << "Error on computing file position for #" << thisRank << endl;
            return -1;
        }
        fseeko(infile, curSeek - 1, SEEK_SET); // 定位到属于该worker的前一个位置，以防止curSeek就是一条完整的read的开端的情况
        while (getc(infile) != '\n'); // 把不完整的read过滤掉
        curSeek = ftello(infile);
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
    infile = NULL;
    //curSeek.__pos = 0;
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
    while (curSeek >= stopSeek) {
        int flag;
        while ((flag = moveToNext()) == -1) ; // 当读到的文件不能打开则马上打开下一个
        if (flag == 0) return -1; // 全部读完则返回-1
    }
    fgets(buffer, BUFFER_SIZE, infile);
    *readpos = curSeek; // 读取前的位置
    curSeek = ftello64(infile);

    int len = strlen(buffer) - 1; // 去除fgets中的换行符
    buffer[len] = '\0';
    outread->clear();
    outread->append(buffer);
    printf("\r%ldMB", (stopSeek - curSeek) / MB);
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
