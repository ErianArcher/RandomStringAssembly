#include <iostream>
#include <mpi.h>
#include <sstream>
#include "entity/idset.h"
#include "entity/read.h"
#include "entity/k_minus_mer.h"

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
    delete vIds;
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

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}