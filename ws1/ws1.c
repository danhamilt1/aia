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
				population[i].gene[j].lowerBound = fabs(randfrom(0,1));
				population[i].gene[j].upperBound = fabs(randfrom(population[i].gene[j].lowerBound,1));

			} else {
				population[i].gene[j].output = vals[rand() % 2];

			}
			//printf("%d, %lf, %lf, %d\n",i, population[i].gene[j].lowerBound, population[i].gene[j].upperBound, population[i].gene[j].output);
		}
		population[i].fitness = calculateFitness(&population[i]);
		///
		if(i%(POPULATION_SIZE/10)==0){
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
	int worstInNewPopulation = 0;
	int bestInNewPopulation = 0;
	int meanPopulationFitness = 0;
	int meanNewPopulationFitness = 0;
	double theta = 0;
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

		move(100, 100);
		mvaddstr(++y, x, "Generation: ");
		printw("%d    ",i);
		mvaddstr(++y, x, "Fitness: ");
		printw("%d    ", population[bestInPopulation].fitness);
		mvaddstr(++y, x, "Time to calculate generation: ");
		printw("%f    ", timeSpent);
		mvaddstr(++y, x, "Test: ");
		printw("%d    ", checkHasLearned(&population[bestInPopulation]));
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

		mvaddstr(++y, x, "meanNew: ");
		printw("%d    ", meanNewPopulationFitness);

		if(newPopulation[bestInNewPopulation].fitness > newPopulation[worstInNewPopulation].fitness) {
			theta = 0.01* ((double)newPopulation[bestInNewPopulation].fitness - (double)meanNewPopulationFitness)/((double)newPopulation[bestInNewPopulation].fitness - (double)newPopulation[worstInNewPopulation].fitness);
		} else if (newPopulation[bestInNewPopulation].fitness == newPopulation[worstInNewPopulation].fitness) {
			theta = 0.01;
		}
		///
		//theta = 0.01;
		///

        if(gCP > gMP){
            MT_PROB -= theta;
            CV_PROB += theta;
        } else {
            MT_PROB += theta;
            CV_PROB -= theta;
        }

        if(CV_PROB < 0.01){
            CV_PROB = 0.01;
        }

				if(MT_PROB < 0.01){
						MT_PROB = 0.01;
				}


				if(CV_PROB > 1.0){
					CV_PROB = 1.0;
				}
				if(MT_PROB > 1.0){
					MT_PROB = 1.0;
				}


		if(population[bestInPopulation].fitness == TRAINING_ROWS){
			//break;
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

struct childPair crossover(struct individual parent1, struct individual parent2) {
	struct childPair children;
	int splitPoint = rand() % INDIVIDUAL_LENGTH;
	int i = 0;

	if (probability(0, CV_PROB)) {
	    children.happened = 1;
		for (i = 0; i < splitPoint; ++i) {
			children.child[0].gene[i] = parent1.gene[i];
			children.child[1].gene[i] = parent2.gene[i];
		}

		for (i = splitPoint; i < INDIVIDUAL_LENGTH; ++i) {
			children.child[0].gene[i] = parent2.gene[i];
			children.child[1].gene[i] = parent1.gene[i];
		}
        children.happened = 1;
	} else {
		children.child[0] = parent1;
		children.child[1] = parent2;
		children.happened = 0;
	}


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

void *runThread(void *threadArgs) {
	int i = 0;
	struct childPair temp;
	struct threadData *data;
	struct individual *oldPopulation;
	struct individual *newPopulation;
    int p1_cv, p2_cv, c1_cv, c2_cv, c1_mt, c2_mt = 0;

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
		data->MP += (c1_mt+c2_mt)-(c1_cv+c2_cv);

	}
	pthread_exit((void *) threadArgs);
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

int mutateIndividual(struct individual *individual) {
	int vals[3] = { '0', '1' };
	int numVals = 8;
	int mutations = 0;
	for (int i = 0; i < INDIVIDUAL_LENGTH; ++i) {
		int mutateTo = 0;

		if (probability(0, MT_PROB)) {
		if ((i+1) % (RULE_LENGTH) != 0) {
			if(rand()%2==0){
				mutations++;
				if(rand()%2==0){
					individual->gene[i].lowerBound = fabs(individual->gene[i].lowerBound - 0.01);//randfrom(0,0.05));
				} else {
					individual->gene[i].lowerBound = fabs(individual->gene[i].lowerBound + 0.01);//randfrom(0,0.05));
				}
				if(individual->gene[i].lowerBound < 0){
					individual->gene[i].lowerBound = 0.000000;
				}

				if (individual->gene[i].lowerBound > 1){
					individual->gene[i].lowerBound = 1.000000;
				}
			}
			if(rand()%2==0) {
				mutations++;
				if(rand()%2==0){
					individual->gene[i].upperBound = fabs(individual->gene[i].upperBound - 0.01);//randfrom(0,0.05));
				} else {
					individual->gene[i].upperBound = fabs(individual->gene[i].upperBound + 0.01);//randfrom(0,0.05));
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
		} else {
				mutations++;
				mutateTo = rand()%2;
				while(vals[mutateTo] == individual->gene[i].output){
					mutateTo = rand()%2;
				}
				individual->gene[i].output = vals[mutateTo];
			}
		}

	}
    return mutations;
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
				|| (population[i].fitness >= population[best].fitness)) {
			best = i;
		}
		// else if (population[i].fitness == population[best].fitness) {
		// 	int random = rand()%2;
		// 	if(random == 0){
		// 		best = i;
		// 	}
		// }
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
		// else if (population[i].fitness == population[worst].fitness) {
		// 	int random = rand()%2;
		// 	if(random == 0){
		// 		worst = i;
		// 	}
		// }
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

	for (i = 0; i < TRAINING_ROWS; ++i) {
		int k = 0;
		for (j = 0; j < INDIVIDUAL_LENGTH; ++j) {
			score = 0;
			for (int k = 0; k < RULE_LENGTH-1; k++) {
					//if (individual->gene[j] != '#') {
						if ((individual->gene[j].lowerBound <= trainingData[i].input[k]) &&
								(individual->gene[j].upperBound >= trainingData[i].input[k])) {
							++score;
						}
					//}
					//else{
					//	++score;
					//}
				++j;
			}

			if (score == RULE_LENGTH-1) {
				if (individual->gene[j].output == trainingData[i].output) {
					yays++;
					break;
				} else {
					//i = TRAINING_ROWS;
					//break;
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
