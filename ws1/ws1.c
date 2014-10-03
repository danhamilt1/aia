#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define P_SIZE 255
#define G_SIZE 10

struct individual{
	int gene[G_SIZE];
	int fitness;
};

int main(void){
	int i = 0;
	int j = 0;
	struct individual offspring[P_SIZE];
	struct individual population[P_SIZE];

	srand(time(NULL));

	//Set initial population
	for(i = 0;i < P_SIZE; i++){
		for(j = 0;j < G_SIZE; j++){
			population[i].gene[j] = rand()%2;
			//printf("%d\n", population[i].gene[j]);
		}
		population[i].fitness = 0;
	}

	//Set fitness of an individual based on 1's in the array
	for(i = 0; i < P_SIZE; i++){
		for(j = 0; j < G_SIZE; j++){
			if(population[i].gene[j]){
				//("Old: %d\n", population[i].fitness);
				population[i].fitness++;
				//printf("New: %d\n\n", population[i].fitness);
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

	int avg_initial;
	int avg_new;

	for(i = 0; i < P_SIZE; i++){
		avg_initial = avg_initial + population[i].fitness;
		avg_new = avg_new + offspring[i].fitness;
	}

	printf("Fitness total\n");
	printf("Old population total: %d\n", avg_initial);
	printf("New population total: %d\n", avg_new);

}