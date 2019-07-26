//
// Created by erianliang on 19-7-16.
//


#ifndef RANDOMSTRINGASSEMBLY_K_MER_H
#define RANDOMSTRINGASSEMBLY_K_MER_H

#include "idset.h"
#include "read.h"
#include <map>

struct kMer {
    size_t id;
    char *value;
    /*bool isZ;
    char *additionValue;
    int sizeOfAdditionValue;
    SetOfID *endHerePathSet;
    SetOfID *startFromHerePathSet;
    SetOfID *includeThisPathSet;*/
    size_t sourceKMinusMerId;
    size_t sinkKMinusMerId;
    int multiplicity;
    int availPassTime;

    kMer();

    virtual ~kMer();
};

typedef struct kMer Edge;
typedef struct kMer KMer;
typedef std::map<EdgeId , Edge *> EdgeList;
typedef EdgeList KMerList;

void setK(int k);

int getK();

int addNewEdge(EdgeList *eList, char *value, VertexId sourceVId, VertexId sinkVId, ReadId rId, KMERPOS_t kmerpos);

int addNewEdge(EdgeList *eList, char *value, EdgeId *fetchedEdgeId, VertexId sourceVId, VertexId sinkVId, ReadId rId, KMERPOS_t kmerpos);

/*
int addNewZEdge(EdgeList *eList, char *value, char *additionValue, EdgeId *fetchedEdgeId, VertexId sourceVId,
                VertexId sinkVId, SetOfID *endHerePathSet, SetOfID *startFromHerePathSet, SetOfID *includeThisPathSet);
*/

int removeEdge(EdgeList *eList, EdgeId eId);

int addReadPathTo(const Edge *e, ReadId rId, KMERPOS_t kmerpos);

int addReadPathTo(EdgeList *eList, EdgeId eId, ReadId rId,KMERPOS_t kmerpos);

int removeReadPathFrom(const Edge *e, ReadId rId, KMERPOS_t kmerpos);

int removeReadPathFrom(EdgeList *eList, EdgeId eId, ReadId rId, KMERPOS_t kmerpos);

int increaseAvailPassTime(EdgeList *eList, EdgeId eId);

int reduceAvailPassTime(EdgeList *eList, EdgeId eId);

/**
 * Check whether the edge can be still access by the original string.
 * @param eList list that store edges
 * @param eId edge id
 * @return true when availPassTime is greater than 0; otherwise false.
 */
bool checkAvailPassTimeNonZero(EdgeList *eList, EdgeId eId);
#endif //RANDOMSTRINGASSEMBLY_K_MER_H
