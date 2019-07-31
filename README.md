# RandomStringAssembly
A final project from the course, Algorithm, requires to rebuild a string with its substrings. This implementation of sequence assembly is based on **De Bruijn Graph**.
## Prerequisite
- MPICH 3.3.1
- PTHREAD
- C++ 11 supported
## Build this project
1. `mkdir build; cd build`
2. `cmake ../`
3. `make`

## Run the program
After executing the command above, a program named `RandomStringAssembly` is generated in the current directory. It should be run by the command of below:
```
mpiexec -np {the number of processes/hosts} ./RandomStringAssembly {the value of k of k-mer} {input folder} {the filename of the file of reads in the input folder}
```
. Multiple filenames can be splitted by white space following the argument of `{input folder}`.

For example:

- Command for running the program with 6 processes, k-mer with 79 characters, and input file named "reads" in the folder of "input".
```
mpiexec -np 6 ./RandomStringAssembly 79 "input" "reads"
```

- Command for running the program with 6 processes, k-mer with 79 characters, and input files named "reads1" and "reads2" in the folder of "input".
```
mpiexec -np 6 ./RandomStringAssembly 79 "input" "reads1" "reads2"
```
