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
	struct individual population[P_SIZE];

	srand(time(NULL));

	//Set initial population
	for(i = 0;i < P_SIZE; i++){
		for(j = 0;j < G_SIZE; j++){
			population[i].gene[j] = rand()%2;
			printf("%d\n", population[i].gene[j]);
		}
		population[i].fitness = 0;
	}

	for(i = 0; i < P_SIZE; i++){
		for(j = 0; j < G_SIZE; j++){
			if(population[i].gene[j]){
				printf("Old: %d\n", population[i].fitness);
				population[i].fitness++;
				printf("New: %d\n\n", population[i].fitness);
			}
		}
	}

}