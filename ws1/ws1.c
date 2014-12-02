#include "ws1.h"

int main(void) {
	FILE *f_csv;
	FILE *out;

	// TIDY THIS UP
	int i = 0;
	int j = 0;

	trainingData = malloc(sizeof(struct ioData) * TRAINING_ROWS);

	struct individual *newPopulation = malloc(
			sizeof(struct individual) * POPULATION_SIZE);
	struct individual *population = malloc(
			sizeof(struct individual) * POPULATION_SIZE);

	if ((mainwin = initscr()) == NULL) {
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(EXIT_FAILURE);
	}

	srand(getSeed());

	readInData();
	selectTrainingData();

	f_csv = fopen("history.csv", "w");

	fprintf(f_csv, "GENERATION, BEST FITNESS, MEAN,");

	refresh();
	//Set-up initial population
	for (i = 0; i < POPULATION_SIZE; ++i) {
		char vals[3] = { '0', '1', '#' };
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			if ((j + 1) % RULE_LENGTH != 0) {
				population[i].gene[j] = vals[rand() % 3];
			} else {
				population[i].gene[j] = vals[rand() % 2];
			}

		}
		population[i].gene[INDIVIDUAL_LENGTH] = '\0';
		population[i].fitness = calculateFitness(&population[i]);
	}

	memcpy(newPopulation, population,
			sizeof(struct individual) * POPULATION_SIZE);

	//////////////////////// Generation loop

	// NCurses coords
	int x = 1;
	int y = 1;
	int bestInPopulation = 0;
	for (i = 0; i < GENERATIONS; ++i) {
		int j = 0;
		x = 1;
		y = 1;

		bestInPopulation = getBestIndex(population);
		numberOfMutations = 0;

		refresh();
		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);


		fprintf(f_csv, "\n %d, %d, %d",
				i,
				population[bestInPopulation].fitness,
				(int)calculatePopulationFitness(population,
						POPULATION_SIZE) / POPULATION_SIZE);

		if ((i % 100) == 0) {
			//printf("Completed: %2.2f\%\n", ((float)i/(float)GENERATIONS)*100.0);
		}

		printIndividual(&population[bestInPopulation], &x, &y);

		move(100, 100);
		mvaddstr(++y, x, "Generation: ");
		printw("%d    ", i);
		mvaddstr(++y, x, "Fitness: ");
		printw("%d    ", population[bestInPopulation].fitness);
		mvaddstr(++y, x, "Mutation rate: ");
		printw("%f    ", MT_PROB);
		mvaddstr(++y, x, "Number of mutations this generation: ");
		printw("%d    ", numberOfMutations);
		//printf("Generation: %d Fitness: %d\n", i, newPopulation[getBestIndex(newPopulation)].fitness);
		//printf("Worst: %s, Generation: %d Fitness: %d\n", newPopulation[getWorstIndex(newPopulation)].gene, i, newPopulation[getWorstIndex(newPopulation)].fitness);

		// mvaddstr(++y, x, "Test: ");
		// printw("%d",
		// 		checkHasLearned(&newPopulation[getBestIndex(newPopulation)]));

		if (population[bestInPopulation].fitness == TRAINING_ROWS) {
			break;
		}
		memcpy(population, newPopulation,
				sizeof(struct individual) * POPULATION_SIZE);

	}

	////////////////////////

	//fclose(f_csv);

	mvaddstr(++y, x, "Found Fittest Individual!");
	y += 2;
	mvaddstr(y, x, newPopulation[getBestIndex(newPopulation)].gene);
	y += 2;
	mvaddstr(y, x, "Fitness: ");
	printw("%d", newPopulation[getBestIndex(newPopulation)].fitness);

	mvaddstr(++y, x, "Test: ");
	printw("%d", checkHasLearned(&newPopulation[getBestIndex(newPopulation)]));

	out = fopen(OUTPUT_FILE, "a");

	fprintf(out, "%d, %d\n", checkHasLearned(&population[bestInPopulation]),i);

	fclose(out);
	
	fclose(f_csv);

	free(trainingData);
	free(allData);
	free(population);
	free(newPopulation);

	mvaddstr(++y, x, "Press any key to exit");
	//getch();

	delwin(mainwin);
	endwin();
	refresh();
}

void printIndividual(struct individual *individual, int *xP, int *yP){
	refresh();
	int j = 0;
	int x = *xP;
	int y = *yP;
	for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
		if ((j + 1) % RULE_LENGTH != 0) {
			mvaddch(y, x, individual->gene[j]);
		} else {
			mvaddch(y, ++x, '|');
			mvaddch(y, ++x, individual->gene[j]);
			y++;
			x = 0;
		}
		x++;

	}

	*xP = x;
	*yP = y;
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
	double randomNumber = randfrom(0.0,1.0);
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
			for (int k = 0; k < RULE_LENGTH - 1; ++k) {
				if (individual->gene[j] != '#') {
					if (individual->gene[j] == trainingData[i].input[k]) {
						++score;
					}
				} else {
					++score;
				}
				++j;
			}

			if (score == RULE_LENGTH - 1) {
				if (individual->gene[j] == trainingData[i].output) {
					++fitness;
					break;
				} else {
					break;
				}
			}
		}

	}

	return fitness;

}

void *runThread(void *threadArgs) {
	int i = 0;
	struct childPair temp;
	struct threadData *data;
	struct individual *oldPopulation;
	struct individual *newPopulation;

	data = (struct threadData *) threadArgs;
	oldPopulation = data->oldPopulation;
	newPopulation = data->newPopulation;

	for (i = data->startIndex; i < data->stopPoint; ++i) {
		//Carry out 2 tournaments to select 2 parents for mating
		int p1 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);
		int p2 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);

		temp = crossover(oldPopulation[p1], oldPopulation[p2]);

		mutateIndividual(&temp.child[0]);
		newPopulation[i] = temp.child[0];
		newPopulation[i].fitness = calculateFitness(&newPopulation[i]);

		++i;

		if (i != data->stopPoint) {
			mutateIndividual(&temp.child[1]);
			newPopulation[i] = temp.child[1];
			newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
		}
	}
	pthread_exit((void *) threadArgs);
}

struct childPair crossover(struct individual parent1, struct individual parent2) {
	struct childPair children;
	int splitPoint = rand() % INDIVIDUAL_LENGTH;
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
	children.child[0].gene[INDIVIDUAL_LENGTH] = '\0';
	children.child[1].gene[INDIVIDUAL_LENGTH] = '\0';
	//children.child[0].fitness = calculateFitness(children.child[0]);
	//children.child[1].fitness = calculateFitness(children.child[1]);

	return children;

}

void createNewPopulation(struct individual *oldPopulation,
		struct individual *newPopulation) {
	int i = 0;
	int rc;

	int split = POPULATION_SIZE / NUM_THREADS;
	struct threadData data[NUM_THREADS];
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int i = 0; i < NUM_THREADS; ++i) {
		data[i].oldPopulation = oldPopulation;
		data[i].newPopulation = newPopulation;
		data[i].startIndex = split * i;
		data[i].stopPoint = data[i].startIndex + split;
		rc = pthread_create(&threads[i], &attr, runThread, (void *) &data[i]);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	pthread_attr_destroy(&attr);

	for (int i = 0; i < NUM_THREADS; ++i) {
		rc = pthread_join(threads[i], NULL);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
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
	int best = -1;
	int challengerIndex = 0;
	int i = 0;

	// Select tournament pool for this iteration
	for (i = 0; i < tournamentSize; ++i) {
		challengerIndex = rand() % populationSize;
		if ((best == -1)
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
		int mutateTo = 0;
		if (probability(0, MT_PROB)) {
			numberOfMutations++;
			if ((i + 1) % (RULE_LENGTH) != 0) {
				mutateTo = rand() % 3;
				while (vals[mutateTo] == individual->gene[i]) {
					mutateTo = rand() % 3;
				}
				individual->gene[i] = vals[mutateTo];
			} else {
				mutateTo = rand() % 2;
				while (vals[mutateTo] == individual->gene[i]) {
					mutateTo = rand() % 2;
				}
				individual->gene[i] = vals[mutateTo];
			}
		}
	}

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
	int best = -1;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((best == -1)
				|| (population[i].fitness > population[best].fitness)) {
			best = i;
		} else if (population[i].fitness == population[best].fitness) {
			int random = rand() % 2;
			if (random == 0) {
				best = i;
			} else {
				best = best;
			}
		}
	}

	return best;
}

int getWorstIndex(struct individual* population) {
	int worst = -1;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((worst == -1)
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

	allData = malloc(sizeof(struct ioData));

	f_data = fopen(DATA_FILE, "r");

	int i = 0;

	while ((line = fgets(input_test, 2048, f_data)) != NULL) {
		record = strtok(line, " ");

		for (int j = 0; j < RULE_LENGTH - 1; j++) {
			allData[i].input[j] = (record[j]);
		}

		record = strtok(NULL, " ");
		allData[i].output = *record;

		++i;
		allData = realloc(allData, sizeof(struct ioData) * (i + 1));
	}
	fclose(f_data);
}

void selectTrainingData() {
	int selected[TRAINING_ROWS];
	int i = 0;
	int j = 0;
	bool hasNumber = false;

	for (i = 0; i < TRAINING_ROWS; i++) {
		selected[i] = -1;
	}

	for (i = 0; i < TRAINING_ROWS; i++) {
		int randomIndex = rand() % TESTING_ROWS;
		do {
			randomIndex = rand() % TESTING_ROWS;
			hasNumber = false;
			for (j = 0; j < TRAINING_ROWS; j++) {
				if (selected[j] == randomIndex) {
					hasNumber = true;
					break;
				}
			}

		} while (hasNumber == true);

		selected[i] = randomIndex;
		trainingData[i] = allData[selected[i]];
	}

}

int checkHasLearned(struct individual *individual) {
	int i = 0;
	int j = 0;
	int score = 0;
	int outputIndex = 0;
	int yays = 0;

	for (i = 0; i < TESTING_ROWS; ++i) {
		int k = 0;
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			score = 0;
			for (int k = 0; k < RULE_LENGTH - 1; k++) {

				if (individual->gene[j] != '#') {
					if (individual->gene[j] == allData[i].input[k]) {
						++score;
					}
				} else {
					++score;
				}

				++j;
			}

			if (score == RULE_LENGTH - 1) {
				if (individual->gene[j] == allData[i].output) {
					//printf("checked gene: %d\n", j+1);
					yays++;
					break;
				} else {
					//i = TESTING_ROWS;
					break;
				}
			}
		}
	}

	return yays;
}

double randfrom(double min, double max) {
		double random = (double) rand() / RAND_MAX;
		return min + (random * (max - min));
}
