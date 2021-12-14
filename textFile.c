#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "textFile.h"


void readInputFromTextFile(Input* input,const char* fileName){

	char temp[10000];
	double w1;
	double w2;
	double w3;
	double w4;

	FILE* fp = fopen(fileName,"r");
	if(!fp){
		printf("error: can't open input file");
		return;
	}
	if(fscanf(fp,"%lf %lf %lf %lf",&w1,&w2,&w3,&w4)!=4)
		return;

	input->weights[0]=w1;
	input->weights[1]=-w2;
	input->weights[2]=-w3;
	input->weights[3]=-w4;

	if(fscanf(fp,"%s",temp)!=1)
			return;
	input->sequence1=strdup(temp);

	if(fscanf(fp,"%s",temp)!=1)
				return;
	input->sequence2=strdup(temp);

	if(fscanf(fp,"%s",temp)!=1)
				return;
	input->min_max_str=strdup(temp);
	fclose(fp);


}

void writeOutputtoTextFile(char* seq2_optimal_mutant,int opt_offset,double opt_score,const char* fileName){

	FILE* fp = fopen(fileName,"w");
		if(!fp)
			return;

	fprintf(fp,"%s\n%d\n%f\n",seq2_optimal_mutant,opt_offset,opt_score);


		fclose(fp);
}




void printInputToConsole(Input* input){
	printf("\n");
	printf("INPUT:\n");
	printf("SEQ1: %s\n", input->sequence1);
	printf("SEQ2: %s\n", input->sequence2);

	printf("Input Weights array:");
	for (int i = 0; i < WEIGHTS_ARR_LEN; i++) {
		printf("%f ", input->weights[i]);
	}
	printf("\n");
	printf("MIN/MAX: %s\n", input->min_max_str);
	printf("\n");


}


/*
 @brief prints the result (aka. mutant) to a given offsets and its score
 *
 * @param[in] offset
 * integer representing the requested offset
 * @param[in] mutant
 * the result (aka. mutant)
 * @param[in] minMaxFlag
 * double that in which the score is stored
 * */

void printMutant( int slave_rank,int offset,char* mutant,double score){

	printf("My rank: %d , Optimal Mutant For Offset: %d  - %s , Score:%f\n",slave_rank,offset,mutant,score);



}

