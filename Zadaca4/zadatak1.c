#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>

#define N 5          // Broj sjedala u vrtuljku
#define M 15         // Broj posjetitelja
#define SEMNAME_MJESTA "mjesta"  // Ime semafora

// Funkcija koju izvršava svaki posjetitelj
void posjetitelj(int i){
    sem_t* mjesta = sem_open(SEMNAME_MJESTA, 0);  // Otvaranje semafora

    while(1){
        sem_wait(mjesta);  // Čekanje na slobodno mjesto u vrtuljku
        printf("proces %d je usao u vrtuljak\n", i);
        sleep(3);  // Simulacija vožnje u vrtuljku
    }
}

// Funkcija koja simulira rad vrtuljka
void vrtuljak(){
    int br = 0;
    sem_t* mjesta = sem_open(SEMNAME_MJESTA, 0);  // Otvaranje semafora
    int *value = malloc(sizeof(int));  // Alociranje memorije za pohranu vrijednosti semafora

    while(1){
        do{
            sem_getvalue(mjesta, value);  // Dohvaćanje trenutne vrijednosti semafora
            printf("broj slobodnih mjesta: %d\n", *value);
            sleep(1);  // Pauza od 1 sekunde
        } while(*value != 0);  // Petlja dok sva mjesta nisu zauzeta

        sleep(1);  // Pauza prije pokretanja vrtuljka
        printf("vrtuljak se vrti %d\n", br++);
        sleep(3);  // Simulacija vrtnje vrtuljka
        printf("Vrtuljak se zaustavio\n");

        // Oslobađanje svih mjesta u vrtuljku
        for(int i = 0; i < N; i++){
            sem_post(mjesta);  // Oslobađanje jednog mjesta
            printf("Sjedalo br %d na vrtuljku se ispraznilo!\n", i + 1);
        }
    }
}

void main(){
    // Kreiranje i inicijalizacija semafora
    sem_t* mjesta = sem_open(SEMNAME_MJESTA, O_CREAT, 0644, N);

    // Inicijalizacija semafora na N (broj slobodnih mjesta)
    for(int i = 0; i < N; i++){
        sem_post(mjesta);
    }

    // Stvaranje procesa za vrtuljak
    if(fork() == 0){
        vrtuljak();
    }

    sleep(1);  // Pauza prije stvaranja posjetitelja

    // Stvaranje procesa za posjetitelje
    for(int i = 0; i < M; i++){
        if(fork() == 0){
            posjetitelj(i);
        }
    }

    // Čekanje završetka svih procesa
    for(int i = 0; i < M + 1; i++){
        wait(NULL);
    }

    // Uništavanje semafora
    sem_destroy(mjesta);
}

  
