#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
//Constant Numbers
#define STUDENTS 100
#define ROOM_KEEPERS 10
const int M = 10;
const int N = 4;

//Semaphores
sem_t Rooms[10];
sem_t Room_Locks[10];
sem_t is_Room_Used[10];
sem_t AllDone;

//Threads
void* room_Keeper(void *num);
void* student(void*);

//Other Functions
void randwait();
int sem_Value(sem_t sem);
int get_Avaible_Room_Number(sem_t Semaphores[]);

int main(void) {

	int i, j;
	//Semaphore initializon
	for (i = 0; i < 10; i++)
		sem_init(&Rooms[i], 0, 4);
	for (i = 0; i < 10; i++)
		sem_init(&Room_Locks[i], 0, 1);
	for (i = 0; i < 10; i++)
		sem_init(&is_Room_Used[i], 0, 0);

	sem_init(&AllDone, 0, 0);

	//Room Keeper Threads
	pthread_t rktid[ROOM_KEEPERS];
	int Number_R[ROOM_KEEPERS];

	for (j = 0; j < ROOM_KEEPERS; j++) {
		Number_R[j] = j;
	}
	for (i = 0; i < ROOM_KEEPERS; i++) {
		pthread_create(&rktid[i], NULL, room_Keeper, (void*) &Number_R[i]);
	}

	//Student Threads
	pthread_t stid[STUDENTS];
	int Number[STUDENTS + 1];

	for (i = 1; i < STUDENTS + 1; i++) {
		Number[i] = i;
	}
	for (i = 1; i < STUDENTS + 1; i++) {
		randwait();
		pthread_create(&stid[i], NULL, student, (void*) &Number[i]);
	}

	//Joins
	for (i = 1; i < STUDENTS + 1; i++) {
		pthread_join(stid[i], NULL);
	}

	for (i = 0; i < ROOM_KEEPERS; i++) {
		pthread_join(rktid[i], NULL);
	}

	printf("All students done.\n");
	return 0;
}

void* room_Keeper(void *info) {
	int num = *(int*) info;
	int flag = 0;
	int flag2 = 0;
	int flag3 = 0;
	while (sem_Value(AllDone) != STUDENTS) {
		while (flag == 0) {
			if (sem_Value(AllDone) == STUDENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 4) {
				flag = 1;
				printf("Room keeper %d opening the room %d\n", num, num);
			}
		}
		while (flag == 1) {
			if (sem_Value(AllDone) == STUDENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 0) {
				sem_wait(&Room_Locks[num]);
				flag = 0;
				printf("Room keeper %d: Room is full!\n", num);
				sem_post(&is_Room_Used[num]);
			} else if (sem_Value(Rooms[num]) == 2 && flag2 == 0) {
				printf("Room keeper %d: Hurry up! 2 more students left!\n",
						num);
				flag2 = 1;
			} else if (sem_Value(Rooms[num]) == 1 && flag3 == 0) {
				printf("Room keeper %d: Hurry up! Last student!\n", num);
				flag3 = 1;
			}
		}
		while (flag == 0) {
			if (sem_Value(AllDone) == STUDENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 4) {
				printf("Room keeper %d cleaning the room %d\n", num, num);
				sleep(3);
				sem_post(&Room_Locks[num]);
				flag = 1;
			}
		}
		flag = 0;
		flag2 = 0;
		flag3 = 0;
	}

}

void* student(void *info) {
	int num = *(int*) info;
	printf("Student number %d waiting for a empty room\n", num);
	int m = get_Avaible_Room_Number(Rooms);
	while (m == -1) {
		m = get_Avaible_Room_Number(Rooms);
	}

	sem_wait(&Rooms[m]);

	printf("Student number %d entered the room %d\n", num, m);
	int flag = 0;
	while (flag == 0) {
		if (sem_Value(Rooms[m]) == 0) {
			printf("Student number %d studying in room %d\n", num, m);
			sleep(3);
			flag = 1;
		}
	}
	sem_post(&Rooms[m]);

	printf("Student number %d leaved the room %d\n", num, m);
	sem_post(&AllDone);
}

void randwait() {
	int random = 1;
	srand(time(NULL));
	random = (rand() % 3) + 1;
	sleep(random);
}

int sem_Value(sem_t sem) {
	int result;
	sem_getvalue(&sem, &result);
	return result;
}

int get_Avaible_Room_Number(sem_t Semaphores[]) {
	int available, i, min, min2;
	min = 99;
	min2 = 99;
	available = -1;
	for (i = 0; i < 10; i++) {
		if (sem_Value(Semaphores[i]) != 0 && sem_Value(Room_Locks[i]) != 0) {
			if (sem_Value(Semaphores[i]) <= min) {
				if (sem_Value(is_Room_Used[i]) <= min2) {
					min2 = sem_Value(is_Room_Used[i]);
					min = sem_Value(Semaphores[i]);
					available = i;

				}

			}
		}

	}
	return available;
}

