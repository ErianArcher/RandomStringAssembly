//
// Created by erianliang on 19-7-19.
//

#include "communicator.h"
#include "blocking_queue.h"
#include "read_io.h"
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <fstream>
#include <csignal>
#include <sstream>

#define BUFFER_SIZE 128

using namespace std;

block_queue<Item *> *blockQueue = new block_queue<Item *>(1024*1024);

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
        if (!blockQueue->push(itemStop)) cerr << "Cannot push stop signal." << endl;
        itemStop = nullptr;
        buf = nullptr;
        packSizeStop = 0;
    }

    // TODO: sender 数量大于receiver 数量时会出现阻塞现象。
    if (1 == world_size) {
        itemStop = new Item;
        blockQueue->push(itemStop);
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
    int strLen = getK();
    int op = EDGE_STORE_OP;
    size_t tarrank = idBelongTo(world_size, edgeId);
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
    item->tarRank = (int) tarrank;
    item->pack = buf;
    item->packSize = packSize;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    if (!blockQueue->push(item)) cerr << "Cannot request new edge" << endl;
}

void
requestOtherRanksToStoreVertex(int currank, int world_size, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {
    char *buf = new char[BUFFER_SIZE];
    int packSize = 0;
    int op = VERTEX_STORE_OP;
    size_t tarrank = idBelongTo(world_size, vertexId);
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

    item->tarRank = (int) tarrank;
    item->pack = buf;
    item->packSize = packSize;
    if (nullptr == blockQueue) cerr << "Cannot create blocking queue." << endl;
    if (!blockQueue->push(item)) cerr << "Cannot request new vertex" << endl;
}

// 该方法负责char *指针的delete
// TODO: 不负责read的delete
int processRecvRead(const char *filename, size_t pos) {
    ifstream infile(filename);
    string *read = new string;
    infile.seekg(pos);
    getline(infile, *read);
    infile.close();
    delete[] filename;
    if (requestWriteRead(*read) == 0) return 0;
    return 1;
}

// 该方法负责char *指针的delete
int processRecvEdge(EdgeList *edgeList, char *value, ReadId readId, KMERPOS_t kmerpos) {
    string *strValue = new string(value, 0, getK()-1);
    VertexId sourceVertex = getId(strValue->c_str());
    delete strValue;
    strValue = new string(value, 1, getK()-1);
    VertexId sinkVertex = getId(strValue->c_str());
    delete strValue;
    int flag = addNewEdge(edgeList, value, sourceVertex, sinkVertex, readId, kmerpos);
    delete[] value;
    return flag;
}

int processRecvVertex(VertexList *vertexList, SetOfID *tangleList, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {
    unsigned int flag = 0;
    if (HEAD_VERTEX == vertexMode)
        flag = addVertex(vertexList, vertexId, 0, edgeId, vertexMode);
    else if (TAIL_VERTEX == vertexMode)
        flag = addVertex(vertexList, vertexId, edgeId, 0, vertexMode);

    if (MULTI_OUT_DEGREE == (flag & MULTI_OUT_DEGREE)) {
        //tangleList->safe_insert(vertexId);
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
    int packSize = BUFFER_SIZE;
    MPI_Comm_rank(MPI_COMM_WORLD, &thisRank);
    int tag = TAG(sourceRank, thisRank);
    MPI_Status status;
    while (true) {
        cout << "recevier" << thisRank << "start." << endl;
        pack = new char[packSize];
        MPI_Recv(pack, packSize, MPI_INT, sourceRank, tag, MPI_COMM_WORLD, &status);
        cout << "Content:(" << packSize << ")" <<  string(pack) << endl;
        delete[] pack;
        break;
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
            if (EDGE_STORE_OP == op) {
                int strLen = 0;
                char *kmer = nullptr;
                ReadId readId;
                MPI_Unpack(pack, packSize, &position, &strLen, 1, MPI_INT, MPI_COMM_WORLD);
                kmer = new char[strLen+1];
                if (strLen != getK()) {
                    cout << "Trasmission error: " << strLen << endl;
                }
                MPI_Unpack(pack, packSize, &position, kmer, strLen, MPI_CHAR, MPI_COMM_WORLD);
                kmer[strLen] = '\0';
                MPI_Unpack(pack, packSize, &position, &readId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
                KMERPOS_t kmerpos;
                MPI_Unpack(pack, packSize, &position, &kmerpos, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
                processRecvEdge(edgeList, kmer, readId, kmerpos);
            } else if (EDGE_QUERY_OP == op) {
                // cout << "char edge reached" << endl;
                char *buf = new char[QUERY_PACK_SIZE];
                int sendMsgSize = 0;
                EdgeId queryEdgeId;
                //VertexId querySinkId;
                char lastChar = '0';
                int queryStatus = FAILED_QUERY;
                MPI_Unpack(pack, packSize, &position, &queryEdgeId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
                if (edgeList->count(queryEdgeId) == 0) {
                    cerr << "1Cannot query the edge #" << queryEdgeId << endl;
                    queryStatus = FAILED_QUERY;
                } else {
                    //querySinkId = edgeList->at(queryEdgeId)->sinkKMinusMerId;
                    lastChar = edgeList->at(queryEdgeId)->value[getK()-1];
                    queryStatus = SUCCESSFUL_QUERY;
                }
                MPI_Pack(&queryStatus, 1, MPI_INT, buf, QUERY_PACK_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Pack(&lastChar, 1, MPI_CHAR, buf, QUERY_PACK_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Send(buf, sendMsgSize, MPI_PACKED, sourceRank, QUERY_TAG(sourceRank, thisRank), MPI_COMM_WORLD);
                delete[] buf;
            } else if (EDGE_FULL_QUERY_OP == op) {
                // cout << "edge op received" << endl;
                char *buf = new char[BUFFER_SIZE];
                int sendMsgSize = 0;
                int valueLen = getK();
                char *value = nullptr;
                EdgeId queryEdgeId;
                int queryStatus = FAILED_QUERY;
                MPI_Unpack(pack, packSize, &position, &queryEdgeId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
                if (edgeList->count(queryEdgeId) == 0) {
                    cerr << "2Cannot query the edge #" << queryEdgeId << endl;
                    queryStatus = FAILED_QUERY;
                } else {
                    value = edgeList->at(queryEdgeId)->value;
                    if (NULL == value) cout << "debug4" << endl;
                    queryStatus = SUCCESSFUL_QUERY;
                }
                //cout << value << endl;
                MPI_Pack(&queryStatus, 1, MPI_INT, buf, BUFFER_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Pack(value, valueLen, MPI_CHAR, buf, BUFFER_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Send(buf, sendMsgSize, MPI_PACKED, sourceRank, QUERY_TAG(sourceRank, thisRank), MPI_COMM_WORLD);
                delete[] buf;
            }
        } else if (PREFIX_VERTEX_OP == op_prefix) {
            VertexId vertexId;
            EdgeId edgeId;
            MPI_Unpack(pack, packSize, &position, &vertexId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
            if (VERTEX_STORE_OP == op) {
                VertexMode_t vertexMode;
                MPI_Unpack(pack, packSize, &position, &edgeId, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
                MPI_Unpack(pack, packSize, &position, &vertexMode, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
                processRecvVertex(vertexList, tangleList, vertexId, edgeId, vertexMode);
            } else if (VERTEX_QUERY_OP == op) {
                // cout << "vertex op received" << endl;
                int queryStatus = FAILED_QUERY;
                char *buf = new char[QUERY_PACK_SIZE];
                int sendMsgSize = 0;

                if (vertexList->count(vertexId) == 0) {
                    vertexList->at(vertexId);
                    cerr << "Cannot query the vertex #" << vertexId << endl;
                    cerr << thisRank << endl;
                    queryStatus = FAILED_QUERY;
                } else {
                    // vertex存在时
                    Vertex *thisVertex = vertexList->at(vertexId);
                    if (thisVertex->outDegree < 1) { // 当遇到sink点时
                        cout << "Sink point is reached." << endl;
                        queryStatus = SINK_QUERY;
                    } else if (thisVertex->outDegree > 1) {
                        // 判断tangle
                        // 并行环境下不能使用tangleList->count(vertexId) != 0来判断是否为tangle
                        queryStatus = TANGLE_QUERY;
                        Vertex *thisVertex = vertexList->at(vertexId);
                        /*edgeId = *thisVertex->outKMer->begin();
                        // 删除vertex的一个出度
                        removeOutEdge(vertexList, vertexId, edgeId);*/
                        getAndRemoveOutEdge(vertexList, vertexId, &edgeId); // 保护并行环境下的删除节点
                    }else {
                        /*edgeId = *thisVertex->outKMer->begin();
                        // 删除vertex的一个出度
                        removeOutEdge(vertexList, vertexId, edgeId);*/
                        getAndRemoveOutEdge(vertexList, vertexId, &edgeId); // 保护并行环境下的删除节点
                        queryStatus = SUCCESSFUL_QUERY;
                    }
                }

                // 开始pack数据
                MPI_Pack(&queryStatus, 1, MPI_INT, buf, QUERY_PACK_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Pack(&edgeId, 1, my_MPI_SIZE_T, buf, QUERY_PACK_SIZE, &sendMsgSize, MPI_COMM_WORLD);
                MPI_Send(buf, sendMsgSize, MPI_PACKED, sourceRank, QUERY_TAG(sourceRank, thisRank), MPI_COMM_WORLD);
                delete[] buf;
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

// 不能与线程并行的共存
void sendSuperContigToRankHead(int headrank, int currank, string &superContig) {
    int msgsize = superContig.size();
    MPI_Send(&msgsize, 1, MPI_INT, headrank, TAG(currank, headrank), MPI_COMM_WORLD);
    MPI_Send(superContig.c_str(), superContig.size(), MPI_CHAR, headrank, TAG(currank, headrank), MPI_COMM_WORLD);
}

// 不能与线程并行的共存
string reduceSuperContigFromOthers(int currank, int world_size, string &superContig) {
    stringstream sc;
    sc << superContig;
    //cout << superContig;
    char *buf = nullptr;
    int size4buf = 0;
    MPI_Status status;
    for (int i = 0; i < world_size; ++i) {
        if (i == currank) continue;
        MPI_Recv(&size4buf, 1, MPI_INT, i, TAG(i, currank), MPI_COMM_WORLD, &status);
        buf = new char[size4buf];
        MPI_Recv(buf, size4buf, MPI_CHAR, i, TAG(i, currank), MPI_COMM_WORLD, &status);
        sc << string(buf, 0, size4buf);
        delete[] buf;
    }
    return sc.str();
}

int queryEdgeById(string *pEdgeStr, int currank, int tarrank, EdgeId edgeId) {
    char *buf = new char[QUERY_PACK_SIZE];
    char *edgeValue = new char[2];
    int packSize = 0;
    int position = 0;
    int op = EDGE_QUERY_OP;
    int queryStatus = FAILED_QUERY;
    MPI_Status status;
    MPI_Pack(&op, 1, MPI_INT, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&edgeId, 1, my_MPI_SIZE_T, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Send(buf, packSize, MPI_PACKED, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);

    delete[] buf;
    buf = new char[QUERY_PACK_SIZE];

    MPI_Recv(buf, QUERY_PACK_SIZE, MPI_PACKED, tarrank, QUERY_TAG(currank, tarrank), MPI_COMM_WORLD, &status);
    MPI_Unpack(buf, QUERY_PACK_SIZE, &position, &queryStatus, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(buf, QUERY_PACK_SIZE, &position, edgeValue, 1, MPI_CHAR, MPI_COMM_WORLD);
    delete[] buf;
    edgeValue[1] = '\0'; // 以防溢出
    // cout << edgeValue << endl;
    // 只有不是失败才append
    if (FAILED_QUERY != queryStatus)
        pEdgeStr->append(edgeValue);
    delete[] edgeValue;
    return queryStatus;
}

int queryOutEdgeOfVertexById(EdgeId *pEdgeId, int currank, int tarrank, VertexId vertexId) {
    char *buf = new char[QUERY_PACK_SIZE];
    EdgeId outEdge;
    int packSize = 0;
    int position = 0;
    int op = VERTEX_QUERY_OP;
    int queryStatus = FAILED_QUERY;
    MPI_Status status;
    MPI_Pack(&op, 1, MPI_INT, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&vertexId, 1, my_MPI_SIZE_T, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Send(buf, packSize, MPI_PACKED, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);

    delete[] buf;
    buf = new char[QUERY_PACK_SIZE];

    MPI_Recv(buf, QUERY_PACK_SIZE, MPI_PACKED, tarrank, QUERY_TAG(currank, tarrank), MPI_COMM_WORLD, &status);
    MPI_Unpack(buf, QUERY_PACK_SIZE, &position, &queryStatus, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(buf, QUERY_PACK_SIZE, &position, &outEdge, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);

    delete[] buf;
    // 只有不是失败才append
    if (FAILED_QUERY != queryStatus)
        *pEdgeId = outEdge;
    return queryStatus;
}

int queryFullEdgeById(string *pString, int currank, int tarrank, EdgeId edgeId) {
    char *buf = new char[QUERY_PACK_SIZE];
    char *edgeValue = new char[getK() + 1];
    int packSize = 0;
    int position = 0;
    int op = EDGE_FULL_QUERY_OP;
    int queryStatus = FAILED_QUERY;
    MPI_Status status;
    MPI_Pack(&op, 1, MPI_INT, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Pack(&edgeId, 1, my_MPI_SIZE_T, buf, QUERY_PACK_SIZE, &packSize, MPI_COMM_WORLD);
    MPI_Send(buf, packSize, MPI_PACKED, tarrank, TAG(currank, tarrank), MPI_COMM_WORLD);
    delete[] buf;
    buf = new char[BUFFER_SIZE];
    MPI_Recv(buf, BUFFER_SIZE, MPI_PACKED, tarrank, QUERY_TAG(currank, tarrank), MPI_COMM_WORLD, &status);
    MPI_Unpack(buf, BUFFER_SIZE, &position, &queryStatus, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(buf, BUFFER_SIZE, &position, edgeValue, getK(), MPI_CHAR, MPI_COMM_WORLD);
    // cout << "debug" << endl;
    delete[] buf;
    edgeValue[getK()] = '\0'; // 以防溢出
    // cout << edgeValue << endl;
    // 只有不是失败才append
    if (FAILED_QUERY != queryStatus)
        pString->append(edgeValue);
    delete[] edgeValue;
    return queryStatus;
}
