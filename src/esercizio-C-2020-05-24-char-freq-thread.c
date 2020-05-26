/*
E' data la string di caratteri content:
char * content = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Mattis rhoncus urna neque viverra justo nec ultrices. Pretium quam vulputate dignissim suspendisse in est ante. Vitae congue mauris rhoncus aenean. Blandit cursus risus at ultrices mi. Ut lectus arcu bibendum at varius vel pharetra vel. Etiam non quam lacus suspendisse faucibus interdum posuere. Eget sit amet tellus cras adipiscing enim eu turpis egestas. Lectus magna fringilla urna porttitor rhoncus dolor purus non. Sit amet consectetur adipiscing elit duis tristique sollicitudin nibh. Nec tincidunt praesent semper feugiat nibh. Sapien pellentesque habitant morbi tristique senectus et netus et malesuada.";

Il programma utilizza due thread aggiuntivi, il primo thread analizza i caratteri nelle posizioni "pari" della stringa
(content[0], content[2], content[4]...) mentre il secondo considera i caratteri nelle posizioni "dispari" della stringa
(content[1], content[3], content[5]...)

definire un array condiviso:
int counter[256];

ciascun thread incrementa il contatore (counter) corrispondente al valore di ogni carattere incontrato:
counter[content[i]]++;

usare un solo semaforo per risolvere il problema (ci sono più modi di risolverlo...)

il thread principale aspetta il termine dei due thread e poi scrive su stdout la frequenza di ogni carattere.

"frequenza %d = %d\n"
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>


#define CHECK_ERR_MMAP(a,msg) {if ((a) == MAP_FAILED) { perror((msg)); exit(EXIT_FAILURE); } }
#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

#define LETTERS 25

char * content = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Mattis rhoncus urna neque viverra justo nec ultrices. Pretium quam vulputate dignissim suspendisse in est ante. Vitae congue mauris rhoncus aenean. Blandit cursus risus at ultrices mi. Ut lectus arcu bibendum at varius vel pharetra vel. Etiam non quam lacus suspendisse faucibus interdum posuere. Eget sit amet tellus cras adipiscing enim eu turpis egestas. Lectus magna fringilla urna porttitor rhoncus dolor purus non. Sit amet consectetur adipiscing elit duis tristique sollicitudin nibh. Nec tincidunt praesent semper feugiat nibh. Sapien pellentesque habitant morbi tristique senectus et netus et malesuada.";
int * counter;
sem_t * process_semaphore;
int * counter;


void * thread_function_1(void * arg) {
	int index = 0;

	for(int i=0; i<strlen(content); i+=2){
		if(content[i] >= 97 && content[i] <= 122){
			index = ((int)content[i]) - 97;
		}
		if(content[i] >= 65 && content[i] <= 90){
			index = ((int)content[i]) - 65;
		}

		if (sem_wait(process_semaphore) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

		counter[index]++;

		if (sem_post(process_semaphore) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}
	}

	return NULL;
}


void * thread_function_2(void * arg) {
	int index = 0;

	for(int i=1; i<strlen(content); i+=2){
		if(content[i] >= 97 && content[i] <= 122){
			index = ((int)content[i]) - 97;
		}
		if(content[i] >= 65 && content[i] <= 90){
			index = ((int)content[i]) - 65;
		}

		if (sem_wait(process_semaphore) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

		counter[index]++;

		if (sem_post(process_semaphore) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}
	}

	return NULL;
}

int main() {
	pthread_t t1;
	pthread_t t2;
	void * res;
	int s;

	process_semaphore = malloc(sizeof(sem_t));
	s = sem_init(process_semaphore,
					0, // 1 => il semaforo è condiviso tra processi,
					   // 0 => il semaforo è condiviso tra threads del processo
					1 // valore iniziale del semaforo (se mettiamo 0 che succede?)
				  );
	CHECK_ERR(s,"sem_init")

	counter = mmap(NULL, // NULL: è il kernel a scegliere l'indirizzo
			LETTERS*sizeof(int), // dimensione della memory map
			PROT_READ | PROT_WRITE, // memory map leggibile e scrivibile
			MAP_SHARED | MAP_ANONYMOUS, // memory map condivisibile con altri processi e senza file di appoggio
			-1,
			0);

	if (pthread_create(&t1, NULL, thread_function_1, NULL) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&t2, NULL, thread_function_2, NULL) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	if (pthread_join(t1, &res) != 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	if (pthread_join(t2, &res) != 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	s = sem_destroy(process_semaphore);
	CHECK_ERR(s,"sem_destroy")

	for(int i=0; i<LETTERS; i++){
		printf("Frequenza %c: %d\n", i+65, counter[i]);
	}

	exit(EXIT_SUCCESS);
}
