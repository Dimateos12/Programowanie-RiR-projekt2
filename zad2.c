//
// Created by mateusz on 27.11.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

double PI(int N) {
    double suma = 0;
    double ulamek;
    for (int n = 1; n <= N; n++) {
        ulamek = pow(-1, n - 1) / (2 * n - 1);
        suma += ulamek;
    }

    return 4 * suma;
}

int main() {
    int p;
    printf("Podaj ilosc procesow: ");
    scanf("%d", &p);

    for (int i = 0; i < p; i++) {
        if (fork() == 0) {
            srand(time(NULL) ^ (getpid() << 16));
            int N = (rand() % (5000 - 100 + 1)) + 100;

            printf("Dla N = %d, przyblizona wartosc PI wynosi: %f\n", N, PI(N));
            exit(0);
        }

    }

    return 0;

}