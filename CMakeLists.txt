cmake_minimum_required(VERSION 3.13)
project(RandomStringAssembly CXX)
find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")

set(CMAKE_CXX_STANDARD 11)

add_executable(RandomStringAssembly main.cpp)
target_link_libraries(RandomStringAssembly PUBLIC MPI::MPI_CXX)
