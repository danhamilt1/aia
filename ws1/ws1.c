#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define GENERATIONS 50
#define P_SIZE 255
#define G_SIZE 10

struct individual{
	int gene[G_SIZE];
	int fitness;
};

struct childPair{
	int child[2][G_SIZE];
};

int getSeed(){
	int randomData = open("/dev/random", O_RDONLY);
	int randomInt;
	size_t randomDataLen = 0;

	while(randomDataLen < sizeof randomInt){
		ssize_t result = read(randomData, ((char *)&randomInt) + randomDataLen, (sizeof randomInt) - randomDataLen);
		if(result < 0){
			//assert(false);
		}
		randomDataLen += result;
	}
	close(randomData);

	return randomInt;
}

struct childPair makeChildren(struct individual parent1, 
						struct individual parent2){
	struct childPair children;
	int splitPoint = rand()%G_SIZE;
	int i = 0;

	for(i = 0; i < splitPoint; i++){
		children.child[0][i] = parent1.gene[i];
		children.child[1][i] = parent2.gene[i];
	}

	for(i = splitPoint; i < G_SIZE; i++){
		children.child[0][i] = parent2.gene[i];
		children.child[1][i] = parent1.gene[i];
	}

	return children;


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

struct individual createIndividual(int gene[G_SIZE]){
	struct individual newIndividual;

	memcpy(newIndividual.gene, gene, G_SIZE);
	newIndividual.fitness = calculateFitness(gene);

	return newIndividual;
}



int main(void){
	int i = 0;
	int j = 0;

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

	//Select the fittest parents
	for(i = 0; i < P_SIZE; i++){
		int p1 = rand()%P_SIZE;
		int p2 = rand()%P_SIZE;

		if(population[p1].fitness >= population[p2].fitness){
			offspring[i] = population[p1];

		} else {
			offspring[i] = population[p2];

		}
	}

	//Display results
	int avg_initial;
	int avg_new;

	for(i = 0; i < P_SIZE; i++){
		avg_initial = avg_initial + population[i].fitness;
		avg_new = avg_new + offspring[i].fitness;

	}

	// printf("Fitness total\n");
	// printf("Old population total: %d\n", avg_initial);
	// printf("New population total: %d\n", avg_new);

	makeChildren(population[1], population[2]);

	struct individual done = createIndividual(population[1].gene);
	for(i = 0; i < G_SIZE; i++){
		printf("%d", done.gene[i]);
	}
	printf("\n%d\n", done.fitness);

}