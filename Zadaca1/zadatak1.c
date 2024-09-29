
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// Globalna varijabla koja označava treba li program završiti
int zavrsi = 0;
// Globalna varijabla koja prati trenutni status obrade
int statusBr;

// Funkcija koja se poziva kad program primi SIGINT signal (npr. pritisak na Ctrl + C)
void sigint_funkcija (int sig) {
    // Ispisuje izbornik i traži unos od korisnika
    printf("\nOdaberite radnju:\n1) Prekid programa (bez dovršavanja radnje)\n2) Završi program\n3) Ispiši trenutni status\n4) Nastavi\n");
    int input;
    // Prima unos od korisnika
    scanf("%d", &input);
    
    // Switch-case blok koji određuje što će se dogoditi ovisno o unosu korisnika
    switch(input) {
        case 1:
            // Opcija 1: Trenutno prekida program
            printf("\nPrekid programa\n");
            exit(0);  // Završava program
            break;
        case 2:
            // Opcija 2: Zatvara program slanjem SIGTERM signala
            kill(getpid(), SIGTERM);
            break;
        case 3:
            // Opcija 3: Prikazuje trenutni status slanjem SIGUSR1 signala
            kill(getpid(), SIGUSR1);
            break;
        default:
            // Ako se odabere opcija 4 ili neka druga, program nastavlja s radom
            break;
    }
}

// Funkcija koja se poziva kad program primi SIGTERM signal
void sigterm_funkcija (int sig) {
    // Ispisuje poruku i označava da program treba završiti
    printf("\nSIGTERM primljen, završavanje programa\n");
    zavrsi = 1;  // Postavlja varijablu zavrsi na 1 kako bi se prekinula glavna petlja u main()
    exit(0);     // Završava program
}

// Funkcija koja se poziva kad program primi SIGUSR1 signal
void sigusr1_funkcija (int sig) {
    // Ispisuje trenutni status obrade, tj. vrijednost statusBr
    printf("\nTrenutni status: %d\n", statusBr);
}

// Funkcija za obradu podataka i ažuriranje statusa
void proces(FILE *status, FILE *obrada) {
    // Ako je statusBr 0, program učitava zadnju vrijednost iz obrada.txt
    if(statusBr == 0) {
        int obradaBr;
        // Čitanje do kraja datoteke obrada.txt
        while(!feof(obrada)) {
            fscanf(obrada, "%d", &obradaBr);
        }
        // Izračunava kvadratni korijen zadnje vrijednosti iz obrada.txt i postavlja ga kao statusBr
        statusBr = round(sqrt(obradaBr));
    }

    // Resetira datoteku status.txt i upisuje 0
    fseek(status, 0, SEEK_SET);
    fprintf(status, "%d", 0);
    fflush(status);  // Osigurava da je sadržaj upisan u datoteku
    fflush(obrada);  // Osigurava da su promjene u obrada.txt spremljene
    sleep(3);        // Program se pauzira 3 sekunde

    // Ažurira status.txt i obrada.txt s novim vrijednostima
    fseek(status, 0, SEEK_SET);
    fseek(obrada, 0, SEEK_END);  // Pomjera pokazivač na kraj datoteke obrada.txt
    statusBr++;                  // Povećava statusBr za 1
    fprintf(status, "%d", statusBr);             // Upisuje novi status u status.txt
    fprintf(obrada, "%d\n", statusBr*statusBr);  // Upisuje kvadrat nove vrijednosti statusBr u obrada.txt

    fflush(status);  // Osigurava da su sve promjene spremljene u datoteku
    fflush(obrada);  // Osigurava da je i obrada.txt ažuriran
    sleep(2);        // Program se pauzira 2 sekunde
}

// Glavna funkcija programa
void main() {
    // Deklaracija pokazivača za dvije datoteke: status.txt i obrada.txt
    FILE *status, *obrada;

    // Otvaranje datoteka
    status = fopen("status.txt", "r");  // status.txt se otvara samo za čitanje
    obrada = fopen("obrada.txt", "r+"); // obrada.txt se otvara za čitanje i pisanje

    // Učitava vrijednost statusBr iz status.txt
    fscanf(status, "%d", &statusBr);

    // Povezivanje signala s odgovarajućim funkcijama
    signal(SIGTERM, sigterm_funkcija);  // Obrada SIGTERM signala
    signal(SIGINT, sigint_funkcija);    // Obrada SIGINT signala
    signal(SIGUSR1, sigusr1_funkcija);  // Obrada SIGUSR1 signala

    // Glavna petlja programa koja se izvodi dok zavrsi nije postavljen na 1
    while(zavrsi == 0) {
        // Ponovno otvara status.txt u načinu za pisanje (resetira datoteku svaki put)
        status = freopen("status.txt", "w+", status);
        // Poziva funkciju za obradu podataka
        proces(status, obrada);
    }

    // Zatvara datoteke kad program završi
    fclose(status);
    fclose(obrada);
}
