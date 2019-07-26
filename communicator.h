//
// Created by erianliang on 19-7-19.
//

#ifndef RANDOMSTRINGASSEMBLY_COMMUNICATOR_H
#define RANDOMSTRINGASSEMBLY_COMMUNICATOR_H

#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"
#include "entity/k_mer.h"
#include <mpi.h>

#if SIZE_MAX == UCHAR_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
#define my_MPI_SIZE_T MPI_UNSIGNED_LONG
//#error "what is happening here?"
#endif

#define DONOTHIN_OP 0
#define EXIT_OP 1
#define READ_STORE_OP 11
#define EDGE_STORE_OP 21
#define EDGE_QUERY_OP 22
#define EDGE_FULL_QUERY_OP 23
#define VERTEX_STORE_OP 31
#define VERTEX_QUERY_OP 32

#define PREFIX_READ_OP 1
#define PREFIX_EDGE_OP 2
#define PREFIX_VERTEX_OP 3

// QUERY STATUS
#define FAILED_QUERY 0
#define SUCCESSFUL_QUERY 1
#define TANGLE_QUERY 31
#define SINK_QUERY 32


#define TAG(source, sink) (source) * 1000 + (sink)

#define QUERY_PACK_SIZE 12
// 只能容纳100个host
#define QUERY_TAG(querySource, querySink) (querySource) * 10000 + (querySink) * 100

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

void sendSuperContigToRankHead(int headrank, int currank, string &superContig);

string reduceSuperContigFromOthers(int currank, int world_size, string &superContig);

int queryEdgeById(string *pEdgeStr, int currank, int tarrank, EdgeId edgeId);

int queryOutEdgeOfVertexById(EdgeId *pEdgeId, int currank, int tarrank, VertexId vertexId);

int queryFullEdgeById(string *pString, int currank, int tarrank, EdgeId edgeId);

#endif //RANDOMSTRINGASSEMBLY_COMMUNICATOR_H
