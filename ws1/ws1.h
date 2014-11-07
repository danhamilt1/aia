#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define GENERATIONS 500
#define POPULATION_SIZE 50
#define RULE_LENGTH 12
#define NO_RULES 10
#define INDIVIDUAL_LENGTH (RULE_LENGTH*NO_RULES)
#define TRAINING_ROWS 1024
#define TESTING_ROWS 2048
#define T_SIZE 2
#define PROB_ACC 1000
#define CV_PROB 700 // Crossover probability
#define MT_PROB (1/POPULATION_SIZE + 1/INDIVIDUAL_LENGTH)/2 // Mutation probability

#define DATA_FILE "data2.txt"

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
struct individual createIndividual(int gene[RULE_LENGTH]);
void selectFittest(struct individual *oldPopulation, struct individual *newPopulation);
int tournamentSelection(struct individual *population, int tournamentSize, int populationSize);
void mutateIndividual(struct individual *individual);
void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation);
//int selectBestFromPopulation(struct individual* population);
int getBestIndex(struct individual* population);
int getWorstIndex(struct individual* population);
void readInData();
void checkHasLearned(struct individual *individual);

struct ioData *trainingData;
