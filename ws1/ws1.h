#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

#define GENERATIONS 50
#define POPULATION_SIZE 1000
#define RULE_LENGTH 12
#define NO_RULES 20
#define INDIVIDUAL_LENGTH (RULE_LENGTH*NO_RULES)
#define TRAINING_ROWS 2000
#define TESTING_ROWS 2048
#define T_SIZE 10
#define PROB_ACC 10000
#define CV_PROB 0//6000 // Crossover probability
#define MT_PROB 90//(1/POPULATION_SIZE + 1/INDIVIDUAL_LENGTH)/2 // Mutation probability

#define DATA_FILE "data2.txt"
#define OUTPUT_FILE "out.txt"

struct individual{
  char gene[INDIVIDUAL_LENGTH + 1];
  int fitness;
};

struct childPair{
  struct individual child[2];
};

struct ioData{
  char input[RULE_LENGTH];
  char output;
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
void mutateIndividual(struct individual *individual);
void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation);
//int selectBestFromPopulation(struct individual* population);
int getBestIndex(struct individual* population);
int getWorstIndex(struct individual* population);
void readInData();
void selectTrainingData();
int checkHasLearned(struct individual *individual);

WINDOW * mainwin;

struct ioData *allData;
struct ioData *trainingData;
