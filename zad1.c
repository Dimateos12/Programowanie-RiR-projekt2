#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

double func(double x) {
    return 4*x - 6*x + 5;
}

double licz(double a, double b, int iloscTrapezow) {
    double suma = 0;
    double h = (b - a) / iloscTrapezow;
    suma += (func(a) + func(b)) / 2;
    for (int i = 1; i < iloscTrapezow; i++) {
        suma += func(a + i * h);
    }

    return suma * h;
}

int main1() {
    int p;
    printf("Podaj ilosc procesow: ");
    scanf("%d", &p);

    for (int i = 0; i < p; i++) {
        if (fork() == 0) {
            srand(time(NULL) ^ (getpid() << 16));

            double a = rand() % 20;
            double b = rand() % 20;

            while (b <= a) {
                b = rand() % 20;
            }

            int iloscTrapezow = (rand() % (20 - 1 + 1)) + 1;


            printf("a: %f, b: %f, ilosc trapezow: %d\n", a, b, iloscTrapezow);
            printf("wartosc calki dla powyzszych danych wynosi: %f\n\n", licz(a, b, iloscTrapezow));

            exit(0);
        }

    }



    return 0;

}