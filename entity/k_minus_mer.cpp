//
// Created by erianliang on 19-7-16.
//

#include "k_minus_mer.h"
#include <iostream>
#include <string.h>

using namespace std;

int addVertex(VertexList *vList, char *value, const EdgeId inKMerId, const EdgeId outKMerId, VertexMode_t mode) {
    string idStr(value);
    VertexId vId = hash<string>()(idStr);
    Vertex *v = nullptr;
    if (vList->find(vId) == vList->end()) { // This vertex does not exist.
        v = new Vertex;
        if (nullptr == v) {
            cerr << "Error occurs when adding vertex #" << vId << ": Out of memory.\n";
            return 0;
        }
        v->id = vId;

        // 不需要存value
        /*// Copy value
        int valueLen = strlen(value);
        //v->value = nullptr;
        v->value = new char[valueLen];
        if (nullptr == v->value) {
            cerr << "Error occurs when adding vertex #" << vId << ": Out of memory.\n";
            return 0;
        }
        strcpy(v->value, value); // coping*/

        // Initial inKMer ID set
        //v->inKMer = nullptr;
        v->inKMer = new SetOfID;
        if (nullptr == v->inKMer) {
            cerr << "Error occurs when adding vertex #" << vId << ": Out of memory.\n";
            return 0;
        }

        // Initial inKMer ID set
        //v->outKMer = nullptr;
        v->outKMer = new SetOfID;
        if (nullptr == v->outKMer) {
            cerr << "Error occurs when adding vertex #" << vId << ": Out of memory.\n";
            return 0;
        }

        // Insert inKMerId
        if (HEAD_VERTEX == (mode & HEAD_VERTEX)) {
            v->inKMer->insert(inKMerId);
            v->inDegree = 1;
        }

        // Insert outKMerId
        if (TAIL_VERTEX == (mode & TAIL_VERTEX)) {
            v->outKMer->insert(outKMerId);
            v->outDegree = 1;
        }

        if (v->inKMer->size() != v->inDegree) {
            cerr << "Error occurs when adding vertex #" << vId << ".\n";
            freeVertex(v);
            return 0;
        }

        if (v->outKMer->size() != v->outDegree) {
            cerr << "Error occurs when adding vertex #" << vId << ".\n";
            freeVertex(v);
            return 0;
        }

        // Insert into vertex list
        vList->insert(make_pair(vId, v));

        if (vList->find(vId) == vList->end()) {
            cerr << "Error occurs when adding vertex #" << vId << ".\n";
            freeVertex(v);
            return 0;
        }

    } else { // This vertex exists.
        v = vList->at(vId);
        if (HEAD_VERTEX == (mode & HEAD_VERTEX)) {
            v->inKMer->insert(inKMerId);
            v->inDegree++;
            if (v->inKMer->size() != v->inDegree) {
                cerr << "Error occurs when adding vertex #" << vId << ".\n";
                v->inDegree--;
                delete value;
                return 0;
            }
        }

        if (TAIL_VERTEX == (mode & TAIL_VERTEX)) {
            v->outKMer->insert(outKMerId);
            v->outDegree++;
            if (v->outKMer->size() != v->outDegree) {
                cerr << "Error occurs when adding vertex #" << vId << ".\n";
                v->outDegree--;
                delete value;
                return 0;
            }
        }
    }
    delete value;
    if (v->outDegree > 1 && v->inDegree > 1) return MULTI_BOTH_DEGREE;
    if (v->inDegree > 1) return MULTI_IN_DEGREE;
    if (v->outDegree > 1) return MULTI_OUT_DEGREE;
    return 1;
}

int removeVertex(VertexList *vList, const VertexId vId) {
    if (vList->find(vId) == vList->end()) {
        cerr << "Error occurs when deleting an non-exist vertex #" << vId << ".\n";
        return 0;
    }
    vList->erase(vId);
    return 1;
}

int addInEdge(const VertexList *vList, const VertexId vId, const EdgeId eId) {
    if (vList->find(vId) == vList->end()) {
        cerr << "Error occurs when adding an in edge to an non-exist vertex #" << vId << ".\n";
        return 0;
    }
    Vertex *v = vList->at(vId);
    if (v->inKMer->find(eId) != v->inKMer->end()) {
        cerr << "Error occurs when adding in edge #" << eId << " to vertex #" << vId << ": Edge exists\n";
        return 0;
    }
    v->inKMer->insert(eId);
    v->inDegree++;
    if (v->inKMer->size() != v->inDegree) {
        cerr << "Error occurs when adding in edge #" << eId << " to vertex #" << vId << ".\n";
        v->inDegree--;
        return 0;
    }
    return 1;
}

int removeInEdge(const VertexList *vList, const VertexId vId, const EdgeId eId) {
    if (vList->find(vId) == vList->end()) {
        cerr << "Error occurs when deleting an in edge of an non-exist vertex #" << vId << ".\n";
        return 0;
    }
    Vertex *v = vList->at(vId);
    if (v->inKMer->find(eId) == v->inKMer->end()) {
        cerr << "Error occurs when deleting an non-exist in edge #" << eId<< " of a vertex #" << vId << ".\n";
        return 0;
    }
    v->inKMer->erase(eId);
    v->inDegree--;
    if (v->inKMer->size() != v->inDegree) {
        cerr << "Error occurs when removing in edge #"<< eId << "from vertex #" << vId << ".\n";
        v->inDegree++;
        return 0;
    }
    return 1;
}

int addOutEdge(const VertexList *vList, const VertexId vId, const EdgeId eId) {
    if (vList->find(vId) == vList->end()) {
        cerr << "Error occurs when adding an out edge to an non-exist vertex #" << vId << ".\n";
        return 0;
    }
    Vertex *v = vList->at(vId);
    if (v->outKMer->find(eId) != v->outKMer->end()) {
        cerr << "Error occurs when adding out edge #" << eId << " to vertex #" << vId << ": Edge exists\n";
        return 0;
    }
    v->outKMer->insert(eId);
    v->outDegree++;
    if (v->outKMer->size() != v->outDegree) {
        cerr << "Error occurs when adding out edge #" << eId << " to vertex #" << vId << ".\n";
        v->outDegree--;
        return 0;
    }
    return 1;
}

int removeOutEdge(const VertexList *vList, const VertexId vId, const EdgeId eId) {
    if (vList->find(vId) == vList->end()) {
        cerr << "Error occurs when deleting an out edge of an non-exist vertex #" << vId << ".\n";
        return 0;
    }
    Vertex *v = vList->at(vId);
    if (v->outKMer->find(eId) == v->outKMer->end()) {
        cerr << "Error occurs when deleting an non-exist out edge #" << eId<< " of a vertex #" << vId << ".\n";
        return 0;
    }
    v->outKMer->erase(eId);
    v->outDegree--;
    if (v->outKMer->size() != v->outDegree) {
        cerr << "Error occurs when removing out edge #"<< eId << "from vertex #" << vId << ".\n";
        v->outDegree++;
        return 0;
    }
    return 1;
}

int freeVertex(Vertex *pVertex) {
    delete pVertex;
    return 1;
}

kMinusMer::~kMinusMer() {
    /*if (nullptr != value) {
        delete value;
        value = nullptr;
    }*/
    if (nullptr != inKMer) {
        delete inKMer;
        inKMer = nullptr;
    }
    if (nullptr != outKMer) {
        delete outKMer;
        outKMer = nullptr;
    }
}

kMinusMer::kMinusMer() {
    id = 0;
    //value = nullptr;
    inKMer = nullptr;
    outKMer = nullptr;
    inDegree = 0;
    outDegree = 0;
}
