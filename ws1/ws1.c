#include "ws1.h"

int main(void) {
	FILE *f_csv;

	// TIDY THIS UP
	int i = 0;
	int j = 0;

	trainingData = malloc(sizeof(struct ioData));

	struct individual *newPopulation = malloc(
			sizeof(struct individual) * POPULATION_SIZE);
	struct individual *population = malloc(sizeof(struct individual) * POPULATION_SIZE);

	readInData();

	f_csv = fopen("history.csv", "w");

	fprintf(f_csv, "BEST FITNESS, MEAN,");

	srand(getSeed());
	//Set-up initial population
	for (i = 0; i < POPULATION_SIZE; ++i) {
		char vals[3] = { '0', '1', '#' };
		for (j = 0; j < INDIVIDUAL_LENGTH+1; ++j) {
			if ((j + 1) % RULE_LENGTH != 0) {
				population[i].gene[j] = vals[rand() % 3];
			} else {
				population[i].gene[j] = vals[rand() % 2];
			}

		}
		population[i].gene[j + 1] = '\0';
		population[i].fitness = calculateFitness(&population[i]);
	}

	memcpy(newPopulation, population, sizeof(struct individual) * POPULATION_SIZE);


  //////////////////////// Generation loop



	for (i = 0; i < GENERATIONS; ++i) {
		int j = 0;

		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);

		//fprintf(f_csv, "\n %d, %d",
		//		newPopulation[getBestIndex(newPopulation)].fitness,
		//		calculatePopulationFitness(newPopulation, POPULATION_SIZE) / POPULATION_SIZE);

		if ((i % 100) == 0) {
			printf("Completed: %2.2f\%\n", ((float)i/(float)GENERATIONS)*100.0);
		}

		printf("Best:  %s, Generation: %d Fitness: %d\n", newPopulation[getBestIndex(newPopulation)].gene, i, newPopulation[getBestIndex(newPopulation)].fitness);
		//printf("Worst: %s, Generation: %d Fitness: %d\n", newPopulation[getWorstIndex(newPopulation)].gene, i, newPopulation[getWorstIndex(newPopulation)].fitness);
		memcpy(population, newPopulation, sizeof(struct individual) * POPULATION_SIZE);

	}

  ////////////////////////

	fclose(f_csv);

	printf("fittest individual: ");

	for (i = 0; i < RULE_LENGTH * NO_RULES; i++) {
		printf("%c", newPopulation[getBestIndex(newPopulation)].gene[i]);
	}

	printf("\n");

	printf("fitness: %d\n", newPopulation[getBestIndex(newPopulation)].fitness);

	checkHasLearned(&newPopulation[getBestIndex(newPopulation)]);

	free(trainingData);
	free(population);
	free(newPopulation);
}

long calculatePopulationFitness(struct individual *population, int arrSize) {
	int i = 0;
	long totalFitness = 0;

	for (i = 0; i < arrSize; ++i) {
		totalFitness = totalFitness + population[i].fitness;
	}
	return totalFitness;
}

bool probability(float minValue, float maxValue) {
	bool retVal = false;
	int randomNumber = rand() % (int) PROB_ACC;
	// Check if value lands between bounds
	if ((randomNumber >= minValue) && (randomNumber <= maxValue)) {
		retVal = true;
	}

	return retVal;
}

int getSeed() {
	int randomData = open("/dev/random", O_RDONLY);
	int randomInt;
	size_t randomDataLen = 0;

	while (randomDataLen < sizeof randomInt) {
		ssize_t result = read(randomData, ((char *) &randomInt) + randomDataLen,
				(sizeof randomInt) - randomDataLen);
		if (result < 0) {
			printf("Could not read from /dev/random\n");
		}
		randomDataLen += result;
	}
	close(randomData);

	return randomInt;
}

int calculateFitness(struct individual *individual) {
	int i = 0;
	int j = 0;
	int fitness = 0;
	int score = 0;

	for (i = 0; i < TRAINING_ROWS; ++i) {
		int k = 0;
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			score = 0;
			for (int k = 0; k < RULE_LENGTH; k++) {
				if ((j + 1) % RULE_LENGTH != 0) {
					if (individual->gene[j] != '#') {
						if (individual->gene[j] == trainingData[i].input[j]) {
							++score;
						}
					} else {
						++score;
					}
				}
				++j;
			}

			if (score == RULE_LENGTH-1) {
				if (individual->gene[j] == trainingData[i].output) {
					fitness++;
					break;
				}
			}
		}
	}

	return fitness;

}

struct childPair crossover(struct individual parent1, struct individual parent2) {
	struct childPair children;
	int splitPoint = rand() % INDIVIDUAL_LENGTH+1;
	int i = 0;

	if (probability(0, CV_PROB)) {
		for (i = 0; i < splitPoint; ++i) {
			children.child[0].gene[i] = parent1.gene[i];
			children.child[1].gene[i] = parent2.gene[i];
		}

		for (i = splitPoint; i < INDIVIDUAL_LENGTH; ++i) {
			children.child[0].gene[i] = parent2.gene[i];
			children.child[1].gene[i] = parent1.gene[i];
		}

	} else {
		children.child[0] = parent1;
		children.child[1] = parent2;
	}
	children.child[0].gene[RULE_LENGTH*NO_RULES] = '\0';
	children.child[1].gene[RULE_LENGTH*NO_RULES] = '\0';
	//children.child[0].fitness = calculateFitness(children.child[0]);
	//children.child[1].fitness = calculateFitness(children.child[1]);

	return children;

}

void createNewPopulation(struct individual *oldPopulation,
		struct individual *newPopulation) {
	struct childPair temp;
	int i = 0;
	for (i = 0; i < POPULATION_SIZE; ++i) {

		//Carry out 2 tournaments to select 2 parents for mating
		int p1 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);
		int p2 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);

		temp = crossover(oldPopulation[p1], oldPopulation[p2]);

		mutateIndividual(&temp.child[0]);
		newPopulation[i] = temp.child[0];
		newPopulation[i].fitness = calculateFitness(&newPopulation[i]);

		++i;

		if (i != POPULATION_SIZE) {
			mutateIndividual(&temp.child[1]);
			newPopulation[i] = temp.child[1];
			newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
		}
	}
}

void selectFittest(struct individual *oldPopulation,
		struct individual *newPopulation) {
	int i = 0;
	//Select the fittest parents
	for (i = 0; i < POPULATION_SIZE; ++i) {
		int p1 = rand() % POPULATION_SIZE; // First parent
		int p2 = rand() % POPULATION_SIZE; // Second parent

		if (oldPopulation[p1].fitness >= oldPopulation[p2].fitness) {
			newPopulation[i] = oldPopulation[p1];

		} else {
			newPopulation[i] = oldPopulation[p2];

		}
	}
}

int tournamentSelection(struct individual *population, int tournamentSize,
		int populationSize) {
	int best = NULL;
	int challengerIndex = 0;
	int i = 0;

	// Select tournament pool for this iteration
	for (i = 0; i < tournamentSize; ++i) {
		challengerIndex = rand() % populationSize;
		if ((best == NULL)
				|| (population[challengerIndex].fitness
						>= population[best].fitness)) {
			best = challengerIndex;
		}
	}

	//printf("Best from tournament: %d Fitness: %d\n", best, population[best].fitness);
	return best;
}

void mutateIndividual(struct individual *individual) {
	int vals[3] = { '0', '1', '#' };
	for (int i = 0; i < INDIVIDUAL_LENGTH; ++i) {
		if (probability(0, MT_PROB)) {
			if ((i+1) % (RULE_LENGTH) != 0) {
			    //printf("i: %d mod: %d\n", i, (i+1)%RULE_LENGTH);
				individual->gene[i] = vals[rand() % 3];
			} else {
				individual->gene[i] = vals[rand() % 2];
			}
		}
	}

}

struct individual createIndividual(int gene[RULE_LENGTH * NO_RULES]) {
	struct individual newIndividual;
	int i = 0;

	for (i = 0; i < INDIVIDUAL_LENGTH+1; ++i) {
		printf("%d", gene[i]);
	}
	printf("\n");

	memcpy(newIndividual.gene, gene, sizeof(*gene) * RULE_LENGTH * NO_RULES);
	newIndividual.fitness = calculateFitness(&newIndividual);
	printf("\n");
	return newIndividual;
}

void selectBestFromPreviousPopulation(struct individual* newPopulation,
		struct individual* oldPopulation) {
	int bestOld = getBestIndex(oldPopulation);
	int bestNew = getBestIndex(newPopulation);
	int i = 0;

	if (oldPopulation[bestOld].fitness > newPopulation[bestNew].fitness) {
		int worstIndex = getWorstIndex(newPopulation);
		memcpy(&newPopulation[worstIndex], &oldPopulation[bestOld],
				sizeof(oldPopulation[bestOld]));
	}
}

int getBestIndex(struct individual* population) {
	int best = NULL;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((best == NULL)
				|| (population[i].fitness > population[best].fitness)) {
			best = i;
		}
	}

	return best;
}

int getWorstIndex(struct individual* population) {
	int worst = NULL;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((worst == NULL)
				|| (population[i].fitness <= population[worst].fitness)) {
			worst = i;
		}
	}

	return worst;
}

void readInData() {
	FILE *f_data;

	char input_test[2048];
	char *record, *line;

	f_data = fopen(DATA_FILE, "r");

	int i = 0;

	while ((line = fgets(input_test, 2048, f_data)) != NULL) {
		record = strtok(line, " ");

		for (int j = 0; j < RULE_LENGTH - 1; j++) {
			trainingData[i].input[j] = (record[j]);
		}

		record = strtok(NULL, " ");
		trainingData[i].output = *record;

		++i;
		trainingData = realloc(trainingData, sizeof(struct ioData) * (i + 1));
	}
	fclose(f_data);
}

void checkHasLearned(struct individual *individual) {
	int i = 0;
	int j = 0;
	int score = 0;
	int outputIndex = 0;
	int yays = 0;

	for (i = 0; i < TESTING_ROWS; ++i) {
		int k = 0;
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			score = 0;
			for (int k = 0; k < RULE_LENGTH; k++) {
				if ((j + 1) % RULE_LENGTH != 0) {
					if (individual->gene[j] != '#') {
						if (individual->gene[j] == trainingData[i].input[j]) {
							++score;
						}
					} else {
						++score;
					}
				}
				++j;
			}

			if (score == RULE_LENGTH-1) {
				if (individual->gene[j] == trainingData[i].output) {
					yays++;
					break;
				}
			}
		}
	}

	printf("%d", yays);
}
