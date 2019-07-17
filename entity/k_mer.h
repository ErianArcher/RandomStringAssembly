//
// Created by erianliang on 19-7-16.
//


#ifndef RANDOMSTRINGASSEMBLY_K_MER_H
#define RANDOMSTRINGASSEMBLY_K_MER_H

#include "idset.h"
#include "read.h"
#include "k_minus_mer.h"
#include <unordered_map>

struct kMer {
    size_t id;
    char *value;
    bool isZ;
    char *additionValue;
    int sizeOfAdditionValue;
    SetOfID *endHerePathSet;
    SetOfID *startFromHerePathSet;
    SetOfID *includeThisPathSet;
    size_t sourceKMinusMerId;
    size_t sinkKMinusMerId;
    int multiplicity;
    int availPassTime;

    kMer();

    virtual ~kMer();
};

typedef struct kMer Edge;
typedef struct kMer KMer;
typedef size_t EdgeId;
typedef std::unordered_map<EdgeId , Edge *> EdgeList;
typedef EdgeList KMerList;


int addNewEdge(EdgeList *eList, char *value, VertexId sourceVId, VertexId sinkVId, ReadId rId, KMERPOS_t kmerpos);

int addNewZEdge(EdgeList *eList, char *value, char *additionValue, VertexId sourceVId, VertexId sinkVId,
                SetOfID *endHerePathSet, SetOfID *startFromHerePathSet, SetOfID *includeThisPathSet);

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
