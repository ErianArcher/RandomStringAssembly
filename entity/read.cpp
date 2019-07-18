//
// Created by erianliang on 19-7-16.
//

#include "read.h"
#include <fstream>
#include <sstream>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

extern int errno;

using namespace std;

int createRead(const string read, ReadId *readId) {
    *readId = hash<string>()(read);
    stringstream ofile;
    ofile << READ_DIR << *readId;
    ofstream outfstream;
    outfstream.open(ofile.str());
    if (!outfstream) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        return 0;
    }
    outfstream << read;
    /*if (!outfstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        outfstream.close();
        return 0;
    }*/
    outfstream.close();
    return 1;
}

int getRead(string *oread, ReadId readId) {
    stringstream ifile;
    ifile << READ_DIR << readId;
    ifstream infstream;
    infstream.open(ifile.str());
    if (!infstream) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        return 0;
    }
    infstream >> *oread;
    /*if (!infstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        infstream.close();
        return 0;
    }*/
    infstream.close();
    return 1;
}

int enlargeRead(string newRead, int increasedLength, ReadId readId, ALTERPOS_t alterationPos) {
    stringstream afile;
    afile << READ_DIR << readId;
    ofstream alterfstream;
    if (ALTER_HEAD == alterationPos || ALTER_BOTH == alterationPos) {
        alterfstream.open(afile.str(), ofstream::out | ofstream::trunc);
        if (!alterfstream) {
            cerr << "File IO error: " << strerror(errno) << "\n";
            return 0;
        }
        alterfstream << newRead;
    } else if (ALTER_TAIL == alterationPos) {
        alterfstream.open(afile.str(), ofstream::out | ofstream::app);
        if (!alterfstream) {
            cerr << "File IO error: " << strerror(errno) << "\n";
            return 0;
        }
        alterfstream << newRead.substr(newRead.length() - increasedLength, increasedLength);
    } else {
        cerr << "Unknown operation for altering read: #" << readId << "\n";
        return 0;
    }

    /*if (!alterfstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        alterfstream.close();
        return 0;
    }*/
    alterfstream.close();
    return 1;
}

int cutRead(const string& newRead, int reducedLength, ReadId readId, ALTERPOS_t alterationPos) {
    // 不管什么方法都重写文件
    stringstream afile;
    afile << READ_DIR << readId;
    ofstream alterfstream;
    alterfstream.open(afile.str());
    if (!alterfstream) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        return 0;
    }
    alterfstream << newRead;
    /*
    if (!alterfstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        alterfstream.close();
        return 0;
    }*/
    alterfstream.close();
    return 1;
}

KMERPOS_t getKMerPositionInRead(const string& kMerStr, const string& readStr) {
    auto pos = readStr.find(kMerStr);
    auto reversePos = readStr.rfind(kMerStr);
    auto readLen = readStr.length();
    auto kMerLen = kMerStr.length();
    auto endIndex = readLen - kMerLen;
    KMERPOS_t kmerpos = NOT_INCLUDE_KMER;
    if (reversePos == pos) {
        // 只有一个或没有kmer 的情况
        if (pos == 0) {
            kmerpos |= START_KMER;
        } else if (pos == -1) {
            kmerpos |= NOT_INCLUDE_KMER;
        } else {
            if (pos == endIndex) kmerpos |= END_KMER;
            else kmerpos |= INCLUDE_KMER;
        }
    } else {
        // 有两个或以上匹配的kmer
        // 判断头部匹配的kmer在哪
        if (pos == 0) {
            kmerpos |= START_KMER;
        } else {
            kmerpos |= INCLUDE_KMER;
        }
        // 判断尾部匹配的kmer在哪
        if (reversePos == endIndex) {
            kmerpos |= END_KMER;
        } else {
            kmerpos |= INCLUDE_KMER;
        }

        // 如果还有第三个kmer在一头一尾的两个kmer之间，这说明kmer还处以该read 的中间
        auto midpos = readStr.substr(pos+kMerLen, reversePos-pos-kMerLen).find(kMerStr);
        if (midpos != -1)
            kmerpos |= INCLUDE_KMER;
    }
    return kmerpos;
}

int createReadDir() {
    string path(READ_DIR);
    int len = path.length();
    char *tmpPath = new char[len];
    for (int i = 0; i < len; i++) {
        tmpPath[i] = '\0';
    }
    for (int i = 0; i < len; i++) {
        if (path[i] == '\\' || path[i] == '/') {
            if (access(tmpPath, 0) == -1) {
                cout << "Prompt: " << tmpPath << " doesn't exist." << endl;
                cout << "Prompt: " << "Making directory " << tmpPath << endl;
                if (mkdir(tmpPath, 0777) != 0) {
                    cerr << "Error: Cannot create a directory " << tmpPath << ". " << strerror(errno) << endl;
                    return 0;
                }
            }
        }
        tmpPath[i] = path[i];
    }
    return 1;
}
