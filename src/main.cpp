#include "prefix_sum.h"

//necessary imports
#include <iostream> 
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

#include <sys/ipc.h> //system v shared mem support
#include <sys/shm.h> //import needed funcs
#include <sys/types.h>//unix process types
#include <sys/wait.h> //wiat for pid func
#include <unistd.h> //fork/exit, etc funcs

using namespace std;

int main(int argc, char* argv[])
{
    //first check the user provided all 4 args

    if (argc !=5){
        cerr<< "please provide valid input like this - " <<argv[0]
        << " <n> <m> <input-file name> <output-file name>"
        <<endl;
        return EXIT_FAILURE;
    }

    //convert n and m into ints and validate w/ all possibilities
    int elementCnt;
    int workerCnt;
    try{
        size_t nChars;
        size_t mChars;

        elementCnt= stoi(argv[1], &nChars); //text-int -n
        workerCnt = stoi(argv[2], &mChars); //text-int- m

        //make sure entire arg are valid ints
        if (nChars !=string(argv[1]).length() ||mChars !=string(argv[2]).length()){
            cerr<<"n and m have ot be integers, invalid type"<<endl;
            return EXIT_FAILURE;
        }
    }
    catch (...)
    {
        cerr << "error n and m need to be valid integers" << endl;
        return EXIT_FAILURE;
    }

    //validate n and m
    if (elementCnt <= 0 || workerCnt <=0){
        cerr<< "n and m need to be greater than 0, error detected" << endl;
        return EXIT_FAILURE;
    }
    if (workerCnt >elementCnt){
        cerr<<"m CANNOT be greater than n" << endl;
        return EXIT_FAILURE;
    }

    //store in/out files
    string inFileName= argv[3];
    string outFileName=argv[4];

    //open input file
    ifstream inputFile(inFileName);

    if (!inputFile){
        cerr<<"can't open input file" <<endl;
        return EXIT_FAILURE;
    }

    //read first n val from teh input file
    vector<long long> inputVals(elementCnt);

    for(int i=0; i< elementCnt; i++){
        if(!(inputFile >>inputVals[i])){
            cerr<<"error, input file has to have at least "<< elementCnt <<" valid numbers"<< endl;
            return EXIT_FAILURE;
        }
    }
    inputFile.close();

    //calc how many hillis steele rounds needed
    int roundCnt =calculateRoundCount(elementCnt); //call worker.cpp func

    //calc required shared memory sizes

    size_t valueCnt= static_cast<size_t>(roundCnt +1)* elementCnt; //one row for og input, one new row for every calc round
    size_t barrierCnt = static_cast<size_t> (roundCnt) * workerCnt;// every round needs one barrier flag foe very worker
    size_t valueBytes = valueCnt * sizeof(long long); //shmget requries teh number of bytes
    //one extra int for worker starter flag
    size_t barrierBytes= (barrierCnt + 1) * sizeof(int); //wait till parent creates every worker

    //create shared mem for the vals 
    //IPC_PRIVATE- creates a System V shared mem segment
    //IPC_CREAT | 0600- creates a new segment for this specific program
    
    //so create segmnet, allow the current user to read/write it

    int valShmId = shmget( IPC_PRIVATE, valueBytes, IPC_CREAT | 0600);
    //segment 1- valueShmID- input, intermediate rows, final output

    if (valShmId == -1){
        cerr<<"cant create shared memory for vals" << endl;
        return EXIT_FAILURE;
    }

    //create shared mem for barrier flags
    int barrierShmId = shmget( IPC_PRIVATE, barrierBytes, IPC_CREAT | 0600);
    //segment 2- varrier flags, worker start flag

    if (barrierShmId == -1){
        cerr << "couldnt create shared memory for barrier flags" << endl;
        //remove first segment cuz the second creation failed
        shmctl(valShmId, IPC_RMID, nullptr);// prevent shared memory leak

        return EXIT_FAILURE;
    }


    //attach the value seg to the parent
    /*
    functions needed/learned-
    shmget()- create shraed memory and get an id
    shmat()- attach it and get a usable address
    shmdt()- detach address
    shmctl()-delete shared memory segment
    */

    void*valAddress = shmat(valShmId, nullptr,0);

    //check if attachment failed, -1 on failure
    if (valAddress == reinterpret_cast<void*>(-1)){
        cerr<<"error, couldnt attach shared memory for the values"<< endl;

        //delete both shared memory segments 
        shmctl(valShmId, IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);

        return EXIT_FAILURE;
    }

    //convert attached address into the pointer type needed to store and access long long values
    volatile long long* sharedVals= static_cast<volatile long long*> (valAddress);
    //barrier segmet to parent
    void* barrierAddress = shmat(barrierShmId, nullptr, 0);

    //check attachment failed- -1 = failure
    if(barrierAddress == reinterpret_cast<void*>(-1)){
        cerr<<"error, couldnt attach shared memory for barriers" << endl;

        //detach values segment since it was attached successfully before
        shmdt (valAddress);

        //delete both segments to prevent memory leak
        shmctl(valShmId, IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);
        return EXIT_FAILURE;
    }
    //convert general barrier address -> integer pointer
    volatile int* barrierFlags = static_cast<volatile int*> (barrierAddress);

    //copy input vals into shared memory row 0
    for (int i=0; i< elementCnt; i++){
        sharedVals[i] = inputVals[i]; //worker reads this row during the first calc round
    }
    //initialize each barrier flag to 0
    for (size_t i=0; i< barrierCnt; i++){
        barrierFlags[i] = 0; //mark each worker as not finished before the calcuation begins
    }

    //0- children wait, 1- all children created, start working, -1- forck failed, children exit
    volatile int* startFlag = barrierFlags+barrierCnt; 
    *startFlag =0;

    //c reate exactly m child process w/ fork

    //have ach child call the runWorker func with its worker id

    //parent wait for EVERY child to finish

    //write final shared memory row ot the output file

    //detatch and then remove all shared memroy segments

    //return success after program finishes correctly


    return 0;
}