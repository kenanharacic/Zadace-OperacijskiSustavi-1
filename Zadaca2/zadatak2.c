#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

// Globalni pokazivači i ID-ovi za zajedničku memoriju
int *PRAVO, *ZASTAVICA, pravoId, zastavicaId;

// Funkcija za ulazak u kritični odsječak
void udi_u_kriticni_odsjecak(int i, int j){
    ZASTAVICA[i] = 1;  // Postavljanje zastavice za proces i
    while(!(ZASTAVICA[j] == 0)){  // Čekanje dok je zastavica za proces j postavljena
        if(*PRAVO == j){  // Provjera je li pravo na kritični odsječak dodijeljeno procesu j
            ZASTAVICA[i] = 0;  // Resetiranje zastavice za proces i
            while(*PRAVO == j){  // Čekanje dok pravo nije prebačeno na proces i
            }
            ZASTAVICA[i] = 1;  // Ponovno postavljanje zastavice za proces i
        }
    }
}

// Funkcija za izlazak iz kritičnog odsječka
void izadji_iz_kriticnog_odsjecka(int i, int j){
    ZASTAVICA[i] = 0;  // Resetiranje zastavice za proces i
    *PRAVO = j;  // Dodjeljivanje prava procesu j
}

// Funkcija koja simulira rad procesa
void proc(int i, int j){
    int k, m;
    for(k = 1; k < 5; k++){  // Petlja koja se izvršava 4 puta
        udi_u_kriticni_odsjecak(i, j);  // Ulazak u kritični odsječak
        for(m = 1; m < 5; m++){  // Unutarnja petlja koja se izvršava 4 puta
            printf("i: %d, k: %d, m: %d\n", i, k, m);  // Ispis trenutnog stanja
            sleep(1);  // Pauza od 1 sekunde
        }
        izadji_iz_kriticnog_odsjecka(i, j);  // Izlazak iz kritičnog odsječka
    }
}

// Funkcija za brisanje zajedničke memorije i izlazak iz programa
void brisi(){
    shmdt(PRAVO);  // Odspajanje zajedničke memorije za PRAVO
    shmdt(ZASTAVICA);  // Odspajanje zajedničke memorije za ZASTAVICA

    shmctl(pravoId, IPC_RMID, NULL);  // Brisanje segmenta zajedničke memorije za PRAVO
    shmctl(zastavicaId, IPC_RMID, NULL);  // Brisanje segmenta zajedničke memorije za ZASTAVICA

    exit(0);  // Prekid programa
}

void main(){
    // Kreiranje segmenta zajedničke memorije za PRAVO
    pravoId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0660);
    // Kreiranje segmenta zajedničke memorije za ZASTAVICA
    zastavicaId = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | 0660);
    // Spajanje segmenta zajedničke memorije za PRAVO na adresni prostor procesa
    PRAVO = (int*)shmat(pravoId, NULL, 0);
    // Spajanje segmenta zajedničke memorije za ZASTAVICA na adresni prostor procesa
    ZASTAVICA = (int*)shmat(zastavicaId, NULL, 0);

    // Stvaranje prvog procesa
    if(fork() == 0){
        proc(1, 0);  // Izvršavanje funkcije proc s argumentima 1 i 0
    }

    // Stvaranje drugog procesa
    if(fork() == 0){
        proc(0, 1);  // Izvršavanje funkcije proc s argumentima 0 i 1
    }

    // Postavljanje rukovatelja za signal SIGINT
    signal(SIGINT, brisi);

    // Čekanje završetka oba procesa
    wait(NULL);
    wait(NULL);

    // Brisanje zajedničke memorije i izlazak iz programa
    brisi();
}

