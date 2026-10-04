#include "prefix_sum.h"

int calculateRoundCount(int n) 
{
	int roundCount = 0;
	long long offset = 1; //jana revised from int (needed to prevent overflow)

	//calculate number of rounds ceil(log2(n))
	while (offset < n) 
    {
		offset *= 2;
		roundCount++;
	}

	return roundCount;
}

//revised arriveAndWait func to implement reusable barrier (5% extra pt)
void arriveAndWait(volatile int* barrierFlags, int round, int workerId, int workerCnt)
{
    //each round represents a new barrier gen
    int generation = round+1;

    //record that this specific worker reached current generation
    barrierFlags[workerId] = generation;

    while(true){
        int allArrived =1;

        //wait till every worker reached this generation
        for (int i = 0; i < workerCnt; i++)
        {
            if (barrierFlags[i] <generation){
                allArrived=0;
                break;
            }
        }
        if(allArrived){
            break;
        }
    }
}

/* 
void arriveAndWait(volatile int* barrierFlags, int round, int workerId, int workerCnt) 
{
	//mark the worker as arrived at the barrier
	barrierFlags[round * workerCnt + workerId] = 1;

	while (true) 
    {
		//wait for all workers to arrive at the barrier
		int allArrived = 1;
		//if any of the flags in the round equal 0, the loop keeps spinning
		for (int i = 0; i < workerCnt; i++) 
        {
			if (barrierFlags[round * workerCnt + i] == 0) 
            {
				allArrived = 0;
				break;
			}
		}
		if (allArrived) 
        {
			break;
		}
	}
}
*/


void runWorker(int workerId, int elementCnt, int workerCnt, int roundCnt,
	volatile long long* sharedVals, volatile int* barrierFlags)
     {
	int offset = 1;

	//calculate the number of elements each worker will process
	int baseWork = elementCnt / workerCnt;
	int remainder = elementCnt % workerCnt;

	//calcualte the starting index for each worker
	int startIndex = workerId * baseWork + (workerId < remainder ? workerId : remainder);

	int workCount = baseWork;

	//increment the base work for the workers that will process the remainder
	if (workerId < remainder) 
    {
		workCount++;
	}

	//go through every round of the Hillis-Steele algorithm
	for (int round = 0; round < roundCnt; round++) 
    {
		//offset is set at 1 for the first round and doubles each time
		if (round != 0) 
        {
			offset = offset * 2;
		}

		//make calcluations according to Hillis-Steele algorithm
		for (int i = startIndex; i < startIndex + workCount; i++) 
        {
			//if j<2^i then / copies prev val when index is < than offset
			if (i < offset) 
            {
				//x^i+1_j = x^i_j / add prev val to val offset position to its left
				sharedVals[(round + 1) * elementCnt + i] = sharedVals[round * elementCnt + i];
			}
			else 
            {
				//x^i+1_j = x^i_j + x^i_j-2^i
				int offsetIndex = i - offset;
				sharedVals[(round + 1) * elementCnt + i] = sharedVals[round * elementCnt + offsetIndex] + sharedVals[round * elementCnt + i];
			}
		}

		//call the barrier to wait for all other workers to finish
		arriveAndWait(barrierFlags, round, workerId, workerCnt);
	}

}