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

    //convert n and m into ints and validate 

    //read the first n values form th einput file

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