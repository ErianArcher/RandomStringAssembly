#include <iostream>
#include <mpi.h>
#include <sstream>
#include <pthread.h>
#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"
#include "entity/k_mer.h"
#include "test.h"
#include "communicator.h"

#define TRESHOLD 1024*1024*256

using namespace std;

inline size_t getId(string value) {
    return hash<string>()(value);
}

string getNextRead(int rank, int world_size, int read_num, string filepath, size_t *readpos);

int main(int argc, char** argv) {
    // Initialize the MPI environment
    // Enable multithread
    MPI_Init_thread(nullptr, nullptr, MPI_THREAD_SERIALIZED, nullptr);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int currank;
    MPI_Comm_rank(MPI_COMM_WORLD, &currank);

/*    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);*/

    // Construct DBG
    string folderPath = "./input/";
    string currentFile = "file";
    VertexList *vertexList = new VertexList;
    SetOfID *tangleList = new SetOfID;
    EdgeList *edgeList = new EdgeList;

    /*VertexList *vertexList4Others = new VertexList;
    EdgeList *edgeList4Others = new EdgeList;
    SetOfID *tangleList4Others = new SetOfID;*/
    int k = 3;
    int readNum = 1024 * 1024 * 1024;
    int numToProcess = readNum / world_size;
    int beginIndex =  currank * numToProcess;
    int endIndex = currank+1==world_size? readNum-1: beginIndex+numToProcess-1;

    // Some temporary variables
    string *curread = new string;
    ReadId *readId = new ReadId;
    size_t readpos;
    char *tmpKMer = new char[k];
    char *tmpKMinusMer1 = new char[k-1];
    char *tmpKMinusMer2 = new char[k-1];
    EdgeId *tmpEdgeId = new EdgeId;
    VertexId *tmpVertexId1 = new VertexId;
    VertexId *tmpVertexId2 = new VertexId;
    KMERPOS_t edgeMode = NOT_INCLUDE_KMER;

    // 开启线程
    pthread_t sender_tid, receiver_tid;
    RecvArg *recvArg = new RecvArg;
    recvArg->vertexList = vertexList;
    recvArg->edgeList = edgeList;
    recvArg->tangleList = tangleList;

    if (pthread_create(&sender_tid, nullptr, senderRunner, nullptr) != 0) {
        cerr << "Cannot create sender thread";
        exit(1);
    }

    if (pthread_create(&receiver_tid, nullptr, receiverRunner, recvArg) != 0) {
        cerr << "Cannot create receiver thread";
        exit(1);
    }

    while (!(*curread = getNextRead(currank, world_size, readNum, folderPath, &readpos)).empty()) {
        // 传送read的文件名和文件指针到目标机器
        int read4Rank = currank;
        *readId = getId(*curread);
        if ((read4Rank = *readId % world_size) != currank) {
            // Create a thread to process
            requestRankToStoreRead(currank, read4Rank, currentFile.c_str(), readpos);
        } else {
            // Create a thread to process
            if (createRead(*curread, nullptr) == 0) {
                cerr << "Error: Cannot create file for read \"" << *curread << "\"" << endl;
            }
        }

        // 先把全部read处理完之后再充分拍kmer和kminusmer
        int end = curread->size()-k+1;
        for (int i = 0; i < curread->size()-k-1; ++i) {
            for (int j = 0; j < k; ++j) {
                char tmp = (*curread)[i + j];
                tmpKMer[j] = tmp;
                if (j < k -1)
                    tmpKMinusMer1[j] = tmp;
                if (j > 0)
                    tmpKMinusMer2[j] = tmp;
            }

            // 判断kmer在read的哪个位置
            if (i == 0) edgeMode = START_KMER;
            else if (i == end - 1) edgeMode = END_KMER;
            else edgeMode = INCLUDE_KMER;

            *tmpEdgeId = getId(string(tmpKMer));
            *tmpVertexId1 = getId(string(tmpKMinusMer1));
            *tmpVertexId2 = getId(string(tmpKMinusMer2));

            // 创建开始Vertex
            if (*tmpVertexId1 % world_size != currank) {
                requestOtherRanksToStoreVertex(currank, world_size, *tmpVertexId1, *tmpEdgeId, HEAD_VERTEX);
            } else {
                if ((addVertex(vertexList, tmpKMinusMer1, 0, *tmpEdgeId, HEAD_VERTEX) & MULTI_OUT_DEGREE) == MULTI_OUT_DEGREE){
                    tangleList->insert(*tmpVertexId1);
                }
            }

            // 创建终止Vertex
            if (*tmpVertexId2 % world_size != currank) {
                requestOtherRanksToStoreVertex(currank, world_size, *tmpVertexId2, *tmpEdgeId, TAIL_VERTEX);
            } else {
                if ((addVertex(vertexList, tmpKMinusMer2, *tmpEdgeId, 0, TAIL_VERTEX) & MULTI_OUT_DEGREE) == MULTI_OUT_DEGREE)
                {
                    tangleList->insert(*tmpVertexId2);
                }
            }

            // 创建Edge
            if (*tmpEdgeId % world_size != currank)
                requestOtherRanksToStoreEdge(currank, world_size, tmpKMer, *tmpEdgeId, *readId, edgeMode);
            else
                addNewEdge(edgeList, tmpKMer, tmpEdgeId, *tmpVertexId1, *tmpVertexId2, *readId, edgeMode);
        }
    }
    // TODO: delete the variables above.

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}