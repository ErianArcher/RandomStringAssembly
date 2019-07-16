//
// Created by erianliang on 19-7-16.
//

#include "read.h"
#include <fstream>
#include <sstream>
#include <errno.h>
#include <iostream>
#include <cstring>

extern int errno;

using namespace std;

int createRead(string read, ReadId_t &readId) {
    readId = hash<string>()(read);
    stringstream ofile;
    ofile << READ_DIR << readId;
    ofstream outfstream;
    outfstream.open(ofile.str());
    if (!outfstream) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        return 0;
    }
    outfstream << read;
    if (!outfstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        outfstream.close();
        return 0;
    }
    outfstream.close();
    return 1;
}

int getRead(string &oread, ReadId_t readId) {
    stringstream ifile;
    ifile << READ_DIR << readId;
    ifstream infstream;
    infstream.open(ifile.str());
    if (!infstream) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        return 0;
    }
    infstream >> oread;
    if (!infstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        infstream.close();
        return 0;
    }
    infstream.close();
    return 1;
}

int enlargeRead(string newRead, int increasedLength, ReadId_t readId, ALTERPOS_t alterationPos) {
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

    if (!alterfstream.good()) {
        cerr << "File IO error: " << strerror(errno) << "\n";
        alterfstream.close();
        return 0;
    }
    alterfstream.close();
    return 1;
}

KMERPOS_t getKMerPositionInRead(string kMerStr, string readStr) {
    auto pos = readStr.find(kMerStr);
    auto readLen = readStr.length();
    auto kMerLen = kMerStr.length();
    auto endIndex = readLen - kMerLen;
    if (pos == 0) {
        return START_KMER;
    } else if (pos == -1) {
        return NOT_INCLUDE_KMER;
    } else {
        if (pos == endIndex) return END_KMER;
        else return INCLUDE_KMER;
    }
}
