
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "textFile.h"
#include "mpi.h"
#include <omp.h>

Input* input;
MPI_struct* MPI;


int main(int argc, char* argv[]){

	int offset=0;
	int len1,len2;

	input=malloc(sizeof(Input));
	MPI=malloc(sizeof(MPI_struct));

	MPI->tag=0;
	MPI->slave=0;
	MPI->slave_result=NULL;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &MPI->my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &MPI->p);

	if (MPI->my_rank ==0){		
		readInputFromTextFile(input,argv[1]);
		runMaster((MPI->p)-1,argv[2]);
	}
	else{
		runSlaves(len1,len2);
	}
	MPI_Finalize();
	free(input);
	free(MPI);
	return 0;
}



void runMaster(int numOfSlaves,char* fileName){

	char *sortedCharsByWeights;
	int conservativeConflict;

	sortedCharsByWeights = (char*) malloc(WEIGHTS_ARR_LEN * sizeof(char));
	int len1=strlen(input->sequence1);
	int len2=strlen(input->sequence2);
	int numOfOffsets = len1-len2+1;
	double tempWeightsArr[] = {(input->weights)[0], (input->weights)[1], (input->weights)[2], (input->weights)[3]};
	arrangeCharsAccordingToWeights(tempWeightsArr, sortedCharsByWeights);
	printInputToConsole(input);
	conservativeConflict=resolveEmptyGroupsConflict(sortedCharsByWeights);

	printf("OUTPUT:\n");
	
	int s,i;
	int offset = 0;
	int base_offsets_ammount = numOfOffsets/numOfSlaves;
	int offsets_rest = numOfOffsets%numOfSlaves;
	
	int* offsetsArr = (int*)malloc(sizeof(int)*(base_offsets_ammount+1));
	int offsetsArr_size;
	int min_max_flag = (strcmp((input->min_max_str),"MAX") == 0);

	for ( MPI->slave = 1; (MPI->slave) <= numOfSlaves; (MPI->slave)++) {
			offsetsArr_size = base_offsets_ammount;
			for(i=0;i<base_offsets_ammount;i++){
				offsetsArr[i] = offset;
				offset++;
			}
			if(offsets_rest>0){
				offsetsArr[offsetsArr_size] = offset;
				offsetsArr_size++;
				offset ++;
				offsets_rest--;
			}
			
			MPI_Send(&offsetsArr_size,1, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(offsetsArr,offsetsArr_size, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			
			MPI_Send(input->weights,4, MPI_DOUBLE, MPI->slave, MPI->tag, MPI_COMM_WORLD);

			MPI_Send(sortedCharsByWeights,WEIGHTS_ARR_LEN, MPI_CHAR, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(&conservativeConflict,1, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);

			MPI_Send(&len1, 1, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(input->sequence1,len1+1, MPI_CHAR, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(&len2, 1, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(input->sequence2,len2+1, MPI_CHAR, MPI->slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Send(&min_max_flag,1, MPI_INT, MPI->slave, MPI->tag, MPI_COMM_WORLD);
	}

	double slave_score,opt_score = 0;
	int slave,message = SEND_REST,opt_offset = NO_OFFSET;
	char* seq2_optimal_mutant = (char*)malloc((len2+1)*sizeof(char));

	for ( MPI->slave = 1; (MPI->slave) <= numOfSlaves; (MPI->slave)++){

		MPI_Recv(&slave_score, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
		slave = (MPI->status).MPI_SOURCE;

		if((opt_offset==NO_OFFSET) ||
				(min_max_flag == MAX  && slave_score>opt_score) ||
								(min_max_flag == MIN && slave_score<opt_score)){

			opt_score = slave_score;
			message = SEND_REST;
			MPI_Send(&message, 1, MPI_INT, slave, MPI->tag, MPI_COMM_WORLD);
			MPI_Recv(&opt_offset, 1, MPI_INT, slave, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
			MPI_Recv(seq2_optimal_mutant, len2+1, MPI_CHAR, slave, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
		}
		else{
			message = DONT_SEND_REST;
			MPI_Send(&message, 1, MPI_INT, slave, MPI->tag, MPI_COMM_WORLD);
		}

	}

	printf("\nOptimal Mutant: Offset: %d ,Mutant: %s ,Score=%.3f\n",opt_offset,seq2_optimal_mutant,opt_score);
	writeOutputtoTextFile(seq2_optimal_mutant,opt_offset,opt_score,fileName);
	free(seq2_optimal_mutant);
	free(sortedCharsByWeights);


}

void runSlaves(){

	double weightsArr[4];
	char sortedCharsByWeights[4];
	int instances[4];
	int conservativeConflict,u,message;

	int offsetsArr_size;
	MPI_Recv(&offsetsArr_size, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	int* offsetsArr = (int*)malloc(sizeof(int)*(offsetsArr_size));
	MPI_Recv(offsetsArr, offsetsArr_size, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	
	MPI_Recv(weightsArr, 4, MPI_DOUBLE, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	MPI_Recv(sortedCharsByWeights, WEIGHTS_ARR_LEN, MPI_CHAR, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	MPI_Recv(&conservativeConflict, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));

	/* seq 1*/
	int len1, len2;
	MPI_Recv(&len1, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	char* seq1 = (char*)malloc((len1+1)*sizeof(char));
	MPI_Recv(seq1, len1+1, MPI_CHAR, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));

	/*seq2 recv*/
	MPI_Recv(&len2, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &MPI->status);
	char* seq2 = (char*)malloc((len2+1)*sizeof(char));
	MPI_Recv(seq2, len2+1, MPI_CHAR, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));

	int min_max_flag;
	MPI_Recv(&min_max_flag, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	
	char* seq1_fixed;
	char* seq2_mutant;
	char* seq2_optimal_mutant;
	int opt_offset=NO_OFFSET;
	double score,opt_score;
	int i;
	
	for ( i = 0; i < offsetsArr_size; i++){
		
		seq1_fixed = fixSeqByOffset(seq1,offsetsArr[i],len1,len2);
		
		seq2_mutant = (char*)malloc((len2+1)*sizeof(char));
		instances[0] = 0;
		instances[1] = 0;
		instances[2] = 0;
		instances[3] = 0;

		#pragma omp parallel private(u)
		{
		#pragma omp for
			for (u = 0; u < len2; u++) {
				seq2_mutant[u] = getSubstitutedInLetter(seq1_fixed[u], seq2[u],sortedCharsByWeights, instances, conservativeConflict);
			}
		}

		score = computeScore(weightsArr, instances);
		free(seq1_fixed);
		
		if((opt_offset==NO_OFFSET) ||
				(min_max_flag == MAX  && score>opt_score) ||
								(min_max_flag == MIN && score<opt_score)){

			opt_score = score;			
			
			if(opt_offset!=NO_OFFSET)
				free(seq2_optimal_mutant);
			
			opt_offset = offsetsArr[i];
			seq2_optimal_mutant = seq2_mutant;
		}
		else{
			free(seq2_mutant);
		}
		
	}	




	printMutant( MPI->my_rank, opt_offset,seq2_optimal_mutant,opt_score);

	MPI_Send(&opt_score, 1, MPI_DOUBLE, 0, MPI->tag, MPI_COMM_WORLD);

	MPI_Recv(&message, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD, &(MPI->status));
	if(message==SEND_REST){
		MPI_Send(&opt_offset, 1, MPI_INT, 0, MPI->tag, MPI_COMM_WORLD);
		MPI_Send(seq2_optimal_mutant, len2+1, MPI_CHAR, 0, MPI->tag, MPI_COMM_WORLD);
	}
	
	free(seq1);
	free(seq2);
	free(seq2_optimal_mutant);
	free(offsetsArr);
}



/*
 @brief calculate the score as defined by the lecturer
 *
 * @param[in] weightsArr
 * array of doubles representing weights
 * @param[in] instances
 * array of integers which stores the number of
 * instances of each symbol of the mutant
  * @return the calculated score
 * */

double computeScore(double weightsArr[],int instances[]){
	return weightsArr[0]*instances[0]+weightsArr[1]*instances[1]+weightsArr[2]*instances[2]+weightsArr[3]*instances[3];
}


/*
 @brief determines the priority in case of
 * the conservative group of a exl. conservative group of b and
 * the semi-conservative group of b exl. conservative groups b
 * are both empty
* @param[in] sortedCharsByWeights
 * pointer to chars (symbols) array sorted by priorities
 * @return 0 if the colon symbol is prioritized,1 if the dot symbol is prioritized ,-1 on default
*/

int resolveEmptyGroupsConflict(char *sortedCharsByWeights) {

	int i=0;
		while(1){

			if(sortedCharsByWeights[i]==COLON)
					{
						return 0;

					}
			else if(sortedCharsByWeights[i]==DOT)
			{
						return 1;

			}
			else{
				i++;
			}

		}
		return -1;
}

/*
 @brief generates the most suitable char to be substituted in according the
 *sets of rules for substitution given by the lecturer
 *
 * @param[in] a,b
 * char variables representing pair of chars in the sequences
 * @param[in] sortedCharsByWeights
 * pointer to chars (symbols) array sorted by priorities
 * @param[in] instances
 * array of integers which for each offset and mutant combination
 * stores the number of instances of each symbol
 * @param[in] conservativeConflict
 * integer flag which determines the priority in case of
 * the conservative group of a exl. conservative group of b and
 * the semi-conservative group of b exl. conservative groups b
 * are both empty
 * @return char to be substituted in
*/

char getSubstitutedInLetter(char a, char b,char *sortedCharsByWeights,int instances[],int conservativeConflict) {

	char pivot;
	char *consGroup;
	char *semiExcludedGroup=NULL;
	char *conExcludedGroup=NULL;
	char *pch;

	consGroup = getConservativeGroup(b);

	//conExcludedGroup = getConservativeGroupExcluded(a, b);
	getConservativeGroupExcluded(a, b,conExcludedGroup);
	 getSemiConservativeGroupExcluded(b,semiExcludedGroup);


	for (int i = 0; i < WEIGHTS_ARR_LEN; i++) {
		pivot = sortedCharsByWeights[i];

		switch (pivot) {

		case STAR:
			pch = strchr(consGroup, a);
			if (pch == NULL) { // null means NOT FOUND

				#pragma omp critical
				{
				instances[0]++;
				}
				return a;
			} else {
				continue;
			}

		case COLON:
			if (conExcludedGroup != NULL) { // null means NOT FOUND
				#pragma omp critical
				{
				instances[1]++;
				}
				return conExcludedGroup[0];
			}

			else {
				continue;
			}

		case DOT:
			if (semiExcludedGroup != NULL) { // null means NOT FOUND
			#pragma omp critical
			{
				instances[2]++;
			}
				return semiExcludedGroup[0];
			}

			else {
				continue;
			}

		case SPACE:

			if(semiExcludedGroup == NULL &&conExcludedGroup == NULL)
			{
				if(conservativeConflict==0) //suitbale for max
				#pragma omp critical
					{
						instances[1]++;
					}
				else if(conservativeConflict==1)
			#pragma omp critical
				{
					instances[2]++;
				}
				else{
				#pragma omp critical
				{
					instances[3]++;
				}
				}
				return b;
			}
			else {
				continue;
			}

		}

	}

	return ' ';
}

/*
 @brief extracting the relevant part from sequence nr.1 according to given offset
 *
 * @param[in] offset
 * integer representing the requested offset
 * @param[in] seq2Len
 * integer representing the length of sequence nr.2
 * @return a pointer to string which stores fixed sequence nr.1
*/
char* fixSeqByOffset(char* seq1,int offset,int len1,int len2) {
	char *fixedSeq=(char*)malloc((len2+1)*sizeof(char*));
	int i;
	for(i=0;i<len2;i++)
		fixedSeq[i] = seq1[i+offset];
	
	fixedSeq[len2] = '\0';

	return fixedSeq;

}

/*
 @brief rearranging array of symbols according to the suitable weights order
 *
 * @param[out] sortedCharsByWeights
 * pointer to chars (symbols) array
 * @param[in] weights
 * array of doubles representing weights
*/

void arrangeCharsAccordingToWeights(double weights[],char *sortedCharsByWeights) {

	double star, colon, dot;
	star = weights[0];
	colon = weights[1];
	dot = weights[2];

	selectionSort(weights, WEIGHTS_ARR_LEN, MAX);

	for (int i = 0; i < WEIGHTS_ARR_LEN; i++) {

		if (weights[i] == star) {
			sortedCharsByWeights[i] = STAR;
		}

		else if (weights[i] == colon) {
			sortedCharsByWeights[i] = COLON;
		}

		else if (weights[i] == dot) {
			sortedCharsByWeights[i] = DOT;
		}

		else {
			sortedCharsByWeights[i] = SPACE;
		}

	}

}

/*
 @brief extracting the conservative group
 *from a dictionary which defined in the *.h file
 *
 * @param[in] a
 * char variable representing  char in the sequence
 * @return a pointer to string representing the group mentioned above
*/

char* getConservativeGroup(char a) {

	return (char*) conseGroupArr[a - 65];

}

/*
 @brief extracting the semi-conservative group
 *from a dictionary which defined in the *.h file
 *
 * @param[in] a
 * char variable representing  char in the sequence
 * @return a pointer to string representing the group mentioned above
*/

char* getSemiConservativeGroup(char a) {
	return (char*) semiConseGroupArr[a - 65];

}

/*
 @brief generates group that eliminates all elements from semi-conservative
 *group of a that appears on conservative group of a.
 *
 * @param[in] a,b
 * char variables representing pair of chars in the sequences
 * @return a pointer to string representing the group mentioned above
*/

void getSemiConservativeGroupExcluded(char a,char *semiExcludedGroup) { // semi-con-group of b excluding con-group of b

	char *x = getConservativeGroup(a);
	char *y = getSemiConservativeGroup(a);
	char *longerGroup = strlen(x) > strlen(y) ? x : y;
	char *smallerGroup = strlen(x) < strlen(y) ? x : y;
	char *pch;
	char pivot;
	char *excludedGroup = NULL;
	int finalLen = 1;

	for (int i = 0; i < strlen(longerGroup); i++) {
		pivot = longerGroup[i];
		pch = strchr(smallerGroup, pivot);
		if (pch == NULL) { // null means not FOUND
			excludedGroup = (char*) realloc(excludedGroup,
					finalLen * sizeof(char));
			excludedGroup[finalLen - 1] = pivot;
			finalLen++;
		}
	}
	semiExcludedGroup=excludedGroup;
	free(excludedGroup);

}

/*
 @brief generates group that eliminates all elements from conservative
 *group of a that appears on conservative group of b.
 *
 * @param[in] a,b
 * char variables representing pair of chars in the sequences
 * @return a  pointer to string representing the group mentioned above
*/

void getConservativeGroupExcluded(char a, char b,char *conExcludedGroup) {// con-group of a excluding con-group of b

	char *x = getConservativeGroup(a);
	char *y = getConservativeGroup(b);
	char *longerGroup = strlen(x) > strlen(y) ? x : y;
	char *smallerGroup = strlen(x) < strlen(y) ? x : y;
	char *pch;
	char pivot;
	char *excludedGroup = NULL;
	int finalLen = 1;

	for (int i = 0; i < strlen(longerGroup); i++) {
		pivot = longerGroup[i];
		pch = strchr(smallerGroup, pivot);
		if (pch == NULL && pivot != a && pivot != b) { //  null means  NOT FOUND
			excludedGroup = (char*) realloc(excludedGroup,
					finalLen * sizeof(char));
			excludedGroup[finalLen - 1] = pivot;
			finalLen++;
		}

	}
	conExcludedGroup=excludedGroup;
	free(excludedGroup);
}

/*
 @brief performs swap of values between 2 doubles variables.
 *
 * @param[out] xp,yp
 * double pointers to swap values
*/

void swap(double *xp, double *yp) {
	double temp = *xp;
	*xp = *yp;
	*yp = temp;
}

/*
 @brief performs sorting of array.
 *
 * @param[in] arr
 * array of doubles representing weights
 * @param[in] n
 * length of the array arr
 * @param[in] minMaxFlag
 * integer that determines the sort should be performed in ascending/descending order
*/

void selectionSort(double arr[], int n, int minMaxFlag) {
	int i, j, min_idx;

	for (i = 0; i < n - 1; i++) {
		min_idx = i;
		for (j = i + 1; j < n; j++)
			if (minMaxFlag == MIN) {
				if (arr[j] < arr[min_idx])
					min_idx = j;
			} else {
				if (arr[j] > arr[min_idx])
					min_idx = j;
			}

		swap(&arr[min_idx], &arr[i]);
	}
}



