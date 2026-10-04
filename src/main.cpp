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

    //store ids of the children created by parent
    vector<pid_t> childPids;
    bool forkFailed= false;
    
    // creae exactly m child processes
    for(int workerId=0; workerId<workerCnt; workerId++){
        pid_t childPid = fork();

        //if PID -1, child creation failed
        if (childPid== -1){
            cerr<<"error, couldnt create woker process" << endl;
            forkFailed= true;
            break;
        }

        // fork return pid 0 indide newly creted child
        if (childPid ==0){
            //waiting for parent to finish creating ALL child
            while (*startFlag ==0){}
            //exit if the parent cant create every child
            if(*startFlag ==-1){
                shmdt(valAddress);
                shmdt(barrierAddress);
                _exit(EXIT_FAILURE);
            }

            //parent successfully created all children, now run child assigned  calcualtion
            runWorker(workerId, elementCnt, workerCnt,roundCnt,sharedVals,barrierFlags);

            //after, child no longer needs shared mem attachemnets
            shmdt(valAddress);
            shmdt(barrierAddress);

            _exit(EXIT_SUCCESS);
        }
    childPids.push_back(childPid);
    }

        //handle fork failure wihtout leaving children waiting
    if (forkFailed){
        *startFlag = -1;

        for(pid_t childPid: childPids){
            waitpid(childPid, nullptr,0);
        }

        // detach and delete the shared memory
        shmdt(valAddress);
        shmdt(barrierAddress);
        shmctl(valShmId,IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);

        return EXIT_FAILURE;
    }
    //every worker was created successfully, start now
    *startFlag = 1;

    //parent wait for EVERY child to finish
    bool workerSuccess =true;

    //assume all workers has succeded, go through each validation for any possible failures
    for(pid_t childPid:childPids){
        int stat;
        if(waitpid(childPid, &stat, 0) == -1){
            cerr<<"error, cant wiat for worder process"<<endl;
            workerSuccess= false;
            continue;
        }
        //check if the child exited norramlly and successfully, otherweise error and failed
        if (!WIFEXITED(stat)||WEXITSTATUS(stat) != EXIT_SUCCESS){
            cerr<<"error, worker process has failed"<< endl;
            workerSuccess = false; 
        }
    }

    //stop if any of the workers has failed
    if (!workerSuccess){
        //detach parent from shared memroy segments shmdt, and delete shared memory segmenets shmctl
        shmdt(valAddress);
        shmdt(barrierAddress);
        shmctl(valShmId,IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);
        return EXIT_FAILURE;
    }

    //open output file
    ofstream outputFile (argv[4]);

    if (!outputFile.is_open()){
        cerr<<"couldnt open file"<<endl;

        // detach and delete the shared memory
        shmdt(valAddress);
        shmdt(barrierAddress);
        shmctl(valShmId,IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);
        return EXIT_FAILURE;
    }

    //minal result stored in the last shared memory row
    size_t finalRow= static_cast<size_t>(roundCnt) * elementCnt;

    //write final prefix sum arry into the output file
    for (int i=0; i<elementCnt; i++){
        outputFile <<sharedVals[finalRow+i];
        if(i<elementCnt-1){
            outputFile<<" ";
        }
    }
    outputFile<<endl;

    //chck fi the writing failed
    if(!outputFile){
        cerr<<"didnt write to the output file"<<endl;
        outputFile.close();
        shmdt(valAddress);
        shmdt(barrierAddress);
        shmctl(valShmId,IPC_RMID, nullptr);
        shmctl(barrierShmId, IPC_RMID, nullptr);
        return EXIT_FAILURE;        
    }
    outputFile.close();


    //detatch and then remove all shared memroy segments
    bool cleanup = true;

    //check each shared memory to see if it has detach and deleted to remove positibility of memory leak
    if(shmdt(valAddress) == -1){
        cerr<< "didnt detach value shared memory"<<endl;
        cleanup = false;
    }
    if(shmdt(barrierAddress) ==-1){
        cerr<<"couldnt detach barrier shared memory" << endl;
        cleanup= false;
    }
    if (shmctl(valShmId,IPC_RMID, nullptr) == -1){
        cerr<<"couldnt delete value shared memroy" << endl;
        cleanup= false;
    }
    if (shmctl(barrierShmId, IPC_RMID, nullptr)==-1){
        cerr<<"couldnt delete barrier shared memroy"<< endl;
        cleanup= false;
    }
    
    if(!cleanup)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

    //return success after program finishes correctly
    return 0;
}