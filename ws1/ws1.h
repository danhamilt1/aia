#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <pthread.h>

#define GENERATIONS 20000
#define POPULATION_SIZE 500
#define RULE_LENGTH 12
#define NO_RULES 10
#define INDIVIDUAL_LENGTH (RULE_LENGTH*NO_RULES)
#define TRAINING_ROWS 1800
#define TESTING_ROWS 2048
#define T_SIZE 5
#define D_CV_PROB 0.5 // Crossover probability
#define D_MT_PROB 0.5//(double)((1.0/(double)POPULATION_SIZE)*((double)1.0/(double)INDIVIDUAL_LENGTH))/2// Mutation probability
#define MT_MIN (double)((1.0/(double)POPULATION_SIZE)+(1.0/(double)INDIVIDUAL_LENGTH))/2

#define DATA_FILE "data2.txt"
#define OUTPUT_FILE "out.txt"

#define NUM_THREADS 2

struct individual{
  char gene[INDIVIDUAL_LENGTH + 1];
  int fitness;
};

struct childPair{
  struct individual child[2];
  int happened;
};

struct ioData{
  char input[RULE_LENGTH];
  char output;
};

struct threadData{
    struct individual* oldPopulation;
    struct individual* newPopulation;
    int startIndex;
    int stopPoint;
    int fitness;
    int numCv;
    int numMt;
    int CP;
    int MP;
};

long calculatePopulationFitness(
  struct individual *population, int arrSize);
bool probability(float minValue, float maxValue);
int getSeed();
int calculateFitness(struct individual *individual);
struct childPair crossover(struct individual parent1,
            struct individual parent2);
void createNewPopulation(struct individual *oldPopulation, struct individual *newPopulation);
void selectFittest(struct individual *oldPopulation, struct individual *newPopulation);
int tournamentSelection(struct individual *population, int tournamentSize, int populationSize);
int mutateIndividual(struct individual *individual);
void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation);
//int selectBestFromPopulation(struct individual* population);
int getBestIndex(struct individual* population);
int getWorstIndex(struct individual* population);
void readInData();
void selectTrainingData();
int checkHasLearned(struct individual *individual);
void *runThread(void *threadArgs);
double randfrom(double min, double max);

WINDOW * mainwin;

struct ioData *allData;
struct ioData *trainingData;


int tSize = T_SIZE;
int numberOfMutations;

double gCP = 0;
double gMP = 0;
double MT_PROB = D_MT_PROB;
double CV_PROB = D_CV_PROB;
