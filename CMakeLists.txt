cmake_minimum_required(VERSION 3.13)
project(RandomStringAssembly CXX)
find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")

set(CMAKE_CXX_STANDARD 11)

add_executable(RandomStringAssembly main.cpp entity/k_mer.cpp entity/k_mer.h entity/k_minus_mer.cpp entity/k_minus_mer.h entity/idset.h entity/read.cpp entity/read.h entity/idset.cpp test.cpp test.h blocking_queue.h communicator.cpp communicator.h)
target_link_libraries(RandomStringAssembly PUBLIC MPI::MPI_CXX)
