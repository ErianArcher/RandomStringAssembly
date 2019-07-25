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
    int startIndex = 0;
    for (; startIndex < argc; ++startIndex) {
        if (string(argv[startIndex]).find("RandomStringAssembly") != string::npos) {
            break;
        }
    }
    int realargc = argc - startIndex;
    int k = 3;
    string folderPath = "./input/";
    int fileNum = 1;
    string *filenames = new string[realargc - 3];
    if (realargc < 4) {
        cout << "program k inputfolder filenames..." << endl;
    }


    k = stoi(string(argv[startIndex + 1]));
    folderPath = string(argv[startIndex + 2]);
    if (*folderPath.cend() != '/') {
        folderPath.append("/");
    }

    for (int i = 3; i < realargc; i++) {
        filenames[i-3] = string(argv[startIndex + i]);
    }

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
    VertexList *vertexList = new VertexList;
    SetOfID *tangleList = new SetOfID;
    EdgeList *edgeList = new EdgeList;

    /*VertexList *vertexList4Others = new VertexList;
    EdgeList *edgeList4Others = new EdgeList;
    SetOfID *tangleList4Others = new SetOfID;*/

    int readNum = 1024 * 1024 * 1024;
    setK(k); // 初始化k值

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
    int sender_num = 1;
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
        // cout << "sucessful" << r;
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
	/*
        if ((read4Rank = *readId % world_size) == currank) {
            // Create a thread to process
            if (requestWriteRead(*curread) == 0) {
                exit(1);
            }
        }
	*/

        // 先把全部read处理完之后再充分拍kmer和kminusmer
        // 并行化处理read中的kmer
        int totalKmerNum = curread->size()-k+1;
        int kMerNum4Each = totalKmerNum / world_size;
        int startKMer = kMerNum4Each * currank;
        int end = (currank + 1 == world_size)? totalKmerNum: (startKMer + kMerNum4Each);
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

            *tmpEdgeId = getId(string(tmpKMer, 0, getK()));
            *tmpVertexId1 = getId(string(tmpKMinusMer1, 0, getK()-1));
            *tmpVertexId2 = getId(string(tmpKMinusMer2, 0, getK()-1));

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

    MPI_Barrier(MPI_COMM_WORLD);

    // 等待线程
    mainThreadTellFinished(currank, world_size);
    joinThreads(sender_tid, sender_num);
    joinThreads(receiver_tid, world_size, currank);


    // 在等待子线程结束之前
    // 寻找source和sink
    SetOfID *sourceList= new SetOfID;
    SetOfID *sinkList= new SetOfID;
    unordered_map<VertexId, string *> contigList;
    stringstream superContigSS;
    for (auto kv: *vertexList) {
        auto vertex = kv.second;
        if (vertex->inDegree == 0 && vertex->outDegree > 0) {
            sourceList->insert(vertex->id);
        }
        if (vertex->outDegree == 0 && vertex->inDegree > 0) {
            sinkList->insert(vertex->id);
        }
    }

    int sourceListSize = sourceList->size();
    int totalSourceListSize = 0;
    int headrank = 0;
    MPI_Reduce(&sourceListSize, &totalSourceListSize, 1, MPI_INT, MPI_SUM, headrank, MPI_COMM_WORLD);
    // 若所有进程都没有符合条件的source，则由主节点随机选一个。
    if (0 == totalSourceListSize) {
        if (currank == headrank) {
            VertexId randomVertexId = (*vertexList->begin()).second->id;
            while (sinkList->count(randomVertexId) == 1) randomVertexId = (*vertexList->begin()).second->id;
            sourceList->insert(randomVertexId);
        }
    }

    /*if (headrank == currank)
        cout << sourceList->size() << endl;*/

    /*
     * 重新开启线程以进行构建contig的工作
     */
    mainThreadTellRunning();
    sender_num = 1;
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
        //cout << "sucessful" << r;
        // pthread_detach(receiver_tid[r]);
    }


    //cout << "debug" << endl;
    // 遍历所有的source点
    for (auto sourceId: *sourceList) {
        // cout << "in source" << endl;
        Vertex *sourceVertex = vertexList->at(sourceId);
        Edge *tmp = nullptr;
        string edgeValue("");
        VertexId nextVertexId;
        EdgeId nextEdgeId = *sourceVertex->outKMer->begin(); // 倘若source点有多个出度则随机选取一个出度
        removeOutEdge(vertexList, sourceId, nextEdgeId); // 每次经过一个出度都要删除
        int vertexInRank;
        int edgeInRank = nextEdgeId % world_size;
        if (edgeInRank != currank) { // edge不在这个机器上
            // cout << "debug" << endl;
            if (FAILED_QUERY == queryFullEdgeById(&edgeValue, currank, edgeInRank, nextEdgeId)) {
                // 查询失败，跳过这个vertex
                cout << "Cannot queue the first out edge of this source vertex #" << sourceId << endl;
                continue;
            }
            // cout << edgeValue << endl;
            nextVertexId = getId(edgeValue.substr(1, getK() - 1)); // 初始化的时候edgeValue的长度应该是K
            //cout << edgeValue.substr(1, getK() - 1) << endl;
        } else {
            if (edgeList->count(nextEdgeId) < 1) {
                //cout << "debug3" << endl;
                continue;
            }
            tmp = edgeList->at(nextEdgeId);
            edgeValue = string(tmp->value, 0, getK());
            nextVertexId = tmp->sinkKMinusMerId;
        }
        // 初始化完成，开始遍历直至遇到sink
        while (true) {
            // cout << "debug" << endl;
            string *tmpEdge = new string("");

            // 遍历vertex
            vertexInRank = nextVertexId % world_size;
            if (vertexInRank != currank) {
                int flag = queryOutEdgeOfVertexById(&nextEdgeId, currank, vertexInRank, nextVertexId); // nextEdgeId 在这已经赋值
                if (FAILED_QUERY == flag || SINK_QUERY == flag) {
                    //cout << "Cannot queue vertex in rank #" << vertexInRank << endl;
                    break;
                }
            } else {
                if (vertexList->count(nextVertexId) == 0) {
                    cerr << "Cannot query the vertex #" << nextVertexId << endl;
                    break;
                } else {
                    // 不是tangle时
                    Vertex *thisVertex = vertexList->at(nextVertexId);
                    if (thisVertex->outDegree < 1) { // 当遇到sink点时
                        cout << "Sink point is reached." << endl;
                        break;
                    } else if(thisVertex->outDegree > 1) {
                        // tangle被检测到
                        // 并行环境下不能使用tangleList->count(vertexId) != 0来判断是否为tangle
                        Vertex *thisVertex = vertexList->at(nextVertexId);
                        nextEdgeId = *thisVertex->outKMer->begin();
                        // 删除vertex的一个出度
                        removeOutEdge(vertexList, nextVertexId, nextEdgeId);
                    } else {
                        nextEdgeId = *thisVertex->outKMer->begin();
                        removeOutEdge(vertexList, nextVertexId, nextEdgeId);
                    }
                }
            }

            // 通过edge查询下一个vertex
            // 添加的时候只添加一位char
            edgeInRank = nextEdgeId % world_size;
            if (edgeInRank != currank) { // edge不在这个机器上
                if (FAILED_QUERY == queryEdgeById(tmpEdge, currank, edgeInRank, nextEdgeId)) {
                    delete tmpEdge;
                    //cout << "debug1" << endl;
                    break;
                }
                //cout << "debug2" << endl;
                // cout << tmpEdge << endl;
                edgeValue.append(*tmpEdge);
                nextVertexId = getId(edgeValue.substr(edgeValue.size() - (getK() - 1), getK() - 1));
            } else {
                if (edgeList->count(nextEdgeId) < 1) {
                    delete tmpEdge;
                    //cout << "debug2" << endl;
                    break;
                }
                tmp = edgeList->at(nextEdgeId);
                edgeValue.append(string(tmp->value, getK() - 1, 1)); // 把最后一位加上去
                nextVertexId = tmp->sinkKMinusMerId;
            }

            delete tmpEdge;
        }

        string *pcontig = new string(edgeValue);
        // 在contig中添加元素
        contigList[sourceId] = pcontig;
    }

    // 构造super contig
    for (auto kv: contigList) {
        superContigSS << *kv.second;
        delete kv.second;
    }

    contigList.clear();

    // 进程间同步
    MPI_Barrier(MPI_COMM_WORLD);

    // 主线程结束操作
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

    //cout << "reached" << endl;

    // 同步各rank的super contig
    string superContig;
    if (0 == currank) {
        superContig = reduceSuperContigFromOthers(currank, world_size, superContigSS.str());
        ofstream outfile("output"); 
	outfile << superContig << endl;
	outfile.close();
    } else {
        sendSuperContigToRankHead(0, currank, superContigSS.str());
    }

    //cout << "reached2" << endl;
    //pthread_join(sender_tid, NULL);
    // !~delete the variables above.
    delete[] recvArg;


    // 最后delete的变量
    delete vertexList;
    delete edgeList;
    delete tangleList;

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}
