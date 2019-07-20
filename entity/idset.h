//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_IDSET_H
#define RANDOMSTRINGASSEMBLY_IDSET_H
#include <cstdlib>
#include <unordered_set>
#include <pthread.h>
#include <string>

inline size_t getId(std::string value) {
    return std::hash<std::string>()(value);
}

//typedef std::unordered_set<size_t> SetOfID;
typedef size_t ReadId;
typedef size_t VertexId;
typedef size_t EdgeId;

class SetOfID : public std::unordered_set<size_t> {
public:
    void safe_insert(size_t id) {
        pthread_mutex_lock(&setOfIDMutex);
        this->insert(id);
        pthread_mutex_unlock(&setOfIDMutex);
    }

private:
    // TODO: 释放资源
    pthread_mutex_t setOfIDMutex = PTHREAD_MUTEX_INITIALIZER;
};

SetOfID *setMinus(const SetOfID &a, const SetOfID &b);

int setMinusTo(SetOfID *a, const SetOfID &b);

SetOfID *setUnion(const SetOfID &a, const SetOfID &b);

int setUnionTo(SetOfID *a, const SetOfID &b);

SetOfID *setIntersect(const SetOfID &a, const SetOfID &b);

int setIntersectTo(SetOfID *a, const SetOfID &b);
#endif //RANDOMSTRINGASSEMBLY_IDSET_H
