#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define GENERATIONS 200
#define P_SIZE 50
#define G_SIZE 64
#define T_SIZE 10
#define PROB_ACC 1000
#define CV_PROB 600 // Crossover probability
#define MT_PROB (1/P_SIZE + 1/G_SIZE)/2 // Mutation probability

#define DATA_FILE "data1.txt"

struct individual{
	int gene[G_SIZE];
	int fitness;
};

struct childPair{
	struct individual child[2];
};

struct ioData{
	char input[12];
	int output;
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
void mutateIndividual(struct individual *individual);
void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation);
//int selectBestFromPopulation(struct individual* population);
int getBestIndex(struct individual* population);
int getWorstIndex(struct individual* population);
void readInData();

struct ioData data_test[G_SIZE];

int main(void){
	FILE *f_csv;

	// TIDY THIS UP
	int i = 0;
	int j = 0;

	struct individual newPopulation[P_SIZE];
	struct individual offspring[P_SIZE];
	struct individual population[P_SIZE];

	readInData();

	f_csv = fopen("history.csv", "w");

	fprintf(f_csv, "BEST FITNESS, MEAN,");

	srand(getSeed());

	//Set-up initial population
	for(i = 0;i < P_SIZE; ++i){
		for(j = 0;j < G_SIZE; ++j){
			population[i].gene[j] = rand()%2;

		}
		population[i].fitness = 0;

	}

	//Initialise the fitness of all new individuals in the population
	for(i = 0; i < P_SIZE; ++i){
		for(j = 0; j < G_SIZE; ++j){
			if(population[i].gene[j]){
				++population[i].fitness;
			}
		}
	}
	memcpy(newPopulation, population, sizeof(struct individual)*P_SIZE);
	printf("Initial population: %d\n",calculatePopulationFitness(newPopulation, sizeof(population)/sizeof(newPopulation[0])));


	for(i = 0; i < GENERATIONS; ++i){
		int j = 0;

		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);


		fprintf(f_csv, "\n %d, %d", newPopulation[getBestIndex(newPopulation)].fitness,
						calculatePopulationFitness(&newPopulation,P_SIZE)/P_SIZE);


		if((i % 50) == 0){
			printf("Completed: %2.2f\%\n", ((float)i/(float)GENERATIONS)*100.0);
		}

		memcpy(population, newPopulation, sizeof(struct individual)*P_SIZE);
	}

	fclose(f_csv);

	printf("\nAfter %d generations: %d\n", (int)GENERATIONS, calculatePopulationFitness(&newPopulation,
						sizeof(newPopulation)/sizeof(struct individual)));

	printf("data_file:          ");
	for(i = 0; i < G_SIZE; i++){
		printf("%d", data_test[i].output);
	}
	printf("\n");

	printf("fittest individual: ");
	for(i = 0; i < G_SIZE; i++){
		printf("%d", newPopulation[getBestIndex(&newPopulation)].gene[i]);
	}
	printf("\n");

	printf("fitness: %d\n", newPopulation[getBestIndex(&newPopulation)].fitness);


}

int calculatePopulationFitness(
	struct individual *population, int arrSize){
	int i = 0;
	int totalFitness = 0;

	for(i = 0; i < arrSize; ++i){
		totalFitness += population[i].fitness;
		//printf("%d\n", population[i].fitness);
	}


	return totalFitness;
}

bool probability(float minValue, float maxValue){
	bool retVal = false;
	int randomNumber = rand()%(int)PROB_ACC;
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
		if(gene[i] == data_test[i].output){
			fitness++;
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

			//Carry out 2 tournaments to select 2 parents for mating
			int p1 = tournamentSelection(oldPopulation, T_SIZE, P_SIZE);
			int p2 = tournamentSelection(oldPopulation, T_SIZE, P_SIZE);

			temp = crossover(oldPopulation[p1],oldPopulation[p2]);

			mutateIndividual(&temp.child[0]);
			newPopulation[i] = temp.child[0];
			//printf("\ncopy:   ");
			//for(int j = 0; j < G_SIZE; ++j){
			//	printf("%d",newPopulation[i].gene[j]);
			//}
			++i;
			//printf("2: %d\n", i);
			if(i!=P_SIZE){
				mutateIndividual(&temp.child[1]);
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

void mutateIndividual(struct individual *individual){
	int c_length = sizeof(individual->gene)/sizeof(individual->gene[0]);
	for(int i = 0; i < c_length; ++i){
		if(probability(0, MT_PROB)){
			int randomIndex = rand()%c_length;
			// Flip bit
			individual->gene[randomIndex] = 1 - individual->gene[randomIndex];
			//printf("Mutation happened!\n");
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

	//for(i = 0; i < G_SIZE; ++i){
	//	printf("%d",newIndividual.gene[i]);
	//}
	printf("\n");
	return newIndividual;
}

void selectBestFromPreviousPopulation(struct individual* newPopulation, struct individual* oldPopulation){
		int bestOld = getBestIndex(oldPopulation);
		int bestNew = getBestIndex(newPopulation);
		int i = 0;

		if(oldPopulation[bestOld].fitness > newPopulation[bestNew].fitness){
			int worstIndex = getWorstIndex(newPopulation);
				/*for(i = 0; i < G_SIZE; ++i){
					printf("%d",oldPopulation[bestOld].gene[i]);
				}
				printf("\n");
				for(i = 0; i < G_SIZE; ++i){
					printf("%d",newPopulation[worstIndex].gene[i]);
				}
				printf("\n");*/
			memcpy(&newPopulation[worstIndex], &oldPopulation[bestOld], sizeof(oldPopulation[bestOld]));
				/*for(i = 0; i < G_SIZE; ++i){
					printf("%d",newPopulation[worstIndex].gene[i]);
				}
				printf("\n");*/

		}
}

int getBestIndex(struct individual* population){
	int best = NULL;
	int i = 0;

	for(i = 0; i < P_SIZE; ++i){
		if((best == NULL) || (population[i].fitness >= population[best].fitness)){
			best = i;
		}
	}

	return best;
}

int getWorstIndex(struct individual* population){
	int worst = NULL;
	int i = 0;

	for(i = 0; i < P_SIZE; ++i){
		if((worst == NULL) || (population[i].fitness <= population[worst].fitness)){
			//printf("Worse fitness: %d\n", population[i].fitness);
			worst = i;
		}
	}

	return worst;
}

void readInData(){
	FILE *f_data;

	char input_test[15];
	char *record, *line;

	f_data = fopen(DATA_FILE, "r");

	int i = 0;
	while((line = fgets(input_test, 15, f_data)) != NULL){
		record = strtok(line, " ");
		strcpy(data_test[i].input, record);
		//printf("A: %s", data_test[i].input);

		record = strtok(NULL, " ");
		data_test[i].output = atoi(record);
		//printf(" B: %d\n", data_test[i].output);

		++i;
	}

	fclose(f_data);
}
