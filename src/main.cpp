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

    //calc required shared memory sizes

    //create shared mem for the vals nd barrier flag

    //attach shared mem segment to parent process

    //copy input into shared memroy and initialize all flagss

    //c reate exactly m child process w/ fork

    //have ach child call the runWorker func with its worker id

    //parent wait for EVERY child to finish

    //write final shared memory row ot the output file

    //detatch and then remove all shared memroy segments

    //return success after program finishes correctly


    return 0;
}