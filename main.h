
#ifndef MAIN_H_
#define MAIN_H_

#include "textFile.h"
#include "mpi.h"

#define MAX 1
#define MIN 0
#define WEIGHTS_ARR_LEN 4

/* symbols */
#define STAR '*'
#define COLON ':'
#define DOT '.'
#define SPACE ' '

/*message types*/
#define SEND_REST 300
#define DONT_SEND_REST 301
#define NO_OFFSET -1

typedef struct
{
	int  my_rank;
	int  p;
	int tag;
	MPI_Status status;
	int slave;
	char* slave_result;
}MPI_struct;

/* function's declarations */
void arrangeCharsAccordingToWeights(double weights[],char *sortedCharsByWeights);
void swap(double *xp, double *yp);
void selectionSort(double arr[], int n, int minMaxFlag);
char* getConservativeGroup(char a);
char* getSemiConservativeGroup(char a);
void getSemiConservativeGroupExcluded(char a,char *semiExcludedGroup);
char getSubstitutedInLetter(char a, char b,char *sortedCharsByWeights,int instances[],int conservativeConflict);
void getConservativeGroupExcluded(char a, char b,char *conExcludedGroup);
char* fixSeqByOffset(char* seq1,int offset,int len1,int len2);
int resolveEmptyGroupsConflict(char *sortedCharsByWeights);
double computeScore(double weightsArr[],int instances[]);
void runSlaves();
void runMaster(int numOfSlaves,char* fileName);


/*  conservative group dictionary */
const char *conseGroupArr[] = {
		"ST",	  //A
		"",		  //B
		"",		  //C
		"NEQ",	  //D
		"NDQK",	  //E
		"YWMIL",  //F
		"",		  //G
		"QRKNY",  //H
		"MLVF",	  //I
		"",		  //J
		"NEQHR",  //K
		"MIVF",	  //L
		"ILVF",	  //M
		"DEQKH",  //N
		"",		  //O
		"",		  //P
		"NDEKHR", //Q
		"QHK",	  //R
		"TA",	  //S
		"SA",	  //T
		"",	      //U
		"MIL",    //V
		"FY",	  //W
		"",	      //X
		"FWH",    //Y
		"",	      //Z
		};

const char *semiConseGroupArr[] = {
		"SGTVCP",    //A
		"",		     //B
		"SA",		 //C
		"SGNEQHK",	 //D
		"NQHRKD",	 //E
		"HYVLIM",	 //F
		"SAND",		 //G
		"NEQRKDFY",	 //H
		"FLM",	     //I
		"",		     //J
		"STNEQHRKD", //K
		"FVIM",	     //L
		"FVLI",	     //M
		"SGDTKEQHR", //N
		"",		     //O
		"STA",		 //P
		"NEHRKDSQ",	 //Q
		"NEQHK",	 //R
		"AGCNDTPKEQ",//S
		"AVSPNK",	 //T
		"",	         //U
		"ATFLIM",    //V
		"",	         //W
		"",	         //X
		"HF",        //Y
		"",	         //Z
		};


#endif /* MAIN_H_ */



/*  */


/*
 @brief generates the most suitable char to be substituted in according the
 *sets of rules for substitution given by the lecturer
 *
 * @param[out] sortedCharsByWeights
 * pointer to chars array rearranged according to the suitable weights order
 * @param[in] weights
 * array of doubles representing weights
 * @param[in] n
 * length of the array arr
 * @param[in] minMaxFlag
 * integer that determines the sort should be performed in ascending/descending order
 * @return 0 on success, -1 on failure
*/
