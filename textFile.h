#ifndef TEXTFILE_H_
#define TEXTFILE_H_

#define WEIGHTS_ARR_LEN 4

typedef struct
{

	double weights[4];
	char* sequence1;
	char* sequence2;
	char* min_max_str;
}Input;



void readInputFromTextFile(Input* input,const char* fileName);
void writeOutputtoTextFile(char* seq2_optimal_mutant,int opt_offset,double opt_score,const char* fileName);
void printInputToConsole(Input* input);
void printMutant( int slave_rank,int offset,char* mutant,double score);


#endif /* TEXTFILE_H_ */

