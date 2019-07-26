//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_K_MINUS_MER_H
#define RANDOMSTRINGASSEMBLY_K_MINUS_MER_H

#include "idset.h"
#include <map>

// Constants for VertexMode_t
#define HEAD_VERTEX 1u
#define TAIL_VERTEX 2u
#define IN_VERTEX (HEAD_VERTEX | TAIL_VERTEX)
#define ISOLATE_VERTEX (~IN_VERTEX)

// Constants for return types of `addVertex`.
#define MULTI_OUT_DEGREE 2u
#define MULTI_IN_DEGREE 4u
#define MULTI_BOTH_DEGREE (MULTI_OUT_DEGREE | MULTI_IN_DEGREE)

struct kMinusMer {
    size_t id;
    //char *value;
    SetOfID *inKMer;
    SetOfID *outKMer;
    int inDegree;
    int outDegree;

    kMinusMer();

    virtual ~kMinusMer();
};

typedef struct kMinusMer Vertex;
typedef struct kMinusMer KMinusMer;
typedef std::map<VertexId, Vertex *> VertexList;

typedef unsigned int VertexMode_t;
//typedef int VertexAddReturn_t;

unsigned int addVertex(VertexList *vList, char *value, EdgeId inKMerId, EdgeId outKMerId, VertexMode_t mode);

unsigned int addVertex(VertexList *vList, char *value, VertexId *fetchedVertexId, EdgeId inKMerId,
                       EdgeId outKMerId, VertexMode_t mode);

unsigned int addVertex(VertexList *vList, VertexId vId, EdgeId inKMerId,
                       EdgeId outKMerId, VertexMode_t mode);

int removeVertex(VertexList *vList, VertexId vId);

int addInEdge(const VertexList *vList, VertexId vId, EdgeId eId);

int removeInEdge(const VertexList *vList, VertexId vId, EdgeId eId);

int addOutEdge(const VertexList *vList, VertexId vId, EdgeId eId);

int removeOutEdge(const VertexList *vList, VertexId vId, EdgeId eId);

int freeVertex(Vertex *pVertex);
#endif //RANDOMSTRINGASSEMBLY_K_MINUS_MER_H
