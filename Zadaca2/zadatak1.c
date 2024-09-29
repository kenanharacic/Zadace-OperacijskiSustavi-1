#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <pthread.h>

// Veličina zajedničke memorije
#define SHARED_MEMORY_SIZE (0x6400)

int segment_Id;  // ID segmenta zajedničke memorije
int *sh_mem;     // Pokazivač na zajedničku memoriju

// Funkcija za rukovanje signalom SIGINT
void brisi(){
    printf("Signal primljen, gasi sve\n");
    shmdt(sh_mem);  // Odspajanje zajedničke memorije
    shmctl(segment_Id, IPC_RMID, NULL);  // Brisanje segmenta zajedničke memorije
    exit(0);  // Prekid programa
}

void main(){
    // Kreiranje segmenta zajedničke memorije
    segment_Id = shmget(IPC_PRIVATE, SHARED_MEMORY_SIZE, IPC_CREAT | 0660);
    // Spajanje segmenta zajedničke memorije na adresni prostor procesa
    sh_mem = (int*) shmat(segment_Id, NULL, 0);
    
    while(1){
        // Stvaranje novog procesa
        if(fork() == 0){
            // U dječjem procesu
            *sh_mem = 1;  // Postavljanje vrijednosti u zajedničkoj memoriji
            printf("%d\n", *sh_mem);  // Ispis vrijednosti
            sleep(3);  // Pauza od 3 sekunde
            printf("%d\n", *sh_mem);  // Ispis vrijednosti
            exit(0);  // Prekid dječjeg procesa
        }

        // U roditeljskom procesu
        signal(SIGINT, brisi);  // Postavljanje rukovatelja za signal SIGINT
        *sh_mem = 2;  // Postavljanje vrijednosti u zajedničkoj memoriji
        printf("%d\n", *sh_mem);  // Ispis vrijednosti
        sleep(3);  // Pauza od 3 sekunde
        printf("%d\n", *sh_mem);  // Ispis vrijednosti
        wait(NULL);  // Čekanje završetka dječjeg procesa
    }
}
