#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define N 5  // Broj filozofa

int fil_br[N] = {0, 1, 2, 3, 4};  // Indeksi filozofa
sem_t sem[N];  // Semafori za svakog filozofa
sem_t mutex;  // Mutex za kritičnu sekciju
int state[N];  // Stanja filozofa (1: gladan, 2: jede, 3: misli)

// Funkcija koja testira može li filozof br uzeti vilice i jesti
void test(int br) {
    if (state[br] == 1 && state[(br + 4) % N] != 2 && state[(br + 1) % N] != 2) {
        state[br] = 2;  // Filozof br jede
        printf("filozof %d uzima vilice i jede\n", br + 1);
        fflush(stdout);
        sleep(1);  // Simulacija vremena jedenja
        sem_post(&sem[br]);  // Oslobađanje semafora za filozofa br
    }
}

// Funkcija kojom filozof uzima vilice
void uzima_vilicu(int br) {
    sem_wait(&mutex);  // Ulazak u kritičnu sekciju
    state[br] = 1;  // Filozof br je gladan
    printf("filozof %d je gladan\n", br + 1);
    test(br);  // Testiranje može li filozof jesti
    sem_post(&mutex);  // Izlazak iz kritične sekcije
    sem_wait(&sem[br]);  // Čekanje da semafor bude oslobođen
    fflush(stdout);
    sleep(1);  // Simulacija čekanja
}

// Funkcija kojom filozof vraća vilice
void vrati_vilice(int br) {
    sem_wait(&mutex);  // Ulazak u kritičnu sekciju
    printf("filozof %d ostavlja vilice i ide misliti\n", br + 1);
    fflush(stdout);
    sleep(1);  // Simulacija vremena razmišljanja
    state[br] = 3;  // Filozof br misli
    test((br + 4) % N);  // Testiranje može li lijevi susjed jesti
    test((br + 1) % N);  // Testiranje može li desni susjed jesti
    sem_post(&mutex);  // Izlazak iz kritične sekcije
}

// Funkcija koju izvršava svaki filozof u dretvi
void* filozof(void* pdr) {
    int temp = *((int *)pdr);  // Dobivanje indeksa filozofa
    while (1) {
        sleep(2);  // Simulacija vremena razmišljanja
        uzima_vilicu(temp);  // Uzima vilice i jede
        sleep(0);  // Simulacija vremena jedenja
        vrati_vilice(temp);  // Vraća vilice i misli
    }
}

void main() {
    pthread_t thread_id[N];  // Polje za dretve filozofa
    sem_init(&mutex, 0, 1);  // Inicijalizacija mutexa
    int br;

    // Inicijalizacija semafora i stanja filozofa
    for (int i = 0; i < N; i++) {
        sem_init(&sem[i], 0, 0);
        state[i] = 3;  // Svi filozofi počinju s razmišljanjem
    }

    // Stvaranje dretvi za svakog filozofa
    for (int i = 0; i < N; i++) {
        int *br = (int *)malloc(sizeof(int));
        *br = i;
        pthread_create(&thread_id[i], NULL, filozof, br);
        printf("Filozof %d misli\n", i + 1);
    }

    // Čekanje da sve dretve završe (nikad se neće dogoditi jer filozofi neprekidno misle i jedu)
    for (int i = 0; i < N; i++) {
        pthread_join(thread_id[i], NULL);
    }
}

