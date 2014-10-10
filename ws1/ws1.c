#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define GENERATIONS 1000
#define P_SIZE 5000
#define G_SIZE 100
#define T_SIZE 50
#define CV_PROB 70 // Crossover probability
#define MT_PROB 1 // Mutation probability
//#define BIAS 1.0

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
struct childPair crossover(struct individual parent1,
						struct individual parent2);
void createNewPopulation(struct individual *oldPopulation, struct individual *newPopulation);
struct individual createIndividual(int gene[G_SIZE]);
void selectFittest(struct individual *oldPopulation, struct individual *newPopulation);
int tournamentSelection(struct individual *population, int tournamentSize, int populationSize);
void mutateIndividual(struct individual individual);


int main(void){
	FILE *csv;

	int i = 0;
	int j = 0;

	struct individual newPopulation[P_SIZE];
	struct individual offspring[P_SIZE];
	struct individual population[P_SIZE];

	csv = fopen("history.csv", "w");
	fprintf(csv, "TOTAL FITNESS, MEAN,");

	srand(getSeed());

	//Set initial population
	for(i = 0;i < P_SIZE; ++i){
		for(j = 0;j < G_SIZE; ++j){
			population[i].gene[j] = rand()%2;

		}
		population[i].fitness = 0;

	}

	//Set fitness of an individual based on 1's in the array
	for(i = 0; i < P_SIZE; ++i){
		for(j = 0; j < G_SIZE; ++j){
			if(population[i].gene[j]){
				population[i].fitness++;
			}
		}
	}
	printf("Pre selection: %d\n",calculatePopulationFitness(population, sizeof(population)/sizeof(population[0])));
	//selectFittest(population, offspring);
	printf("Post selection: %d\n",calculatePopulationFitness(population, sizeof(population)/sizeof(population[0])));

	for(i = 0; i < GENERATIONS; i++){
		int j = 0;
		//Switch around to make more semantic sense
		memcpy(newPopulation, population, sizeof(struct individual)*P_SIZE);
		createNewPopulation(newPopulation, population);
		//selectFittest(newPopulation, population);

		fprintf(csv, "\n %d, %d", calculatePopulationFitness(&newPopulation,
						sizeof(newPopulation)/sizeof(struct individual)),
						calculatePopulationFitness(&newPopulation,
						sizeof(newPopulation)/sizeof(struct individual))/P_SIZE);
		if((i % 50) == 0){
			printf("Completed: %2.2f\%\n", ((float)i/(float)GENERATIONS)*100.0);
		}
	}

	fclose(csv);

	printf("After %d generations: %d\n", (int)GENERATIONS, calculatePopulationFitness(&newPopulation,
						sizeof(newPopulation)/sizeof(struct individual)));

}

int calculatePopulationFitness(
	struct individual *population, int arrSize){
	int i = 0;
	int totalFitness = 0;

	for(i = 0; i < arrSize; ++i){
		totalFitness += population[i].fitness;
	}


	return totalFitness;
}

bool probability(float minValue, float maxValue){
	bool retVal = false;
	int randomNumber = rand()%(int)100;
	// Check if value lands between bounds
	if((randomNumber >= minValue) && (randomNumber <= maxValue)){
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

	for(i = 0; i < G_SIZE; ++i){
		if(gene[i]){
			++fitness;
		}
	}

	return fitness;
}

struct childPair crossover(struct individual parent1,
						struct individual parent2){
	struct childPair children;
	int splitPoint = rand()%G_SIZE;
	int i = 0;

	if(probability(0, CV_PROB)){
		for(i = 0; i < splitPoint; ++i){
			children.child[0].gene[i] = parent1.gene[i];
			children.child[1].gene[i] = parent2.gene[i];
		}

		for(i = splitPoint; i < G_SIZE; ++i){
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
		for(i = 0; i < P_SIZE; ++i){

			int p1 = tournamentSelection(oldPopulation, T_SIZE, P_SIZE);
			int p2 = tournamentSelection(oldPopulation, T_SIZE, P_SIZE);

			temp = crossover(oldPopulation[p1],oldPopulation[p2]);
			mutateIndividual(temp.child[0]);
			newPopulation[i] = temp.child[0];
			++i;
			//printf("2: %d\n", i);
			if(i!=P_SIZE){
				mutateIndividual(temp.child[1]);
				newPopulation[i] = temp.child[1];
			}
		}
}

void selectFittest(struct individual *oldPopulation, struct individual *newPopulation){
	int i = 0;
	//Select the fittest parents
	for(i = 0; i < P_SIZE; ++i){
		int p1 = rand()%P_SIZE; // First parent
		int p2 = rand()%P_SIZE; // Second parent

		if(oldPopulation[p1].fitness >= oldPopulation[p2].fitness){
			newPopulation[i] = oldPopulation[p1];

		} else {
			newPopulation[i] = oldPopulation[p2];

		}
	}
}

int tournamentSelection(struct individual *population, int tournamentSize, int populationSize){
	int best = NULL;
	int challengerIndex = 0;
	int i = 0;

	// Select tournament pool for this iteration
	for(i = 0; i < tournamentSize; ++i){
		challengerIndex = rand()%populationSize;
		if((best == NULL) || (population[challengerIndex].fitness >= population[best].fitness)){
			best = challengerIndex;
		}
	}

	//printf("Best from tournament: %d Fitness: %d\n", best, population[best].fitness);
	return best;
}

void mutateIndividual(struct individual individual){
	int c_length = sizeof(individual.gene)/sizeof(individual.gene[0]);
	//printf("\nMutations: ");
	for(int i = 0; i < c_length; i++){
		if(probability(0, MT_PROB)){
		//printf("%d ",i);
			int randomIndex = rand()%c_length;
			individual.gene[randomIndex] = 1 - individual.gene[randomIndex];
		}
	}

}

struct individual createIndividual(int gene[G_SIZE]){
	struct individual newIndividual;
	int i = 0;

	for(i = 0; i < G_SIZE; ++i){
		printf("%d",gene[i]);
	}
	printf("\n");

	memcpy(newIndividual.gene, gene, sizeof(*gene)*10);
	newIndividual.fitness = calculateFitness(gene);

	for(i = 0; i < G_SIZE; ++i){
		printf("%d",newIndividual.gene[i]);
	}
	printf("\n");
	return newIndividual;
}

