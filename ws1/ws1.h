#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define GENERATIONS 500
#define P_SIZE 100
#define G_SIZE 7
#define NO_RULES 2
#define TRAINING_ROWS 64
#define TESTING_ROWS 64
#define T_SIZE 2
#define PROB_ACC 1000
#define CV_PROB 600 // Crossover probability
#define MT_PROB 100//(1/P_SIZE + 1/G_SIZE*NO_RULES)/2 // Mutation probability

#define DATA_FILE "/home/daniel/Desktop/github/aia.git/ws1/data1.txt"

struct individual{
  char gene[((G_SIZE)*NO_RULES) + 1];
  int fitness;
};

struct childPair{
  struct individual child[2];
};

struct ioData{
  char input[G_SIZE];
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
struct individual createIndividual(int gene[G_SIZE]);
void selectFittest(struct individual *oldPopulation, struct individual *newPopulation);
int tournamentSelection(struct individual *population, int tournamentSize, int populationSize);
void mutateIndividual(struct individual *individual);
void mutateOutput(struct individual *individual);
void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation);
//int selectBestFromPopulation(struct individual* population);
int getBestIndex(struct individual* population);
int getWorstIndex(struct individual* population);
void readInData();
void checkHasLearned(struct individual *individual);

struct ioData *data_test;
