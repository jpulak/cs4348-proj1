// connects teh main and worker files, common funcs

#ifndef PREFIX_SUM_H
#define PREFIX_SUM_H

 //calc the number of rounds that is needed for n elements
int calculateRoundCount(int n); //main and worker file (jana and cristina)

//resuable const-space barrier
// barrierFlags[0] stores the next worker id allowed to report arrival
//barrierFlags[1] stores the latest completed generation
void arriveAndWait(
    volatile int* barrierFlags, //shared memory flag to show which worker finished
    int round, //curr round
    int workerId, //what worker
    int workerCnt //how many workers it has to wait for
); //implemented n called by worker file (cristina)

//perfomrs a workers assigned prefix sum calc
//each child should call this func after being created
void runWorker(
    int workerId, //current child identifier
    int elementCnt, //number of input values, n
    int workerCnt, //number of child processes, m
    int roundCnt, //total num of calc rounds
    volatile long long* sharedVals, //point to shared memory of arrsy with input, intermediate results, and final output
    volatile int* barrierFlags // point ot shared memory barrier flag
); //implemented in worker file (cristina), called from main file by each child (jana)


#endif
