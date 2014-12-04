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

	//Initialise global data variables
	readInData();
	selectTrainingData();

	//Write header to CSV
	f_csv = fopen("history.csv", "w");
	fprintf(f_csv, "GENERATION, BEST FITNESS, MEAN, TEST");
	fclose(f_csv);

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

	//////////////////////// Generation loop below ///////////////////////////////

	// NCurses coords
	int x = 1;
	int y = 1;
	int bestInPopulation = 0;
	int worstInNewPopulation = 0;
	int bestInNewPopulation = 0;
	int meanPopulationFitness = 0;
	int meanNewPopulationFitness = 0;
	double theta = 0;
	for (i = 0; i < GENERATIONS; ++i) {
		int j = 0;
		x = 1;
		y = 1;

		bestInPopulation = getBestIndex(population);
		meanPopulationFitness = calculatePopulationFitness(population, POPULATION_SIZE) / POPULATION_SIZE;
		numberOfMutations = 0;

		refresh();
		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);

		f_csv = fopen("history.csv", "a");
		fprintf(f_csv, "\n %d, %d, %d, %d",
				i,
				population[bestInPopulation].fitness,
				(int)calculatePopulationFitness(population,
						POPULATION_SIZE) / POPULATION_SIZE,
						checkHasLearned(&population[bestInPopulation]));
		fclose(f_csv);

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
		mvaddstr(++y, x, "gMP: ");
		printw("%lf    ", gMP);
		mvaddstr(++y, x, "gCP: ");
		printw("%lf    ", gCP);
		mvaddstr(++y, x, "MT_PROB: ");
		printw("%f    ", MT_PROB);
		mvaddstr(++y, x, "CV_PROB: ");
		printw("%f    ", CV_PROB);
		mvaddstr(++y, x, "Theta: ");
		printw("%f    ", theta);
		mvaddstr(++y, x, "meanOld: ");
		printw("%d    ", meanPopulationFitness);

		bestInNewPopulation = getBestIndex(newPopulation);
		worstInNewPopulation = getWorstIndex(newPopulation);
		meanNewPopulationFitness = calculatePopulationFitness(newPopulation, POPULATION_SIZE) / POPULATION_SIZE;

		//Attempt at using clever dynamic mutation and crossover
/*
		if(newPopulation[bestInNewPopulation].fitness > newPopulation[worstInNewPopulation].fitness) {
			theta = 0.01* ((double)newPopulation[bestInNewPopulation].fitness - (double)meanNewPopulationFitness)/((double)newPopulation[bestInNewPopulation].fitness - (double)newPopulation[worstInNewPopulation].fitness);
		} else if (newPopulation[bestInNewPopulation].fitness == newPopulation[worstInNewPopulation].fitness) {
			theta = 0.01;
		}

		if(gCP > gMP){
				MT_PROB -= theta;
				CV_PROB += theta;
		} else {
				MT_PROB += theta;
				CV_PROB -= theta;
		}

		if(CV_PROB < 0.001){
				CV_PROB = 0.001;
		}

		if(MT_PROB < 0.001){
				MT_PROB = 0.01;
		}


		if(CV_PROB > 0.9){
			CV_PROB = 0.9;
		}
		if(MT_PROB > 1.0){
			MT_PROB = 1.0;
		}
*/
		if(MT_PROB < 0.00001){
			MT_PROB = 0.1;
		}

		MT_PROB -= 0.005;

		memcpy(population, newPopulation,
				sizeof(struct individual) * POPULATION_SIZE);

	}

	//////////////////////// Generation loop above ///////////////////////////////


	mvaddstr(++y, x, "Found Fittest Individual!");
	y += 2;
	mvaddstr(y, x, population[bestInPopulation].gene);
	y += 2;
	mvaddstr(y, x, "Fitness: ");
	printw("%d", population[bestInPopulation].fitness);

	mvaddstr(++y, x, "Test: ");
	printw("%d", checkHasLearned(&population[bestInPopulation]));

	out = fopen(OUTPUT_FILE, "a");

	fprintf(out,"%d, %d\n",
			checkHasLearned(&population[bestInPopulation]), i);

	fclose(out);

	free(trainingData);
	free(allData);
	free(population);
	free(newPopulation);

	mvaddstr(++y, x, "Press any key to exit");

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

/**
*	Calculate the fitness of the population
*
*/
long calculatePopulationFitness(struct individual *population, int arrSize) {
	int i = 0;
	long totalFitness = 0;

	for (i = 0; i < arrSize; ++i) {
		totalFitness = totalFitness + population[i].fitness;
	}
	return totalFitness;
}

/**
*	Return true of false with a probability
*
*/
bool probability(float minValue, float maxValue) {
	bool retVal = false;
	double randomNumber = randfrom(0.0,1.0);
	// Check if value lands between bounds
	if ((randomNumber >= minValue) && (randomNumber <= maxValue)) {
		retVal = true;
	}

	return retVal;
}

/**
*	Get seed for srand() from /dev/random
*
*/
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

/**
*	Calculate the fitness of the individual passed in, returns
* the fitness
*
*/
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
					//i = TRAINING_ROWS;
					break;
				}
			}
		}

	}

	return fitness;

}

/**
*	Thread initialised from createNewPoplation()
*
*/
void *runThread(void *threadArgs) {
	int i = 0;
	struct childPair temp;
	struct threadData *data;
	struct individual *oldPopulation;
	struct individual *newPopulation;
	int p1_cv = 0, p2_cv = 0, c1_cv = 0, c2_cv = 0, c1_mt = 0, c2_mt = 0;

	data = (struct threadData *) threadArgs;
	oldPopulation = data->oldPopulation;
	newPopulation = data->newPopulation;

	for (i = data->startIndex; i < data->stopPoint; ++i) {
		//Carry out 2 tournaments to select 2 parents for mating
		int p1 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);
		int p2 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);

		temp = crossover(oldPopulation[p1], oldPopulation[p2]);
		data->numCv += temp.happened;
		c1_cv = calculateFitness(&temp.child[0]);
		data->numMt += mutateIndividual(&temp.child[0]);

		newPopulation[i] = temp.child[0];
		newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
		c1_mt = newPopulation[i].fitness;

		++i;

		if (i != data->stopPoint) {
			c2_cv = calculateFitness(&temp.child[1]);
			data->numMt += mutateIndividual(&temp.child[1]);
			newPopulation[i] = temp.child[1];
			newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
			c2_mt = newPopulation[i].fitness;
		}
		p1_cv = oldPopulation[p1].fitness;
		p2_cv = oldPopulation[p2].fitness;
		data->CP += (c1_cv+c2_cv)-(p1_cv+p2_cv);
		data->MP += (c1_mt+c2_mt)-(p1_cv+p2_cv);
	}
	pthread_exit((void *) threadArgs);
}

/**
*	Crossover the two individuals, returns a pair of new individuals
*
*/
struct childPair crossover(struct individual parent1, struct individual parent2) {
	struct childPair children;
	int splitPoint = (rand() % NO_RULES) * RULE_LENGTH;
	int splitPoint2 = (rand() % NO_RULES) * RULE_LENGTH;
	int i = 0;
	int j = 0;

	if (probability(0, CV_PROB)) {
		children.happened=2;
		for (i = 0; i < splitPoint; ++i) {
			children.child[0].gene[i] = parent1.gene[i];
		}
		for (i = splitPoint; i < splitPoint + RULE_LENGTH; ++i) {
			children.child[0].gene[i] = parent2.gene[i];

		}
		for (i = splitPoint + RULE_LENGTH; i < INDIVIDUAL_LENGTH; ++i) {
			children.child[0].gene[i] = parent1.gene[i];
		}

		for (j = 0; j < splitPoint2; ++j) {
			children.child[1].gene[j] = parent2.gene[j];
		}
		for(j = splitPoint2; j < splitPoint2 + RULE_LENGTH; ++j){
			children.child[1].gene[j] = parent1.gene[j];
			//children.happened++;
		}
		for(j = splitPoint2 + RULE_LENGTH; j < INDIVIDUAL_LENGTH; ++j){
			children.child[1].gene[j] = parent2.gene[j];
		}

	} else {
		children.happened=2;
		children.child[0] = parent1;
		children.child[1] = parent2;
		//children.happened = 0;
	}


	return children;

}

/**
*	Creates a new population of individuals, crossed and mutated from the oldPopulation
*
*/
void createNewPopulation(struct individual *oldPopulation,
		struct individual *newPopulation) {
	int i = 0;
	int rc;

	int split = POPULATION_SIZE / NUM_THREADS;
	struct threadData data[NUM_THREADS];
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	int numCv;
	int numMt;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int i = 0; i < NUM_THREADS; ++i) {
		data[i].oldPopulation = oldPopulation;
		data[i].newPopulation = newPopulation;
		data[i].startIndex = split * i;
		data[i].stopPoint = data[i].startIndex + split;
		data[i].CP = 0;
		data[i].MP = 0;
		data[i].numMt = 0;
		data[i].numCv = 0;
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

	//Calculations to work out if crossover or mutation is currently
	//doing the best
	gCP = 0;
	gMP = 0;
	numCv = 0;
	numMt = 0;
	for (int i = 0; i < NUM_THREADS; ++i) {
			gCP += data[i].CP;
			gMP += data[i].MP;
			numCv += data[i].numCv;
			numMt += data[i].numMt;
	}

	gCP = (1.0/(double)numCv)*(double)gCP;
	gMP = (1.0/(double)numMt)*(double)gMP;

}

/**
*	Tournament selection algorithm
*
*/
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

	return best;
}

/**
*	Carry out mutations on the individual passed in
*
*/
int mutateIndividual(struct individual *individual) {
	int vals[3] = { '0', '1', '#' };
	int mutations = 0;

	for (int i = 0; i < INDIVIDUAL_LENGTH; ++i) {
		int mutateTo = 0;
		if (probability(0, MT_PROB)) {
			mutations++;
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
	return mutations;
}

/**
* Select best individual from previous population and add it to the
* new population
*/
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

/**
*	Get the index of the best individual in the population
*
*/
int getBestIndex(struct individual* population) {
	int best = -1;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((best == -1)
				|| (population[i].fitness >= population[best].fitness)) {
			best = i;
		}
		else if (population[i].fitness == population[best].fitness) {
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

/**
*	Get the index of the worst individual in the population
*
*/
int getWorstIndex(struct individual* population) {
	int worst = -1;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((worst == -1)
				|| (population[i].fitness <= population[worst].fitness)) {
			worst = i;
		}
		else if (population[i].fitness == population[worst].fitness) {
			int random = rand() % 2;
			if (random == 0) {
				worst = i;
			} else {
				worst = worst;
			}
		}
	}

	return worst;
}

/**
*	Read in the data from the file
*
*/
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

/**
*	Select a percentage of the data to train with
*
*/
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

/**
*	Check if the population has learned with the full data file
*
*/
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

/**
* Generate random float
*
*/
double randfrom(double min, double max) {
		double random = (double) rand() / RAND_MAX;
		return min + (random * (max - min));
}
