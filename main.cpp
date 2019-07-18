#include <iostream>
#include <mpi.h>
#include <sstream>
#include "entity/idset.h"
#include "entity/read.h"

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
    SetOfID *readIds = new SetOfID;
    for (int i = 0; i < len; ++i) {
        ReadId *readId = new ReadId;
        createRead(reads[i], readId);
        readIds->insert(*readId);
        delete readId;
    }

    cout << "Test getRead:" << endl;
    string *oread = new string;
    for (auto readId : *readIds) {
        getRead(oread, readId);
        cout << "Read id #" << readId << ": " << *oread << endl;
    }

    cout << "Test read alteration:" << endl;
    int alteredLen = 1;
    int no = 0;
    int alteredReadId = *readIds->begin();
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

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}