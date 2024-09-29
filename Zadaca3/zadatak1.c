#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

// Globalni pokazivači i varijable za broj stolova i dretvi
int *ULAZ, *BROJ, brSt, brDr, *REZ;

// Funkcija koja vraća maksimalni broj iz niza BROJ
int max(){
    int najv = BROJ[0];
    for(int i = 0; i < brSt; i++){
        if(BROJ[i] > najv){
            najv = BROJ[i];
        }
    }
    return najv;
}

// Funkcija koja provjerava jesu li svi stolovi zauzeti
int sve_zauzeto(){
    for(int i = 0; i < brSt; i++){
        if(REZ[i] == -1){
            return 0;
        }
    }
    return 1;
}

// Funkcija za ulazak u kritični odsječak
void udji_u_kriticni_odsjecak(int i){
    ULAZ[i] = 1;  // Postavljanje ulaza za proces i
    BROJ[i] = max() + 1;  // Postavljanje broja za proces i
    ULAZ[i] = 0;  // Resetiranje ulaza za proces i

    for(int j = 0; j < brSt; j++){
        while(ULAZ[j] != 0) {}  // Čekanje dok proces j ne izađe iz ulaza
        while(BROJ[j] != 0 && (BROJ[j] < BROJ[i] || (BROJ[j] == BROJ[i] && j < i))) {}  // Čekanje dok proces j ne izađe iz kritičnog odsječka
    }
}

// Funkcija za izlazak iz kritičnog odsječka
void izadji_iz_kriticnog_odsjecka(int i){
    BROJ[i] = 0;  // Resetiranje broja za proces i
}

// Funkcija koja provjerava stanje stola i pokušava ga rezervirati
void *provjeri_stol(void *pdr){
    if(sve_zauzeto()){
        pthread_exit("svi su stolovi zauzeti");  // Ako su svi stolovi zauzeti, dretva završava
    }
    int dr = *((int *)pdr);  // Dobivanje broja dretve

    int rand_tbl = rand() % brSt;  // Generiranje slučajnog broja stola
    printf("Dretva %d: pokusavam rez stol %d\n", dr + 1, rand_tbl + 1);
    udji_u_kriticni_odsjecak(rand_tbl);  // Ulazak u kritični odsječak za taj stol
    char stanje[brSt + 1];
    for(int i = 0; i < brSt; i++){
        stanje[i] = (REZ[i] == -1) ? '-' : REZ[i] + 1 + '0';  // Postavljanje stanja stolova
    }
    stanje[brSt] = '\0';
    if(REZ[rand_tbl] == -1){
        REZ[rand_tbl] = dr;  // Rezervacija stola
        for(int i = 0; i < brSt; i++){
            stanje[i] = (REZ[i] == -1) ? '-' : REZ[i] + 1 + '0';  // Ažuriranje stanja stolova
        }
        stanje[brSt] = '\0';
        printf("Dretva %d: rezerviram stol %d, stanje: %s\n", dr + 1, rand_tbl + 1, stanje);
    } else {
        printf("Dretva %d: neuspjela rezervacija stola %d, stanje: %s\n", dr + 1, rand_tbl + 1, stanje);
    }
    izadji_iz_kriticnog_odsjecka(rand_tbl);  // Izlazak iz kritičnog odsječka za taj stol
    pthread_exit(NULL);
}

void main(){
    printf("br dretvi: ");
    scanf("%d", &brDr);  // Unos broja dretvi
    printf("br st: ");
    scanf("%d", &brSt);  // Unos broja stolova
    fflush(stdin);

    pthread_t thread_id;

    // Alociranje memorije za zajedničke nizove
    REZ = (int*)malloc(sizeof(int) * brSt);
    BROJ = (int*)malloc(sizeof(int) * brSt);
    ULAZ = (int*)malloc(sizeof(int) * brSt);

    // Inicijalizacija zajedničkih nizova
    for(int i = 0; i < brSt; i++){
        REZ[i] = -1;
        BROJ[i] = 0;
        ULAZ[i] = 0;
    }

    // Stvaranje dretvi sve dok nisu svi stolovi zauzeti
    while(!sve_zauzeto()){
        for(int i = 0; i < brDr; i++){
            int *pdr = (int *)malloc(sizeof(int));
            *pdr = i;
            pthread_create(&thread_id, NULL, provjeri_stol, pdr);
        }
    }

    // Čekanje završetka svih dretvi
    for(int i = 0; i < brDr; i++){
        pthread_join(thread_id, NULL);
    }

    // Oslobađanje alocirane memorije
    free(REZ);
    free(BROJ);
    free(ULAZ);
}
