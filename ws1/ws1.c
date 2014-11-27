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

	readInData();
	selectTrainingData();

	f_csv = fopen("history.csv", "w");

	fprintf(f_csv, "BEST FITNESS, TEST, MEAN,");

	fclose(f_csv);

	refresh();
	//Set-up initial population
	mvaddstr(0,0,"Initialising population...");
	for (i = 0; i < POPULATION_SIZE; ++i) {
		char vals[3] = { '0', '1' };
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			if ((j + 1) % RULE_LENGTH != 0) {
				population[i].gene[j].lowerBound = randfrom(0,1);
				population[i].gene[j].upperBound = randfrom(population[i].gene[j].lowerBound,1);

			} else {
				population[i].gene[j].output = vals[rand() % 2];

			}
			//printf("%d, %lf, %lf, %d\n",i, population[i].gene[j].lowerBound, population[i].gene[j].upperBound, population[i].gene[j].output);
		}
		population[i].fitness = calculateFitness(&population[i]);
		///
		if(i%(POPULATION_SIZE/100)==0){
			mvaddch(1,0,'[');
			for(int prog = 0; prog < (double)i/(double)POPULATION_SIZE*50; prog++){
				refresh();
				mvaddch(1,prog+1,'#');
			}
			mvaddch(1,51,']');
			mvprintw(1,53,"%1.2f\%", (double)i/(double)POPULATION_SIZE*100);
			addch('\%');
		}
		///
	}
	mvaddch(1,50,'#');
	mvprintw(1,53,"%1.2f\%", 100.00);
	addstr(" Done!");

	memcpy(newPopulation, population, sizeof(struct individual) * POPULATION_SIZE);


  //////////////////////// Generation loop

	// NCurses coords
	int x = 1;
	int y = 1;
	int bestInPopulation = 0;
	for (i = 0; i < GENERATIONS; ++i) {
		int j = 0;
		time_t begin, end = 0;
		double timeSpent = 0;
		begin = clock();
		bestInPopulation = getBestIndex(population);
		x = 1;
		y = 1;
		refresh();
		createNewPopulation(population, newPopulation);
		selectBestFromPreviousPopulation(newPopulation, population);

		clear();
		f_csv = fopen("history.csv", "a");
		fprintf(f_csv, "\n %d, %d, %d",
				population[bestInPopulation].fitness,
				checkHasLearned(&population[bestInPopulation]) - population[bestInPopulation].fitness,
				calculatePopulationFitness(population, POPULATION_SIZE) / POPULATION_SIZE);
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

		move(100, 100);
		mvaddstr(++y, x, "Generation: ");
		printw("%d    ",i);
		mvaddstr(++y, x, "Fitness: ");
		printw("%d    ", population[bestInPopulation].fitness);
		mvaddstr(++y, x, "Time to calculate generation: ");
		printw("%f    ", timeSpent);
		mvaddstr(++y, x, "Test: ");
		printw("%d    ", checkHasLearned(&population[bestInPopulation]));

		if(population[bestInPopulation].fitness == TRAINING_ROWS){
			break;
		}
		memcpy(population, newPopulation, sizeof(struct individual) * POPULATION_SIZE);

	}

	// CLEAN UP

	out = fopen(OUTPUT_FILE, "w");

	fprintf(out, "Test matches: %d", checkHasLearned(&newPopulation[getBestIndex(newPopulation)]));

	fclose(out);
	//fclose(f_csv);

	free(trainingData);
	free(allData);
	free(population);
	free(newPopulation);

	// mvaddstr(++y,x, "Press any key to exit");
	getch();

	delwin(mainwin);
  endwin();
  refresh();
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

void crossover(struct individual parent1, struct individual parent2, struct childPair *children) {
	int splitPoint = (rand() % NO_RULES) * RULE_LENGTH;
	int splitPoint2 = (rand() % NO_RULES) * RULE_LENGTH;
	int i = 0;
	int j = 0;

	if (probability(0, CV_PROB)) {
		for (i = 0; i < splitPoint; ++i) {
			children->child[0].gene[i] = parent1.gene[i];
		}
		for (i = splitPoint; i < splitPoint + RULE_LENGTH; ++i) {
			children->child[0].gene[i] = parent2.gene[i];

		}
		for (i = splitPoint + RULE_LENGTH; i < INDIVIDUAL_LENGTH; ++i) {
			children->child[0].gene[i] = parent1.gene[i];
		}

		for (j = 0; j < splitPoint2; ++j) {
			children->child[1].gene[j] = parent2.gene[j];
		}
		for(j = splitPoint2; j < splitPoint2 + RULE_LENGTH; ++j){
			children->child[1].gene[j] = parent1.gene[j];
		}
		for(j = splitPoint2 + RULE_LENGTH; j < INDIVIDUAL_LENGTH; ++j){
			children->child[1].gene[j] = parent2.gene[j];
		}

	} else {
		children->child[0] = parent1;
		children->child[1] = parent2;
	}
}


void createNewPopulation(struct individual *oldPopulation,
		struct individual *newPopulation) {
	struct childPair temp;
	int i = 0;

	for (i = 0; i < POPULATION_SIZE; ++i) {
		//Carry out 2 tournaments to select 2 parents for mating
		int p1 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);
		int p2 = tournamentSelection(oldPopulation, T_SIZE, POPULATION_SIZE);

		crossover(oldPopulation[p1], oldPopulation[p2], &temp);

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

int rouletteSelection(struct individual *population, int populationSize) {
	int totalFitness = calculatePopulationFitness(population, POPULATION_SIZE);
	int selectedIndividual = 0;
	int random = rand()%totalFitness;
	int rollingTotal = 0;
	int i = 0;

	do{
		rollingTotal += population[i].fitness;
		selectedIndividual = i;
		++i;
	}while(rollingTotal < random);

	return selectedIndividual;
}

void mutateIndividual(struct individual *individual) {
	int vals[3] = { '0', '1' };
	for (int i = 0; i < INDIVIDUAL_LENGTH; ++i) {
		int mutateTo = 0;
		if (probability(0, MT_PROB)) {
		if ((i+1) % (RULE_LENGTH) != 0) {
			if (rand()%2 == 0) {
				if(rand()%2==0){
					individual->gene[i].lowerBound = individual->gene[i].lowerBound - randfrom(0,0.1);
				} else {
					individual->gene[i].lowerBound = individual->gene[i].lowerBound + randfrom(0,0.1);
				}
				if(individual->gene[i].lowerBound < 0){
					individual->gene[i].lowerBound = 0.000000;
				}

				if (individual->gene[i].lowerBound > 1){
					individual->gene[i].lowerBound = 1.000000;
				}
			} else {
				if(rand()%2==0){
					individual->gene[i].upperBound = individual->gene[i].upperBound - randfrom(0,0.1);
				} else {
					individual->gene[i].upperBound = individual->gene[i].upperBound + randfrom(0,0.1);
				}
				if (individual->gene[i].upperBound > 1){
					individual->gene[i].upperBound = 1.000000;
				}
				if(individual->gene[i].upperBound < 0){
					individual->gene[i].upperBound = 0.000000;
				}
			}

			if(individual->gene[i].lowerBound > individual->gene[i].upperBound){
			    double swap = individual->gene[i].lowerBound;
			    individual->gene[i].lowerBound = individual->gene[i].upperBound;
			    individual->gene[i].upperBound = swap;
			}
		}
	} else {
				mutateTo = rand()%2;
				while(vals[mutateTo] == individual->gene[i].output){
					mutateTo = rand()%2;
				}
				individual->gene[i].output = vals[mutateTo];
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
			}

		}while(hasNumber == true);

		selected[i] = randomIndex;
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
