//
// Created by erianliang on 19-7-16.
//

#ifndef RANDOMSTRINGASSEMBLY_IDSET_H
#define RANDOMSTRINGASSEMBLY_IDSET_H
#include <cstdlib>
#include <unordered_set>
typedef std::unordered_set<size_t> SetOfID;

SetOfID *setMinus(const SetOfID &a, const SetOfID &b);

int setMinusTo(SetOfID *a, const SetOfID &b);

SetOfID *setUnion(const SetOfID &a, const SetOfID &b);

int setUnionTo(SetOfID *a, const SetOfID &b);

SetOfID *setIntersect(const SetOfID &a, const SetOfID &b);

int setIntersectTo(SetOfID *a, const SetOfID &b);
#endif //RANDOMSTRINGASSEMBLY_IDSET_H
