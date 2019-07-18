#include <iostream>
#include <mpi.h>
#include <sstream>
#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"
#include "entity/k_mer.h"

using namespace std;

string setToString(SetOfID *set) {
    stringstream sstream;
    sstream << "{";
    for (auto itr: *set) {
        sstream << itr << ", ";
    }
    sstream << "}";
    return sstream.str();
}

void idSetTest() {
    SetOfID *set1 = new SetOfID;
    SetOfID *set2 = new SetOfID;
    SetOfID *resset = nullptr;
    set1->insert(hash<string>()("Hello"));
    set1->insert(hash<string>()("world"));
    set2->insert(hash<string>()("Hello"));
    set2->insert(hash<string>()("erian"));
    cout << "set1: " << setToString(set1) << "\n" <<
         "set2: " << setToString(set2) << "\n";
    cout << "set1 union set2: ";
    resset = setUnion(*set1, *set2);
    cout << setToString(resset) << "\n";
    delete resset;
    cout << "set1 intersect set2: ";
    resset = setIntersect(*set1, *set2);
    cout << setToString(resset) << "\n";
    delete resset;
    cout << "set1 minus set2: ";
    resset = setMinus(*set1, *set2);
    cout << setToString(resset) << "\n";
    delete resset;

    cout << "set1 union set2: ";
    setUnionTo(set1, *set2);
    cout << setToString(set1) << "\n";
    cout << "set1 intersect set2: ";
    setIntersectTo(set1, *set2);
    cout << setToString(set1) << "\n";
    cout << "set2 minus set1: ";
    setMinusTo(set2, *set1);
    cout << setToString(set2) << "\n";
}

void readTest() {
    createReadDir();
    string reads[] = {"abcdefghabcijklmnabc", "gdfeabcgvfaondfabcevob", "guabceprgvujbgorpgvfadpuoguahbabc",
                      "abcjfoaihjgoigoreabcbug", "abcvgnedfaiosngjogrenbfoijgbnv", "fioeawrhgophnoarhgouiabc",
                      "goierahabcfreoiaghoi"};
    int len = 7;
    ReadId *readIds = new ReadId[len];
    for (int i = 0; i < len; ++i) {
        ReadId *readId = new ReadId;
        createRead(reads[i], readId);
        readIds[i] = *readId;
        delete readId;
    }

    cout << "Test getRead:" << endl;
    string *oread = new string;
    for (int i = 0; i < len; ++i) {
        getRead(oread, readIds[i]);
        cout << "Read id #" << readIds[i] << ": " << *oread << endl;
    }

    cout << "Test read alteration:" << endl;
    int alteredLen = 1;
    int no = 0;
    ReadId alteredReadId = readIds[no];
    stringstream ss;
    ss << "b" << reads[no];
    enlargeRead(ss.str(), alteredLen, alteredReadId, ALTER_HEAD);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;
    cutRead(reads[no], alteredLen, alteredReadId, ALTER_HEAD);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;

    ss.str("");
    ss << reads[no] << "a";
    enlargeRead(ss.str(), alteredLen, alteredReadId, ALTER_TAIL);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;
    cutRead(reads[no], alteredLen, alteredReadId, ALTER_TAIL);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;

    ss.str("");
    ss << "b" << reads[no] << "a";
    enlargeRead(ss.str(), 2, alteredReadId, ALTER_BOTH);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;
    cutRead(reads[no], 2, alteredReadId, ALTER_BOTH);
    getRead(oread, alteredReadId);
    cout << "Read id #" << alteredReadId << ": " << *oread << endl;

    KMERPOS_t flag;
    if ((flag = getKMerPositionInRead("abc", reads[no])) != (START_KMER | INCLUDE_KMER | END_KMER)) {
        cout << "Mismatch: " << reads[no] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[6])) != (INCLUDE_KMER)) {
        cout << "Mismatch: " << reads[6] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[5])) != (END_KMER)) {
        cout << "Mismatch: " << reads[5] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[4])) != (START_KMER)) {
        cout << "Mismatch: " << reads[4] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[3])) != (START_KMER | INCLUDE_KMER)) {
        cout << "Mismatch: " << reads[3] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[2])) != (INCLUDE_KMER | END_KMER)) {
        cout << "Mismatch: " << reads[2] << "; flag: " << flag << endl;
    }

    if ((flag = getKMerPositionInRead("abc", reads[1])) != (INCLUDE_KMER)) {
        cout << "Mismatch: " << reads[1] << "; flag: " << flag << endl;
    }

    delete oread;
    delete readIds;
}

string vertexToString(Vertex *v) {
    stringstream sstream;
    sstream << "{";
    sstream << "VertexId: " << v->id << ", ";
    sstream << "InEdges:(" << v->inDegree << ") " << setToString(v->inKMer) << ", ";
    sstream << "OutEdges:(" << v->outDegree << ") " << setToString(v->outKMer) << "";
    sstream << "}";
    return sstream.str();
}

void vertexTest() {
    VertexList *vertexList = new VertexList;
    EdgeId eids[] = {1, 2, 3, 4};
    VertexId *vIds = new VertexId[4];
    char *kMinusMers[] = {"hello", "world", "erian", "isolation"};
    addVertex(vertexList, kMinusMers[0], (EdgeId) 0, eids[0], HEAD_VERTEX);
    if (addVertex(vertexList, kMinusMers[0], (EdgeId) 0, eids[3], HEAD_VERTEX) != MULTI_OUT_DEGREE)
        cout << "Multi out degree fails" << endl;
    addVertex(vertexList, kMinusMers[1], eids[2], eids[3], IN_VERTEX);
    if (addVertex(vertexList, kMinusMers[1], eids[0], eids[1], IN_VERTEX) != MULTI_BOTH_DEGREE)
        cout << "Multi both degree fails" << endl;
    addVertex(vertexList, kMinusMers[2], eids[1], (EdgeId) 0, TAIL_VERTEX);
    if (addVertex(vertexList, kMinusMers[2], eids[0], (EdgeId) 0, TAIL_VERTEX) != MULTI_IN_DEGREE)
        cout << "Multi in degree fails" << endl;
    addVertex(vertexList, kMinusMers[3], &vIds[3], (EdgeId) 0, (EdgeId) 0, ISOLATE_VERTEX);
    for (int i = 0; i < 4; ++i) {
        VertexId vId = hash<string>()(kMinusMers[i]);
        if (i == 3 && vId != vIds[i]) cout << "Error in addVertex() for fetching vertex id." << endl;
        vIds[i] = vId;
        cout << vertexToString(vertexList->at(vId)) << endl;
    }

    cout << "=============================" << endl;
    Vertex *op = vertexList->at(vIds[3]);
    addInEdge(vertexList, vIds[3], eids[0]);
    cout << vertexToString(op) << endl;
    removeInEdge(vertexList, vIds[3], eids[0]);
    cout << vertexToString(op) << endl;
    addOutEdge(vertexList, vIds[3], eids[0]);
    cout << vertexToString(op) << endl;
    removeOutEdge(vertexList, vIds[3], eids[0]);
    cout << vertexToString(op) << endl;

    cout << "=============================" << endl;
    removeVertex(vertexList, vIds[3]);
    for (auto kv: *vertexList) {
        cout << vertexToString(kv.second) << endl;
    }

    delete vertexList;
    delete[] vIds;
}

string edgeToString(Edge *edge) {
    stringstream sstream;
    sstream << "{";
    sstream << "EdgeId: " << edge->id << ", ";
    sstream << "Value: " << edge->value << ", ";
    sstream << "AddtionValue:(" << edge->isZ << ")(" << edge->sizeOfAdditionValue << ") "
    << (edge->additionValue?edge->additionValue:"") << ", ";
    sstream << "Connection: " << edge->sourceKMinusMerId << "->" << edge->sinkKMinusMerId << ", ";
    sstream << "Multiplicity: " << edge->multiplicity << ", ";
    sstream << "AvailPassTime: " << edge->availPassTime << ", ";
    sstream << "endHerePathSet: " << setToString(edge->endHerePathSet) << ", ";
    sstream << "startFromHerePathSet: " << setToString(edge->startFromHerePathSet) << ", ";
    sstream << "includeThisPathSet: " << setToString(edge->includeThisPathSet);
    return sstream.str();
}

void edgeTest() {
    VertexId vIds[] = {1, 2, 3, 4};
    ReadId rIds[] = {1, 2, 3, 4};
    char *kMers[] = {"hello", "world", "erian", "multireads"};
    EdgeList *edgeList = new EdgeList;
    EdgeId *eIds = new EdgeId[4];
    addNewEdge(edgeList, kMers[0], vIds[1], vIds[0], rIds[0], INCLUDE_KMER);
    addNewEdge(edgeList, kMers[1], vIds[0], vIds[2], rIds[1], START_KMER);
    addNewEdge(edgeList, kMers[1], vIds[0], vIds[2], rIds[2], END_KMER);
    addNewEdge(edgeList, kMers[2], vIds[0], vIds[3], rIds[2], END_KMER);
    addNewEdge(edgeList, kMers[3], &eIds[3], vIds[1], vIds[3], rIds[3], START_KMER | INCLUDE_KMER | END_KMER);

    for (int i = 0; i < 4; ++i) {
        EdgeId eId= hash<string>()(kMers[i]);
        if (i == 3 && eId != eIds[i]) cout << "Error in addNewEdge() for fetching edge id." << endl;
        eIds[i] = eId;
        cout << edgeToString(edgeList->at(eId)) << endl;
    }

    Edge *op = edgeList->at(eIds[3]);
    EdgeId *zeId = new EdgeId;
    addReadPathTo(edgeList, eIds[3], rIds[1], START_KMER);
    addNewZEdge(edgeList, op->value, "z", zeId, op->sourceKMinusMerId, vIds[2],
                op->endHerePathSet, op->startFromHerePathSet,op->includeThisPathSet);
    cout << edgeToString(edgeList->at(*zeId)) << endl;
    addNewZEdge(edgeList, op->value, "z", zeId, op->sourceKMinusMerId, vIds[2],
                op->startFromHerePathSet, op->startFromHerePathSet,op->includeThisPathSet);
    cout << edgeToString(edgeList->at(*zeId)) << endl;
    cout << "=======removing edge from z edge ===============" << endl;
    removeReadPathFrom(edgeList, *zeId, rIds[1], END_KMER);
    cout << edgeToString(edgeList->at(*zeId)) << endl;
    removeEdge(edgeList, *zeId);
    cout << "==================================" << endl;
    for (auto kv : *edgeList) {
        cout << edgeToString(kv.second) << endl;
    }
    reduceAvailPassTime(edgeList, eIds[3]);
    if (!checkAvailPassTimeNonZero(edgeList, eIds[3])) cout << "1Edge #" << eIds[3] << "cannot be passed anymore." << endl;
    increaseAvailPassTime(edgeList, eIds[3]);
    if (!checkAvailPassTimeNonZero(edgeList, eIds[3])) cout << "2Edge #" << eIds[3] << "cannot be passed anymore." << endl;

    delete[](eIds);
    delete edgeList;
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    cout << hash<string>()("hello") << "\n" << hash<string>()("hella") << "\n";
    idSetTest();
    readTest();
    vertexTest();
    edgeTest();

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}