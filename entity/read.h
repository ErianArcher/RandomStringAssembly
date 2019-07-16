//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_READ_H
#define RANDOMSTRINGASSEMBLY_READ_H

#include "idset.h"
#include <string>
#include <unordered_map>

// Some constants for KMERPOS_t
#define NOT_INCLUDE_KMER -1
#define START_KMER 0
#define INCLUDE_KMER 1
#define END_KMER 2

// Some constants for ALTERPOS_t
#define ALTER_HEAD 1
#define ALTER_TAIL 2
#define ALTER_BOTH ALTER_TAIL | ALTER_HEAD

// The directory to store reads by id
#define READ_DIR "./reads/"

using namespace std;

typedef size_t ReadId_t;

typedef int KMERPOS_t;

typedef int ALTERPOS_t;

int createRead(string read, ReadId_t &readId);

int getRead(string &oread, ReadId_t readId);

int enlargeRead(string newRead, int increasedLength, ReadId_t readId, ALTERPOS_t alterationPos);

int cutRead(string newRead, int increasedLength, ReadId_t readId, ALTERPOS_t alterationPos);

/**
 * Check if the kMer is in this read, and if so then return its blurred position in the read.
 * @param kMerStr the string of k-mer
 * @param read the structure of Read
 * @return three types of positions or not in this read
 */
KMERPOS_t getKMerPositionInRead(string kMerStr, string readStr);

#endif //RANDOMSTRINGASSEMBLY_READ_H
