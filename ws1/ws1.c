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
	struct individual *population = malloc(sizeof(struct individual) * POPULATION_SIZE);


	if ( (mainwin = initscr()) == NULL ) {
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(EXIT_FAILURE);
	}
	curs_set(0);
	srand(getSeed());

	//Initialise global data variables
	readInData();
	selectTrainingData();

	//Write header to CSV
	f_csv = fopen("history.csv", "w");
	fprintf(f_csv, "BEST FITNESS, TEST, MEAN,");
	fclose(f_csv);

	refresh();

	//Set-up initial population
  initPopulation(population);

	//initialise newPopulation variable
	memcpy(newPopulation, population, sizeof(struct individual) * POPULATION_SIZE);


	// NCurses coords
	int x = 1;
	int y = 1;


	int bestInPopulation = 0;
	int worstInNewPopulation = 0;
	int bestInNewPopulation = 0;
	int meanPopulationFitness = 0;
	int meanNewPopulationFitness = 0;
	int previousBest = 0;
	int flatLine = 0;
	bool mutating = false;
	//////////////////////// Generation loop below ///////////////////////////////
	for (i = 0; i < GENERATIONS; ++i) {
		int j = 0;
		time_t begin, end = 0;
		double timeSpent = 0;
		begin = clock();
		bestInPopulation = getBestIndex(population);
		meanPopulationFitness = calculatePopulationFitness(population, POPULATION_SIZE) / POPULATION_SIZE;

		x = 1;
		y = 1;
		refresh();
		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);

		clear();
		f_csv = fopen("history.csv", "a");
		fprintf(f_csv, "\n %d, %d, %d",
				population[bestInPopulation].fitness,
				checkHasLearned(&population[bestInPopulation]),
				meanPopulationFitness);
		fclose(f_csv);

		 for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			x+=20;
		 	if ((j + 1) % RULE_LENGTH != 0) {
		 		mvprintw(y,x,"{%f,%f}",population[bestInPopulation].gene[j].lowerBound,population[bestInPopulation].gene[j].upperBound);

		 	} else {
		 		mvaddch(y,++x,'|');
		 		mvaddch(y,++x,newPopulation[getBestIndex(newPopulation)].gene[j].output);
		 		y++;
				x=1;
		 	}
		 }

		end = clock();
		timeSpent = (double)(end - begin)/CLOCKS_PER_SEC;

		++gen;

		double accuracy = (double)checkHasLearned(&population[bestInPopulation])/(double)TESTING_ROWS*100.0;
		double accuracy2 = (double)population[bestInPopulation].fitness/(double)TRAINING_ROWS*100.0;
		//double accuracy2 = ((double)checkHasLearned(&population[bestInPopulation])-(double)population[bestInPopulation].fitness)/(double)(TESTING_ROWS-TRAINING_ROWS)*100;

		move(100, 100);
		mvaddstr(++y, x, "Generation: ");
		printw("%d    ",i);
		mvaddstr(++y, x, "Fitness: ");
		printw("%d    ", population[bestInPopulation].fitness);
		mvaddstr(++y, x, "Time to calculate generation: ");
		printw("%f    ", timeSpent);
		mvaddstr(++y, x, "Test: ");
		printw("%d    ", checkHasLearned(&population[bestInPopulation]));
		mvaddstr(++y, x, "Accuracy: ");
		printw("%1.2f    ", accuracy);
		mvaddstr(++y, x, "Accuracy2: ");
		printw("%1.2f    ", accuracy2);
		mvaddstr(++y, x, "MT_PROB: ");
		printw("%f    ", MT_PROB);
		mvaddstr(++y, x, "CV_PROB: ");
		printw("%f    ", CV_PROB);
		mvaddstr(++y, x, "meanOld: ");
		printw("%d    ", meanPopulationFitness);


		bestInNewPopulation = getBestIndex(newPopulation);
		worstInNewPopulation = getWorstIndex(newPopulation);
		meanNewPopulationFitness = calculatePopulationFitness(newPopulation, POPULATION_SIZE) / POPULATION_SIZE;

		mvaddstr(++y, x, "meanNew: ");
		printw("%d    ", meanNewPopulationFitness);

		// MT_PROB -= 0.0005;
		// if(MT_PROB < D_MT_PROB){
		// 	MT_PROB = 0.1;
		// }

		if(population[bestInPopulation].fitness == TRAINING_ROWS){
			break;
		}

		memcpy(population, newPopulation, sizeof(struct individual) * POPULATION_SIZE);

	}

	//////////////////////// Generation loop above ///////////////////////////////

	// CLEAN UP

	out = fopen(OUTPUT_FILE, "a");

	fprintf(out, "%d, %d\n", checkHasLearned(&population[bestInPopulation]),i);

	fclose(out);
	//fclose(f_csv);

	free(trainingData);
	free(allData);
	free(population);
	free(newPopulation);

	// mvaddstr(++y,x, "Press any key to exit");

	delwin(mainwin);
  endwin();
  refresh();
}

/**
*	Initialise a population with random individuals
*
*/
void initPopulation(struct individual *population){
	for (int i = 0; i < POPULATION_SIZE; ++i) {
		char vals[3] = { '0', '1', '2' };
		for (int j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			if ((j + 1) % RULE_LENGTH != 0) {
				population[i].gene[j].lowerBound = fabs(randfrom(0,10));
				population[i].gene[j].upperBound = fabs(randfrom(population[i].gene[j].lowerBound,10));

			} else {
				population[i].gene[j].output = vals[rand() % 3];

			}
		}
		population[i].fitness = calculateFitness(&population[i]);
	}
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
			for (int k = 0; k < RULE_LENGTH-1; k++) {

						if ((individual->gene[j].lowerBound <= trainingData[i].input[k]) &&
								(individual->gene[j].upperBound >= trainingData[i].input[k])) {
							++score;
						}

				++j;
			}

			if (score == RULE_LENGTH-1) {
				if (individual->gene[j].output == trainingData[i].output) {
					fitness++;
					break;
				} else {
					break;
				}
			}
		}

	}

	return fitness;

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
		}
		for(j = splitPoint2 + RULE_LENGTH; j < INDIVIDUAL_LENGTH; ++j){
			children.child[1].gene[j] = parent2.gene[j];
		}

	} else {
		children.child[0] = parent1;
		children.child[1] = parent2;
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

	data = (struct threadData *) threadArgs;
	oldPopulation = data->oldPopulation;
	newPopulation = data->newPopulation;

	for (i = data->startIndex; i < data->stopPoint; ++i) {
		//Carry out 2 tournaments to select 2 parents for mating
		int p1 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);
		int p2 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);

		temp = crossover(oldPopulation[p1], oldPopulation[p2]);
    calculateFitness(&temp.child[0]);
		mutateIndividual(&temp.child[0]);

		newPopulation[i] = temp.child[0];
		newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
		newPopulation[i].fitness;

		++i;

		if (i != data->stopPoint) {
		  calculateFitness(&temp.child[1]);
			 mutateIndividual(&temp.child[1]);
			newPopulation[i] = temp.child[1];
			newPopulation[i].fitness = calculateFitness(&newPopulation[i]);
			newPopulation[i].fitness;

		}

	}
	pthread_exit((void *) threadArgs);
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

	//printf("Best from tournament: %d Fitness: %d\n", best, population[best].fitness);
	return best;
}

/**
*	Carry out mutations on the individual passed in
*
*/
void mutateIndividual(struct individual *individual) {
	int vals[3] = { '0', '1', '2'};

	for (int i = 0; i < INDIVIDUAL_LENGTH; ++i) {
		int mutateTo = 0;

		if(individual->gene[i].lowerBound > individual->gene[i].upperBound){
				double swap = individual->gene[i].lowerBound;
				individual->gene[i].lowerBound = individual->gene[i].upperBound;
				individual->gene[i].upperBound = swap;
		}


		if ((i+1) % (RULE_LENGTH) != 0) {

			if (probability(0, MT_PROB)) {
				if(rand()%2==0){
					individual->gene[i].lowerBound = fabs(individual->gene[i].lowerBound - randfrom(0,0.1));
				} else {
					individual->gene[i].lowerBound = fabs(individual->gene[i].lowerBound + randfrom(0,0.1));
				}

			}

			if (probability(0, MT_PROB)) {
				if(rand()%2==0){
					individual->gene[i].upperBound = fabs(individual->gene[i].upperBound - randfrom(0,0.1));
				} else {
					individual->gene[i].upperBound = fabs(individual->gene[i].upperBound + randfrom(0,0.1));
				}

			}

		} else {
			if (probability(0, MT_PROB)) {
				mutateTo = rand()%3;
				while(vals[mutateTo] == individual->gene[i].output){
					mutateTo = rand()%3;
				}
				individual->gene[i].output = vals[mutateTo];
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
		//int worstIndex = getWorstIndex(newPopulation);
		memcpy(&newPopulation[0], &oldPopulation[bestOld],
				sizeof(oldPopulation[bestOld]));
	}
}

int getBestIndex(struct individual* population) {
	int best = -1;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		if ((best == -1)
				|| (population[i].fitness >= population[best].fitness)) {
			best = i;
		}
		else if (population[i].fitness == population[best].fitness) {
			int random = rand()%2;
			if(random == 0){
				best = i;
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
		else if (population[i].fitness == population[worst].fitness) {
			int random = rand()%2;
			if(random == 0){
				worst = i;
			}
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
			allData[i].input[j] = atof(record);
			record = strtok(NULL, " ");
		}
		allData[i].output = *record;

		++i;
		allData = realloc(allData, sizeof(struct ioData) * (i + 1));
	}
	fclose(f_data);
}

void selectTrainingData(){
	int selected[TRAINING_ROWS];
	int i = 0;
	int j = 0;
	bool hasNumber = false;

	for(i = 0; i < TRAINING_ROWS; i++){
		selected[i] = -1;
	}

	for(i = 0; i < TRAINING_ROWS; i++){
		int randomIndex = rand()%TESTING_ROWS;
		do{
			randomIndex = rand()%TESTING_ROWS;
			hasNumber = false;
			for(j = 0; j < TRAINING_ROWS; j++){
				//printf("%d\n", randomIndex);
					if(selected[j] == randomIndex){
						hasNumber = true;
						break;
					}
					//printf("%d\n",j);
			}

		}while(hasNumber == true);

		selected[i] = randomIndex;
		//printf("%d\n", i);
	  trainingData[i] = allData[selected[i]];
	}

}

int checkHasLearned(struct individual *individual) {
	int i = 0;
	int j = 0;
	int fitness = 0;
	int score = 0;
	int yays = 0;

	for (i = 0; i < TESTING_ROWS; ++i) {
		int k = 0;
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			score = 0;
			for (int k = 0; k < RULE_LENGTH-1; k++) {
					//if (individual->gene[j] != '#') {
						if ((individual->gene[j].lowerBound <= allData[i].input[k]) &&
								(individual->gene[j].upperBound >= allData[i].input[k])) {
							++score;
						}
				++j;
			}

			if (score == RULE_LENGTH-1) {
				if (individual->gene[j].output == allData[i].output) {
					yays++;
					break;
				} else {
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
