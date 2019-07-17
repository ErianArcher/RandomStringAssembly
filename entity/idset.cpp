//
// Created by erianliang on 19-7-17.
//

#include <iostream>
#include <algorithm>
#include "idset.h"
using namespace std;

static void copySetTo(SetOfID *dest, const SetOfID *src) {
    dest->clear();
    for (auto id : *src) {
        dest->insert(id);
    }
}

SetOfID *setMinus(const SetOfID &a, const SetOfID &b) {
    SetOfID *result = nullptr;
    result = new SetOfID;
    set_difference(a.begin(), a.end(), b.begin(), b.end(), inserter(result, result->begin()));
    return result;
}


int setMinusTo(SetOfID *a, const SetOfID &b) {
    SetOfID *r = setMinus(*a, b);
    copySetTo(a, r);
    delete r;
    return a->size();
}

SetOfID *setUnion(const SetOfID &a, const SetOfID &b) {
    SetOfID *result = nullptr;
    result = new SetOfID;
    set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(result, result->begin()));
    return result;
}

int setUnionTo(SetOfID *a, const SetOfID &b) {
    SetOfID *r = setUnion(*a, b);
    copySetTo(a, r);
    delete r;
    return a->size();
}

SetOfID *setIntersect(const SetOfID &a, const SetOfID &b) {
    SetOfID *result = nullptr;
    result = new SetOfID;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(result, result->begin()));
    return result;
}

int setIntersectTo(SetOfID *a, const SetOfID &b) {
    SetOfID *r = setIntersect(*a, b);
    copySetTo(a, r);
    delete r;
    return a->size();
}
