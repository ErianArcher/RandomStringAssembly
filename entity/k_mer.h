//
// Created by erianliang on 19-7-16.
//


#ifndef RANDOMSTRINGASSEMBLY_K_MER_H
#define RANDOMSTRINGASSEMBLY_K_MER_H

#include "idset.h"

struct kMer {
    size_t id;
    char *value;
    bool isZ;
    char *additionValue;
    unsigned int sizeOfAdditionValue;
    SetOfID endHerePathSet;
    SetOfID startFromHerePathSet;
    SetOfID includeThisPathSet;
    size_t sourceKMinusMerId;
    size_t sinkKMinusMerId;
    int multiplicity;
    int availPassTime;
};

typedef struct kMer Edge;
typedef struct kMer KMer;

#endif //RANDOMSTRINGASSEMBLY_K_MER_H
