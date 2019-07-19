//
// Created by erianliang on 19-7-19.
//

#include "communicator.h"
#include "blocking_queue.h"

const block_queue<char *> *blockQueue = new block_queue<char *>(1024*1024);

void requestRankToStoreRead(int currank, int tarrank, const char *filename, size_t pos) {

}

void requestOtherRanksToStoreEdge(int currank, int world_size, const char *value, EdgeId edgeId, ReadId readId,
                                  KMERPOS_t kmerpos) {

}

void
requestOtherRanksToStoreVertex(int currank, int world_size, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {

}

void requestOtherRanksToStoreTangle(int currank, int world_size, VertexId tangleId) {

}

int processRecvRead(const char *filename, size_t pos) {
    return 0;
}

int processRecvEdge(EdgeList *edgeList, char *value, ReadId readId, KMERPOS_t kmerpos) {
    return 0;
}

int processRecvVertex(VertexList *vertexList, VertexId vertexId, EdgeId edgeId, VertexMode_t vertexMode) {
    return 0;
}

void senderRunner() {

}

void receiverRunner(VertexList *vertexList, EdgeList *edgeList, SetOfID *tangleList) {

}
