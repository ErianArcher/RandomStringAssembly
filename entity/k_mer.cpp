//
// Created by erianliang on 19-7-16.
//

#include "k_mer.h"
#include <iostream>
#include <cstring>

int copyPathSetToEdge(Edge *e, SetOfID *pathSet);

Edge *fetchEdgeFromList(EdgeList *eList, EdgeId eId);

using namespace std;

kMer::~kMer() {
    if (nullptr != value) {
        delete value;
        value = nullptr;
    }
    if (nullptr != additionValue) {
        delete additionValue;
        additionValue = nullptr;
    }
    if (nullptr != endHerePathSet) {
        delete endHerePathSet;
        endHerePathSet = nullptr;
    }
    if (nullptr != startFromHerePathSet) {
        delete startFromHerePathSet;
        startFromHerePathSet = nullptr;
    }
    if (nullptr != includeThisPathSet) {
        delete includeThisPathSet;
        includeThisPathSet = nullptr;
    }
}

kMer::kMer() {
    id = 0;
    value = nullptr;
    isZ = false;
    additionValue = nullptr;
    sizeOfAdditionValue = 0;
    endHerePathSet = nullptr;
    startFromHerePathSet = nullptr;
    includeThisPathSet = nullptr;
    sourceKMinusMerId = 0;
    sinkKMinusMerId = 0;
    multiplicity = 0;
    availPassTime = 1;
}

int
addNewEdge(EdgeList *eList, char *value, VertexId sourceVId, VertexId sinkVId, ReadId rId, KMERPOS_t kmerpos) {
    string idStr(value);
    EdgeId eId = hash<string>()(idStr);
    Edge *e = nullptr;
    if (eList->find(eId) == eList->end()) {
        e = new Edge;
        if (nullptr == e) {
            cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
            return 0;
        }
        e->id = eId;

        // Copy value
        int valueLen = strlen(value);
        e->value = new char[valueLen];
        if (nullptr == e->value) {
            cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
            return 0;
        }
        strcpy(e->value, value);

        e->sourceKMinusMerId = sourceVId;
        e->sinkKMinusMerId = sinkVId;
        e->multiplicity++;

        e->endHerePathSet = new SetOfID;
        if (nullptr == e->endHerePathSet) {
            cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
            return 0;
        }

        e->startFromHerePathSet = new SetOfID;
        if (nullptr == e->startFromHerePathSet) {
            cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
            return 0;
        }

        e->includeThisPathSet = new SetOfID;
        if (nullptr == e->includeThisPathSet) {
            cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
            return 0;
        }

        // Insert into edge list
        eList->insert(make_pair(eId, e));

        if (eList->find(eId) == eList->end()) {
            cerr << "Error occurs when adding edge #" << eId << ".\n";
            delete e;
            return 0;
        }
    } else {
        e = eList->at(eId);
        if (nullptr == e) {
            cerr << "Error occurs when adding edge #" << eId << ": Cannot read edge from edge list.\n";
            return 0;
        }
        e->multiplicity++;
    }
    addReadPathTo(e, rId, kmerpos);
    delete value;
    return 1;
}

int addNewZEdge(EdgeList *eList, char *value, char *additionValue, VertexId sourceVId, VertexId sinkVId,
                SetOfID *endHerePathSet, SetOfID *startFromHerePathSet, SetOfID *includeThisPathSet) {

    int lenValue = strlen(value);
    int lenAdditionValue = strlen(additionValue);
    char *mergedValue = new char[lenValue+lenAdditionValue];
    strcpy(mergedValue, value);
    strcat(mergedValue, additionValue);
    string idStr(mergedValue);
    EdgeId eId = hash<string>()(idStr);
    Edge *e = nullptr;
    if (addNewEdge(eList, mergedValue, sourceVId, sinkVId, (ReadId) 0, NOT_INCLUDE_KMER) == 0) {
        cerr << "Error occurs when adding edge #" << eId << ".\n";
        return 0;
    }
    e = eList->at(eId);
    if (nullptr == e) {
        cerr << "Error occurs when adding edge #" << eId << ": Cannot read edge from edge list.\n";
        return 0;
    }

    // Reconstruct some fields.
    e->multiplicity = 0;
    // Reconstruct value
    e->value = nullptr;
    e->value = new char[lenValue];
    if (nullptr == e->value) {
        cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
        return 0;
    }
    strcpy(e->value, value);
    // Assign additionValue to edge
    e->additionValue = new char[lenAdditionValue];
    if (nullptr == e->additionValue) {
        cerr << "Error occurs when adding edge #" << eId << ": Out of memory.\n";
        return 0;
    }
    strcpy(e->additionValue, additionValue);

    // Add paths to the Z edge
    e->multiplicity += copyPathSetToEdge(e, endHerePathSet);
    e->multiplicity += copyPathSetToEdge(e, startFromHerePathSet);
    e->multiplicity += copyPathSetToEdge(e, includeThisPathSet);

    // Delete the unnecessary variables
    delete value;
    delete additionValue;
    delete endHerePathSet;
    delete startFromHerePathSet;
    delete includeThisPathSet;
    return 1;
}

int copyPathSetToEdge(Edge *e, SetOfID *pathSet) {
    int count = 0;
    for (unsigned long itr : *pathSet) {
        e->endHerePathSet->insert(itr);
        count++;
    }
    return count;
}

Edge *fetchEdgeFromList(EdgeList *eList, EdgeId eId) {
    Edge *e = nullptr;
    if (eList->find(eId) == eList->end()) {
        cerr << "Error occurs when fetching an edge #" << eId << "from edge list.\n";
        return nullptr;
    }
    e = eList->at(eId);
    return e;
}

int removeEdge(EdgeList *eList, EdgeId eId) {
    if (eList->find(eId) == eList->end()) {
        cerr << "Error occurs when deleting an non-exist edge #" << eId << ".\n";
        return 0;
    }
    eList->erase(eId);
    return 1;
}

int addReadPathTo(const Edge *e, ReadId rId, KMERPOS_t kmerpos) {
    int count = 0;
    if (START_KMER == (kmerpos & START_KMER)){
        e->startFromHerePathSet->insert(rId);
        count++;
    }
    if (END_KMER == (kmerpos & END_KMER)) {
        e->endHerePathSet->insert(rId);
        count++;
    }
    if (INCLUDE_KMER == (kmerpos & INCLUDE_KMER)) {
        e->includeThisPathSet->insert(rId);
        count++;
    }
    return count;
}

int addReadPathTo(EdgeList *eList, EdgeId eId, ReadId rId, KMERPOS_t kmerpos) {
    if (eList->find(eId) == eList->end()) {
        cerr << "Error occurs when add a read path to the set of an non-exist edge #" << eId << ".\n";
        return 0;
    }
    Edge *e = nullptr;
    e = eList->at(eId);
    return addReadPathTo(e, rId, kmerpos);
}

int removeReadPathFrom(const Edge *e, ReadId rId, KMERPOS_t kmerpos) {
    int count = 0;
    if (START_KMER == (kmerpos & START_KMER)){
        if (e->startFromHerePathSet->find(rId) != e->startFromHerePathSet->end()) {
            e->startFromHerePathSet->erase(rId);
            count++;
        }
    }
    if (END_KMER == (kmerpos & END_KMER)) {
        if (e->endHerePathSet->find(rId) != e->endHerePathSet->end()) {
            e->endHerePathSet->erase(rId);
            count++;
        }
    }
    if (INCLUDE_KMER == (kmerpos & INCLUDE_KMER)) {
        if (e->includeThisPathSet->find(rId) != e->includeThisPathSet->end()) {
            e->includeThisPathSet->erase(rId);
            count++;
        }
    }
    return count;
}

int removeReadPathFrom(EdgeList *eList, EdgeId eId, ReadId rId, KMERPOS_t kmerpos) {
    if (eList->find(eId) == eList->end()) {
        cerr << "Error occurs when removing a read path to the set of an non-exist edge #" << eId << ".\n";
        return 0;
    }
    Edge *e = nullptr;
    e = eList->at(eId);
    return removeReadPathFrom(e, rId, kmerpos);
}

int increaseAvailPassTime(EdgeList *eList, EdgeId eId) {
    Edge *e = fetchEdgeFromList(eList, eId);
    if (nullptr == e) {
        cerr << "Cannot fetch the edge #" << eId << ".\n";
        return -1;
    }
    if (e->availPassTime >= 0)
        return ++e->availPassTime;
    else return -1;
}

int reduceAvailPassTime(EdgeList *eList, EdgeId eId) {
    Edge *e = fetchEdgeFromList(eList, eId);
    if (nullptr == e) {
        cerr << "Cannot fetch the edge #" << eId << ".\n";
        return -1;
    }
    if (e->availPassTime > 0)
        return --e->availPassTime;
    else return -1;
}

bool checkAvailPassTimeNonZero(EdgeList *eList, EdgeId eId) {
    Edge *e = fetchEdgeFromList(eList, eId);
    if (nullptr == e) {
        cerr << "Cannot fetch the edge #" << eId << ".\n";
        return -1;
    }
    return e->availPassTime > 0;
}
