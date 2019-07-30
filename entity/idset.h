//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_IDSET_H
#define RANDOMSTRINGASSEMBLY_IDSET_H
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <vector>
#include <algorithm>

inline size_t getId(const std::string& value) {
    return std::hash<std::string>()(value);
}

inline size_t getId(const char *value) {
    return std::hash<std::string>()(value);
}

size_t idBelongTo(int world_size, size_t id);

//typedef std::unordered_set<size_t> SetOfID;
typedef size_t ReadId;
typedef size_t VertexId;
typedef size_t EdgeId;

class SetOfID : public std::vector<size_t> {
public:
    void safe_insert(size_t id) {
        pthread_mutex_lock(&setOfIDMutex);
        this->push_back(id);
        pthread_mutex_unlock(&setOfIDMutex);
    }

    void insert(size_t id) {
        this->push_back(id);
    }

    int count(size_t id) const {
        if (this->end() != std::find(this->begin(), this->end(), id)) {
            return 1;
        }
        return 0;
    }

    void erase_item(size_t id) {
        pthread_mutex_lock(&setOfIDMutex);
        for (int j = 0; j < this->size(); ++j) {
            if (id == *(this->begin()+j)) this->erase(this->begin()+j);
        }
        pthread_mutex_unlock(&setOfIDMutex);
    }

    void erase_last() {
        pthread_mutex_lock(&setOfIDMutex);
        this->pop_back();
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
