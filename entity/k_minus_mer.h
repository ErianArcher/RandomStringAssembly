//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_K_MINUS_MER_H
#define RANDOMSTRINGASSEMBLY_K_MINUS_MER_H

#include "idset.h"

struct kMinusMer {
    size_t id;
    char *value;
    SetOfID inKMer;
    SetOfID outKMer;
    int inDegree;
    int outDegree;
};

typedef struct kMinusMer Vertex;
typedef struct kMinusMer KMinusMer;



#endif //RANDOMSTRINGASSEMBLY_K_MINUS_MER_H
