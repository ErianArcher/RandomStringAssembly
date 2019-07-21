//
// Created by erianliang on 19-7-19.
//

#include "communicator.h"
#include "blocking_queue.h"
#include "read_io.h"
#include <string.h>
#include <mpi.h>
#include <stdint.h>
#include <limits.h>
#include <fstream>
#include <csignal>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define BUFFER_SIZE 128

using namespace std;

block_queue<Item *> *blockQueue = new block_queue<Item *>(1024);

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
   #error "what is happening here?"
#endif

#define DONOTHIN_OP 0
#define EXIT_OP 1
#define READ_STORE_OP 11
#define EDGE_STORE_OP 21
#define VERTEX_STORE_OP 31

#define PREFIX_READ_OP 1
#define PREFIX_EDGE_OP 2
#define PREFIX_VERTEX_OP 3

#define TAG(source, sink) (source) * 1000 + (sink)

#define MAIN_THREAD_RUNNING 1
#define MAIN_THREAD_WAITING 0

static int mainStatus = MAIN_THREAD_RUNNING;

void mainThreadTellRunning() {
    mainStatus = MAIN_THREAD_RUNNING;
}

void mainThreadTellFinished(int thisRank, int world_size) {
    Item *itemStop = nullptr;
    int packSizeStop = 0;
    int opStop = EXIT_OP;
    char *buf = nullptr;
    for (int i = 0; i < world_size; ++i) {
        if (i == thisRank) continue;
        itemStop = new Item;
        buf = new char[BUFFER_SIZE];
        MPI_Pack(&opStop, 1, MPI_INT, buf, BUFFER_SIZE, &packSizeStop, MPI_COMM_WORLD);
        itemStop->packSize = packSizeStop;
        itemStop->tarRank = i;
        itemStop->pack = buf;
        if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
        blockQueue->push(itemStop);
        itemStop = nullptr;
        buf = nullptr;
        packSizeStop = 0;
    }
    mainStatus = MAIN_THREAD_WAITING;
}

void requestRankToDoTest(int currank, int tarrank, const char *content) {
    char *buf = new char[BUFFER_SIZE];
    int packSize = 0;
    Item *item = new Item;
    MPI_Pack(content, strlen(content), MPI_CHAR, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&tarrank, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    item->tarRank = tarrank;
    item->packSize = packSize;
    item->pack = buf;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    //cout << blockQueue->size() << endl;
    blockQueue->push(item);
}

void requestRankToStoreRead(int currank, int tarrank, const char *filename, size_t pos) {
    char *buf = new char[BUFFER_SIZE];
    int packSize = 0;
    int strLen = strlen(filename);
    int op = READ_STORE_OP;
    Item *item = new Item;
    MPI_Pack(&op, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&strLen, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(filename, strLen, MPI_CHAR, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&pos, 1, my_MPI_SIZE_T, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);

    // Push into blocking queue
    item->tarRank = tarrank;
    item->packSize = packSize;
    item->pack = buf;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    blockQueue->push(item);
}

void requestOtherRanksToStoreEdge(int currank, int world_size, const char *value, EdgeId edgeId, ReadId readId,
                                  KMERPOS_t kmerpos) {
    char *buf = new char[BUFFER_SIZE];
    int packSize = 0;
    int strLen = strlen(value);
    int op = EDGE_STORE_OP;
    int tarrank = edgeId % world_size;
    if (tarrank == currank) {
        cerr << "Putting a edge belonging to this host to other hosts." << endl;
        return;
    }
    Item *item = new Item;
    MPI_Pack(&op, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&strLen, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(value, strLen, MPI_CHAR, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&readId, 1, my_MPI_SIZE_T, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&kmerpos, 1, MPI_UNSIGNED, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);

    // DEBUG
    //MPI_Send(&packSize, 1, MPI_INT, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);
    //MPI_Send(buf, packSize, MPI_PACKED, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);
    item->tarRank = tarrank;
    item->pack = buf;
    item->packSize = packSize;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    blockQueue->push(item);
}

void
requestOtherRanksToStoreVertex(int currank, int world_size, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {
    char *buf = new char[BUFFER_SIZE];
    int packSize = 0;
    int op = VERTEX_STORE_OP;
    int tarrank = vertexId % world_size;
    if (tarrank == currank) {
        cerr << "Putting a vertex belonging to this host to other hosts." << endl;
        return;
    }
    Item *item = new Item;
    MPI_Pack(&op, 1, MPI_INT, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&vertexId, 1, my_MPI_SIZE_T, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&edgeId, 1, my_MPI_SIZE_T, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&vertexMode, 1, MPI_UNSIGNED, buf, BUFFER_SIZE, &packSize, MPI_COMM_WORLD);

    // DEBUG
    //MPI_Send(&packSize, 1, MPI_INT, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);
    //MPI_Send(buf, packSize, MPI_PACKED, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);

    item->tarRank = tarrank;
    item->pack = buf;
    item->packSize = packSize;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    blockQueue->push(item);
}

// 该方法负责char *指针的delete
int processRecvRead(const char *filename, size_t pos) {
    ifstream infile(filename);
    string read;
    infile.seekg(pos);
    getline(infile, read);
    infile.close();
    delete filename;
    if (requestWriteRead(read) == 0) return 0;
    return 1;
}

// 该方法负责char *指针的delete
int processRecvEdge(EdgeList *edgeList, char *value, ReadId readId, KMERPOS_t kmerpos) {
    string strValue(value);

    VertexId sourceVertex = getId(strValue.substr(0, strValue.size()-1));
    VertexId sinkVertex = getId(strValue.substr(1, strValue.size()-1));
    int flag = addNewEdge(edgeList, value, sourceVertex, sinkVertex, readId, kmerpos);
    delete value;
    return flag;
}

int processRecvVertex(VertexList *vertexList, SetOfID *tangleList, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {
    unsigned int flag = 0;
    if (HEAD_VERTEX == vertexMode)
        flag = addVertex(vertexList, vertexId, 0, edgeId, vertexMode);
    else if (TAIL_VERTEX == vertexMode)
        flag = addVertex(vertexList, vertexId, edgeId, 0, vertexMode);

    if (MULTI_OUT_DEGREE == (flag & MULTI_OUT_DEGREE)) {
        tangleList->safe_insert(vertexId);
    }
    return flag;
}

void *senderRunner(void *arg) {
    int world_size;
    int thisRank;
    int tarRank;
    char *pack = nullptr;
    int packSize = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &thisRank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int tag;
    Item *item;
    bool terminate = false;
    while (true) {
        // 退出sender的条件
        if (MAIN_THREAD_WAITING == mainStatus && blockQueue->empty()) break;

        blockQueue->pop(&item);
        tarRank = item->tarRank;
        pack = item->pack;
        packSize = item->packSize;
        tag = TAG(thisRank, tarRank);
        if (nullptr != pack) {
            // 直接使用宏定义来代替packSize，以使用多个sender。
            //MPI_Send(&packSize, 1, MPI_INT, tarRank, tag, MPI_COMM_WORLD);
            MPI_Send(pack, packSize, MPI_PACKED, tarRank, tag, MPI_COMM_WORLD);
        }
        delete item;
        item = nullptr;
    }
    return (void *) 0;
}

void *testReceiverRunner(void *arg) {
    int thisRank;
    int sourceRank = *((int *)arg);
    char *pack = nullptr;
    int packSize = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &thisRank);
    int tag = TAG(sourceRank, thisRank);
    MPI_Status status;
    while (true) {
        cout << "recevier" << thisRank << "start." << endl;
        MPI_Recv(&packSize, 1, MPI_INT, sourceRank, tag, MPI_COMM_WORLD, &status);
        pack = new char[packSize];
        MPI_Recv(pack, packSize, MPI_INT, sourceRank, tag, MPI_COMM_WORLD, &status);
        cout << "Content:(" << packSize << ")" <<  string(pack) << endl;
        delete[] pack;
    }
    return (void *) 0;
}

void *receiverRunner(void *args) {
    RecvArg *recvArg = (RecvArg *)args;
    EdgeList *edgeList = recvArg->edgeList;
    VertexList *vertexList = recvArg->vertexList;
    SetOfID *tangleList = recvArg->tangleList;
    int thisRank;
    int sourceRank = recvArg->sourceRank;
    char *pack = nullptr;
    int packSize = BUFFER_SIZE;
    MPI_Comm_rank(MPI_COMM_WORLD, &thisRank);
    int tag = TAG(sourceRank, thisRank);
    MPI_Status status;
    while (true) {
        // Store op type
        int op = DONOTHIN_OP;
        int position = 0;
        int op_prefix;

        //MPI_Recv(&packSize, 1, MPI_INT, sourceRank, tag, MPI_COMM_WORLD, &status);
        pack = new char[packSize];
        MPI_Recv(pack, packSize, MPI_PACKED, sourceRank, tag, MPI_COMM_WORLD, &status);

        MPI_Unpack(pack, packSize, &position, &op, 1, MPI_INT, MPI_COMM_WORLD);
        op_prefix = op / 10;
        if (DONOTHIN_OP == op) {
            // Do nothing
        } else if (EXIT_OP == op) {
            break;
        } else if (PREFIX_READ_OP == op_prefix) {
            int strLen = 0;
            char *str = nullptr;
            MPI_Unpack(pack, packSize, &position, &strLen, 1, MPI_INT, MPI_COMM_WORLD);
            str = new char[strLen];
            MPI_Unpack(pack, packSize, &position, str, strLen, MPI_CHAR, MPI_COMM_WORLD);
            if (READ_STORE_OP == op) {
                size_t pos = 0;
                MPI_Unpack(pack, packSize, &position, &pos, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
                processRecvRead(str, pos);
            }
        } else if (PREFIX_EDGE_OP == op_prefix) {
            int strLen = 0;
            char *kmer = nullptr;
            ReadId readId;
            MPI_Unpack(pack, packSize, &position, &strLen, 1, MPI_INT, MPI_COMM_WORLD);
            kmer = new char[strLen];
            if (strLen != 3) {
                cout << "Trasmission error: " << strLen << endl;
            }
            MPI_Unpack(pack, packSize, &position, kmer, strLen, MPI_CHAR, MPI_COMM_WORLD);
            MPI_Unpack(pack, packSize, &position, &readId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
            if (EDGE_STORE_OP == op) {
                KMERPOS_t kmerpos;
                MPI_Unpack(pack, packSize, &position, &kmerpos, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
                processRecvEdge(edgeList, kmer, readId, kmerpos);
            }
        } else if (PREFIX_VERTEX_OP == op_prefix) {
            VertexId vertexId;
            EdgeId edgeId;
            MPI_Unpack(pack, packSize, &position, &vertexId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
            MPI_Unpack(pack, packSize, &position, &edgeId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
            if (VERTEX_STORE_OP == op) {
                VertexMode_t vertexMode;
                MPI_Unpack(pack, packSize, &position, &vertexMode, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
                processRecvVertex(vertexList, tangleList, vertexId, edgeId, vertexMode);
            }
        }
        delete[] pack;
    }
    return (void *) 0;
}

item::~item() {
    if (nullptr != pack) {
        delete[] pack;
        pack = nullptr;
    }
}

item::item() {
    pack = nullptr;
    tarRank = -1;
    packSize = 0;
}

#pragma clang diagnostic pop