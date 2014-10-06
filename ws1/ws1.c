#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define GENERATIONS 100000
#define P_SIZE 500
#define G_SIZE 10
#define CV_PROB 10 // Crossover probability
#define MT_PROB 10 // Mutation probability
#define BIAS 0.9

struct individual{
	int gene[G_SIZE];
	int fitness;
};

struct childPair{
	struct individual child[2];
};

int calculatePopulationFitness(
	struct individual *population, int arrSize);
bool probability(float minValue, float maxValue);
int getSeed();
int calculateFitness(int gene[G_SIZE]);
struct childPair makeChildren(struct individual parent1, 
						struct individual parent2);
void createNewPopulation(struct individual *oldPopulation, struct individual *newPopulation);
struct individual createIndividual(int gene[G_SIZE]);
void selectFittest(struct individual *oldPopulation, struct individual *newPopulation);


int main(void){
	int i = 0;
	int j = 0;

	struct individual newPopulation[P_SIZE];
	struct individual offspring[P_SIZE];
	struct individual population[P_SIZE];

	srand(getSeed());

	//Set initial population
	for(i = 0;i < P_SIZE; i++){
		for(j = 0;j < G_SIZE; j++){
			population[i].gene[j] = rand()%2;

		}
		population[i].fitness = 0;

	}

	//Set fitness of an individual based on 1's in the array
	for(i = 0; i < P_SIZE; i++){
		for(j = 0; j < G_SIZE; j++){
			if(population[i].gene[j]){
				population[i].fitness++;

			}
		}
	}

	selectFittest(population, offspring);

	for(i = 0; i < GENERATIONS; i++){
		int j = 0;
		//Switch around to make more semantic sense
		memcpy(newPopulation, offspring, sizeof(struct individual)*P_SIZE);
		createNewPopulation(newPopulation, offspring);
		selectFittest(newPopulation, offspring);
	}

	printf("newPop: %d\n", calculatePopulationFitness(&newPopulation, 
						sizeof(newPopulation)/sizeof(struct individual)));

}

int calculatePopulationFitness(
	struct individual *population, int arrSize){
	int i = 0;
	int totalFitness = 0;

	for(i = 0; i < arrSize; i++){
		totalFitness += population[i].fitness;
	}


	return totalFitness;
}

bool probability(float minValue, float maxValue){
	float randomNumber = rand()%P_SIZE;
	bool retVal = false;
	randomNumber = (1.0/randomNumber) * BIAS;

	// Check if value lands between bounds
	if((randomNumber > minValue) && (randomNumber < maxValue)){
		retVal = true;
	}

	return retVal;
}

int getSeed(){
	int randomData = open("/dev/random", O_RDONLY);
	int randomInt;
	size_t randomDataLen = 0;

	while(randomDataLen < sizeof randomInt){
		ssize_t result = read(randomData, ((char *)&randomInt) + randomDataLen, (sizeof randomInt) - randomDataLen);
		if(result < 0){
			printf("Could not read from /dev/random\n");
		}
		randomDataLen += result;
	}
	close(randomData);

	return randomInt;
}

int calculateFitness(int gene[G_SIZE]){
	int i = 0;
	int fitness = 0;

	for(i = 0; i < G_SIZE; i++){
		if(gene[i]){
			fitness++;
		}
	}

	return fitness;
}

struct childPair makeChildren(struct individual parent1, 
						struct individual parent2){
	struct childPair children;
	int splitPoint = rand()%G_SIZE;
	int i = 0;

	if(probability(1.0/P_SIZE, 1.0/G_SIZE)){
		for(i = 0; i < splitPoint; i++){
			children.child[0].gene[i] = parent1.gene[i];
			children.child[1].gene[i] = parent2.gene[i];
		}

		for(i = splitPoint; i < G_SIZE; i++){
			children.child[0].gene[i] = parent2.gene[i];
			children.child[1].gene[i] = parent1.gene[i];
		}
		//("Cross occurred\n");
	} else {
		children.child[0] = parent1;
		children.child[1] = parent2;
		//printf("No Cross\n");
	}

	children.child[0].fitness = calculateFitness(children.child[0].gene);
	children.child[1].fitness = calculateFitness(children.child[1].gene);

	return children;

}

void createNewPopulation(struct individual *oldPopulation, struct individual *newPopulation){
		struct childPair temp;
		int i = 0;
		for(i = 0; i < P_SIZE; i++){
			int p1 = rand()%P_SIZE;
			int p2 = rand()%P_SIZE;

			temp = makeChildren(oldPopulation[p1],oldPopulation[p2]);
			newPopulation[i] = temp.child[0];
			i++;
			if(i!=P_SIZE){
				newPopulation[i] = temp.child[1];
			}
		}
}

void selectFittest(struct individual *oldPopulation, struct individual *newPopulation){
	int i = 0;
	//Select the fittest parents
	for(i = 0; i < P_SIZE; i++){
		int p1 = rand()%P_SIZE; // First parent
		int p2 = rand()%P_SIZE; // Second parent

		if(oldPopulation[p1].fitness >= oldPopulation[p2].fitness){
			newPopulation[i] = oldPopulation[p1];

		} else {
			newPopulation[i] = oldPopulation[p2];

		}
	}
}


struct individual createIndividual(int gene[G_SIZE]){
	struct individual newIndividual;
	int i = 0;

	for(i = 0; i < G_SIZE; i++){
		printf("%d",gene[i]);
	}
	printf("\n");

	memcpy(newIndividual.gene, gene, sizeof(*gene)*10);
	newIndividual.fitness = calculateFitness(gene);

	for(i = 0; i < G_SIZE; i++){
		printf("%d",newIndividual.gene[i]);
	}
	printf("\n");
	return newIndividual;
}

