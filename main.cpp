#include <iostream>
#include <mpi.h>
#include <sstream>
#include <pthread.h>
#include <fstream>
#include <thread>
#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"
#include "entity/k_mer.h"
#include "test.h"
#include "communicator.h"
#include "read_io.h"
#include "test.h"

#define TRESHOLD 1024*1024*256


using namespace std;

void joinThreads(pthread_t tids[], int tnum, int exceptIndex = -1) {
    for (int i = 0; i < tnum; ++i) {
        if (i == exceptIndex) continue;
        pthread_join(tids[i], NULL);
    }
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    // Enable multithread
    int provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);

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
    int fileNum = 1;
    string filenames[] = {string("reads")};
    VertexList *vertexList = new VertexList;
    SetOfID *tangleList = new SetOfID;
    EdgeList *edgeList = new EdgeList;

    /*VertexList *vertexList4Others = new VertexList;
    EdgeList *edgeList4Others = new EdgeList;
    SetOfID *tangleList4Others = new SetOfID;*/
    int k = 3;
    int readNum = 1024 * 1024 * 1024;

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
    int sender_num = 2;
    pthread_t sender_tid[sender_num], receiver_tid[world_size];
    int sourceRank[world_size];
    RecvArg *recvArg = new RecvArg[world_size];

    mainThreadTellRunning();
    for (int l = 0; l < sender_num; ++l) {
        if (pthread_create(&sender_tid[l], NULL, senderRunner, NULL) != 0) {
            cerr << "Cannot create sender thread";
            exit(1);
        }
        // pthread_detach(sender_tid[l]);
    }


    for (int r = 0; r < world_size; ++r) {
        sourceRank[r] = r;
        if (r == currank) continue;

        // TODO: 用receiverRunner 替换
        recvArg[r].sourceRank = sourceRank[r];
        recvArg[r].vertexList = vertexList;
        recvArg[r].edgeList = edgeList;
        recvArg[r].tangleList = tangleList;
        if (pthread_create(&receiver_tid[r], NULL, receiverRunner, (void *) &recvArg[r]) != 0) {
            cerr << "Cannot create receiver thread";
            exit(1);
        }
        cout << "sucessful" << r;
        // pthread_detach(receiver_tid[r]);
    }

    /*char content[] = "hello world";
    int lrank;
    for (int l = 0; l < 100; ++l) {
        lrank = l % world_size;
        if (lrank != currank) {
            //cout << "in" << l << endl;
            requestRankToDoTest(currank, lrank, content);
        }
    }*/

    // 初始化文件IO
    if (readIOInit(currank, world_size, folderPath, filenames, fileNum) == -1) {
        cerr << "Error: No files can be read." << endl;
        exit(1);
    }

    while (getNextRead(curread, &readpos) != -1) {
        // 传送read的文件名和文件指针到目标机器
        int read4Rank = currank;
        *readId = getId(*curread);
        if ((read4Rank = *readId % world_size) == currank) {
            // Create a thread to process
            if (requestWriteRead(*curread) == 0) {
                exit(1);
            }
        }

        // 先把全部read处理完之后再充分拍kmer和kminusmer
        // 并行化处理read中的kmer
        int totalKmerNum = curread->size()-k+1;
        int kMerNum4Each = totalKmerNum / world_size;
        int startKMer = kMerNum4Each * currank;
        int end = currank + 1 == world_size? totalKmerNum: (startKMer + kMerNum4Each);
        for (int i = startKMer; i < end; ++i) {
            for (int j = 0; j < k; ++j) {
                char tmp = (*curread)[i + j];
                tmpKMer[j] = tmp;
                if (j < k -1)
                    tmpKMinusMer1[j] = tmp;
                if (j > 0)
                    tmpKMinusMer2[j-1] = tmp;
            }

            // 判断kmer在read的哪个位置
            if (i == 0) edgeMode = START_KMER;
            else if (i == end - 1) edgeMode = END_KMER;
            else edgeMode = INCLUDE_KMER;

            *tmpEdgeId = getId(string(tmpKMer));
            *tmpVertexId1 = getId(string(tmpKMinusMer1));
            *tmpVertexId2 = getId(string(tmpKMinusMer2));

            // 创建开始Vertex
            //cout << "index: " << i << "; content: " << tmpKMinusMer1 << endl;
            if (*tmpVertexId1 % world_size != currank) {
                requestOtherRanksToStoreVertex(currank, world_size, *tmpVertexId1, *tmpEdgeId, HEAD_VERTEX);
            } else {
                if ((addVertex(vertexList, tmpKMinusMer1, 0, *tmpEdgeId, HEAD_VERTEX) & MULTI_OUT_DEGREE) == MULTI_OUT_DEGREE){
                    tangleList->safe_insert(*tmpVertexId1);
                }
            }

            // 创建终止Vertex
            //cout << "index: " << i << "; content: " << tmpKMinusMer2 << endl;
            if (*tmpVertexId2 % world_size != currank) {
                requestOtherRanksToStoreVertex(currank, world_size, *tmpVertexId2, *tmpEdgeId, TAIL_VERTEX);
            } else {
                if ((addVertex(vertexList, tmpKMinusMer2, *tmpEdgeId, 0, TAIL_VERTEX) & MULTI_OUT_DEGREE) == MULTI_OUT_DEGREE)
                {
                    tangleList->safe_insert(*tmpVertexId2);
                }
            }

            // 创建Edge
            if (*tmpEdgeId % world_size != currank)
                requestOtherRanksToStoreEdge(currank, world_size, tmpKMer, *tmpEdgeId, *readId, edgeMode);
            else
                addNewEdge(edgeList, tmpKMer, tmpEdgeId, *tmpVertexId1, *tmpVertexId2, *readId, edgeMode);
        }
    }


    // 等待线程
    mainThreadTellFinished(currank, world_size);
    joinThreads(sender_tid, sender_num);
    joinThreads(receiver_tid, world_size, currank);

    //std::chrono::milliseconds dura(10000);
    //std::this_thread::sleep_for(dura);
    // DEBUG
    stringstream ss;
    ss << "./input/debug_host" << currank << ".txt";
    string debugFile = ss.str();
    ofstream outfile(debugFile);
    outfile << "===========Edges===========" << edgeList->size() << endl;
    for (auto kv: *edgeList) {
        outfile << edgeToString(kv.second) << endl;
    }
    outfile << "===========Vertices===========" << vertexList->size() << endl;
    for (auto kv: *vertexList) {
        outfile << vertexToString(kv.second) << endl;
    }
    outfile << "===========Tangles===========" << tangleList->size() << endl;
    for (auto v: *tangleList) {
        outfile << v << endl;
    }

    //pthread_join(sender_tid, NULL);
    // TODO: delete the variables above.
    delete[] recvArg;


    // 最后delete的变量
    delete vertexList;
    delete edgeList;
    delete tangleList;

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}