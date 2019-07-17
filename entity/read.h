//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_READ_H
#define RANDOMSTRINGASSEMBLY_READ_H

#include "idset.h"
#include <string>
#include <unordered_map>

// Some constants for KMERPOS_t
#define NOT_INCLUDE_KMER 0u
#define START_KMER 1u
#define INCLUDE_KMER 2u
#define END_KMER 4u

// Some constants for ALTERPOS_t
#define ALTER_HEAD 1u
#define ALTER_TAIL 2u
#define ALTER_BOTH (ALTER_TAIL | ALTER_HEAD)

// The directory to store reads by id
#define READ_DIR "./reads/"

using namespace std;

typedef size_t ReadId;

typedef unsigned int KMERPOS_t;

typedef unsigned int ALTERPOS_t;

int createRead(string read, ReadId &readId);

int getRead(string &oread, ReadId readId);

int enlargeRead(string newRead, int increasedLength, ReadId readId, ALTERPOS_t alterationPos);

int cutRead(string newRead, int increasedLength, ReadId readId, ALTERPOS_t alterationPos);

/**
 * Check if the kMer is in this read, and if so then return its blurred position in the read.
 * 潜在问题：同一个k-mer被同一个edge经过多次
 * @param kMerStr the string of k-mer
 * @param read the structure of Read
 * @return three types of positions or not in this read
 */
KMERPOS_t getKMerPositionInRead(string kMerStr, string readStr);

#endif //RANDOMSTRINGASSEMBLY_READ_H
