//
// Created by erianliang on 19-7-19.
//

#ifndef RANDOMSTRINGASSEMBLY_COMMUNICATOR_H
#define RANDOMSTRINGASSEMBLY_COMMUNICATOR_H

#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"
#include "entity/k_mer.h"

struct threeLists {
    int sourceRank;
    VertexList *vertexList;
    EdgeList *edgeList;
    SetOfID *tangleList;
};

typedef struct threeLists RecvArg;

struct item {
    int tarRank;
    char *pack;
    int packSize;

    item();

    virtual ~item();
};

typedef struct item Item;

void mainThreadTellRunning();

void mainThreadTellFinished(int thisRank, int world_size);

void requestRankToDoTest(int currank, int tarrank, const char *content);

void requestRankToStoreRead(int currank, int tarrank, const char *filename, size_t pos);

void requestOtherRanksToStoreEdge(int currank, int world_size,
                                  const char *value, EdgeId edgeId, ReadId readId, KMERPOS_t kmerpos);

void requestOtherRanksToStoreVertex(int currank, int world_size,
                                    VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode);

int processRecvRead(const char *filename, size_t pos);

int processRecvEdge(EdgeList *edgeList, char *value, ReadId readId, KMERPOS_t kmerpos);

// TODO: 记得tangle 判断
int processRecvVertex(VertexList *vertexList, SetOfID *tangleList, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode);

void *senderRunner(void *arg);

void *testReceiverRunner(void *arg);

void *receiverRunner(void *recvArg);

#endif //RANDOMSTRINGASSEMBLY_COMMUNICATOR_H
