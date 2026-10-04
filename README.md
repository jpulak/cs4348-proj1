# cs4348-proj1 Concurrent Prrefix Sum

concurrent prefix sum using fork() and sys v shared memory

##Team members
- Jana Pulak
- Cristina Luu

Team Number - 21

project layout- 
```
cs4348-proj1/
├── Makefile
├── README.md
├── include/
│   └── prefix_sum.h
└── src/
    ├── main.cpp
    └── worker.cpp
```


## Program description
This program is going to calcuate the prefix sum of an arry using the concurrent Hillies-stelle algorithm. For example the
```
1 2 3 4
produces
1 3 6 10
```

the parent process reads the input file, creates the request number of worker process using fork() and then write the file resule into the output file

The worker processes divide the arry indices as evenly as possible and then performs each round of the algorithm concurrently. System V shared memory stores the input values, intermediate results, final output and barrier flags. The barriers helps ensure that every worker finishes the current round before any worker begins the next round

## Requirements
Porgram has to be compied and executed in a UNIX/Linux envrionment
- g=+ with C++ 17 support
- make

This program uses Linuz system calls such as fork(), shmget(), shmat(), it shouldnt be comiled directly through Windows Powershell

## Project Structure
After extracting ZIP file provided, the project should contain 

```
Makefile
README.md
include/
    prefix_sum.h
src/
    main.cpp
    worker.cpp
```

## Compilation instruction
Open a terminal in the extracted project directory, the directory containing the Makefile

Compile the program by running
``` make ```
creating an executable named ```my-sum```

make sure the compilation will be clean so run
```
make clean
make
```

## Execution instruction
run the program using the  following format:
```./my-sum <n> <m> <input-file> <output-file>```
The four args are as followed
- n : Number of values to read from teh input file, it must be greater than 0
- m: Number of worker processes, it must satisfy 1<=m<=n
- input-file : path to a file containing at least n integer values
- output-file : Path where the prefix-sum results will be written

The input values hsould be seperated by spaces, tabs, or new lines

## Example

Here is a example input file-
```printf "1 2 3 4 5 6 7 8\n" > input.txt```

Compile the program
```make```

Run the program with eight input elements and four worker processes
```./my-sum 8 4 input.txt output.txt```

Display output
```cat output.txt```
Expected output
```1 3 6 10 15 21 28 36```

If the program successfully completes, then the prefix sum results will be stored int hte specified output file

## Clean project
To remove compiled my-sum executable, run
```make clean```
