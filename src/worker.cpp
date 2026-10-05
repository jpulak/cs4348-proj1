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
//revised arriveAndWait for the reusable barrier to use only O(1) space(10% extra pt)
void arriveAndWait(volatile int* barrierFlags, int round, int workerId, int workerCnt)
{
	while (barrierFlags[0] != workerId) {
		//wait for the worker's turn to arrive
	}

	if (workerId == workerCnt - 1) {
		//release workers waiting for complete generation
		barrierFlags[1] = round + 1;

		//reset the arrival turn for the next generation
		barrierFlags[0] = 0;
	}
	else {
		barrierFlags[0] = workerId + 1;
		
		while (barrierFlags[1] < round +1) {
			//wait for the last worker to finish and them update the generation flag
		}
	}

}

//revised runWorker to use only O(n) space (10% extra pt)
void runWorker(int workerId, int elementCnt, int workerCnt, int roundCnt,
	volatile long long* sharedVals, volatile int* barrierFlags)
     {
	int offset = 1; //offset for Hillis-Steele algorithm
	//we only need the previous row and the current row for the Hillis-Steele algorithm, so we can use two rows in shared memory to store the values
	int currentOffset;
	int nextOffset;

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
		//will go back and forth between the current and next offset for each round(generation)
		if (round % 2 == 0)
		{
			currentOffset = 0; // read from row 0
			nextOffset = elementCnt; // write to row 1
		}
		else
		{
			currentOffset = elementCnt; // read from row 1
			nextOffset = 0; // write to row 0
		}

		//make calcluations according to Hillis-Steele algorithm
		for (int i = startIndex; i < startIndex + workCount; i++) 
        {
			//if j<2^i then / copies prev val when index is < than offset
			if (i < offset) 
            {
				//x^i+1_j = x^i_j / add prev val to val offset position to its left
				sharedVals[nextOffset + i] = sharedVals[currentOffset + i];
			}
			else 
            {
				//x^i+1_j = x^i_j + x^i_j-2^i
				int offsetIndex = i - offset;
				sharedVals[nextOffset + i] = sharedVals[currentOffset + offsetIndex] + sharedVals[currentOffset + i];
			}
		}

		//call the barrier to wait for all other workers to finish
		arriveAndWait(barrierFlags, round, workerId, workerCnt);

		offset *= 2; //double the offset for the next round
	}

}