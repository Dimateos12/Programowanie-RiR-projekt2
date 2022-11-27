//
// Created by mateusz on 27.11.22.
//
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <sys/time.h>
/* Przyjmujemy, ze proces 0 to proces Root, który rozdziela wiersze
i kolumny macierzy B, C pomiędzy procesy robocze Slave wykonujące
obliczenia cząstkowe mnożenia macierzy. Procesy Slave zwracają wyniki
do procesu Root, z których składa macierz wynikową A */
//Liczba wierszy i kolumn w macierzach A, B, C
#define N 40
MPI_Status status;
double A[N][N],B[N][N],C[N][N];
int main(int argc, char **argv) {
    int processCount, processId, slaveTaskCount, source, dest, rows, offset;
    struct timeval start, stop;
//Inicjalizacja srodowiska MPI
    MPI_Init(&argc, &argv);
//Uzyskanie numeru aktualnego procesu
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
//Uzyskanie liczby wszystkich procesów
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);
    slaveTaskCount = processCount - 1;
//Przyjęto, ze proces 0 Root (Master) - poniżej jego kod
    if (processId == 0) {
        double start = MPI_Wtime();
// Inicjalizacja macierzy A i B
        srand ( time(NULL) );
        for (int i = 0; i<N; i++) {
            for (int j = 0; j<N; j++) {
                A[i][j]= rand()%10;
                B[i][j]= rand()%10;
            }
        }
//Wypisanie zawartości macierzy A i B
        printf("\n Mnozenie macierzy za pomoca MPI \n");
        printf("\nMacierz A\n\n");
        for (int i = 0; i<N; i++) {
            for (int j = 0; j<N; j++) {
                printf("%.0f\t", A[i][j]);
            }
            printf("\n");
        }
        printf("\nMacierz B\n\n");
        for (int i = 0; i<N; i++) {
            for (int j = 0; j<N; j++) {
                printf("%.0f\t", B[i][j]);
            }
            printf("\n");
        }
        //Okreslene liczby wierzy macierzy A, która zostanie wysłana o każdego
//z procesów Slave
        rows = N/slaveTaskCount;
//Zmienna offset określa aktualny pierwszy z wierszy do wysłania do
//aktualnego procesu Slave
        offset = 0;
// Przygotowujemy wiersze i kolumny do wysłania do kolejnych procesów
//Slave o Id od 1 do slaveTaskCount, zostaną one przesłane wiadomością
//z tag 1
        for (dest=1; dest <= slaveTaskCount; dest++)
        {
//Przesylamy offset wzledem wiersza 0
            MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
//Ile wierszy przesylamy
            MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
//przesylamy wiersze macierzy A do procesow Slave
            MPI_Send(&A[offset][0], rows*N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
//Przesylamy kolumny macierzy B do procesow Slave
            MPI_Send(&B, N*N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
//Modyfikujemy aktualną wartość zmiennej offset
            offset = offset + rows;
        }
        //Proces Root czeka aż procesy Slave obliczą cząstkowe iloczyny wierszy
//i kolumn z macierzy A i B, wyniki zostaną odebrane z wiadomościach
// z tag 2
        for (int i = 1; i <= slaveTaskCount; i++)
        {
            source = i;
//Proces Root otrzymuje offset od aktualnego procesu Slave
            MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
//Proces Root otrzymuje liczbę wierszy, którą otrzyma od aktualnego
//procesu Slave
            MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
//Otrzymanie danych cząstkowych z mnożenia A*B do C
            MPI_Recv(&C[offset][0], rows*N, MPI_DOUBLE, source, 2
                    , MPI_COMM_WORLD, &status);
        }
//Wypisanie macierzy wynikowej C
        printf("\nWynikowa macierz C = A * B:\n\n");
        for (int i = 0; i<N; i++) {
            for (int j = 0; j<N; j++)
                printf("%.0f\t", C[i][j]);
            printf ("\n");
        }
        printf ("\n");
        double end = MPI_Wtime();
        printf("Czas obliczen %f",end - start);
    } //
    if (processId > 0) {
// Podanie procesom Slave numeru procesu Root
        source = 0;
//Procesy Slave czekają na wiadomości z tag 1 od procesu Root
//Każdy z procesów Slave wykonuje oddzielnie następujący kod
//Procesy Slave otrzymują wartość offsetu od procesu Root
        MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
// Procesy Slave otrzymują liczbę wierszy, która zostanie przesłana od
// procesu Root
        MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
// Procesy Slave otrzymują fragmenty macierzy A od procesu Root
        MPI_Recv(&A, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
// Procesy Slave otrzymują fragmenty macierzy B od procesu Root
        MPI_Recv(&B, N*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
// Mnożenie macierz y
        for (int k = 0; k<N; k++) {
            for (int i = 0; i<rows; i++) {
//
                C[i][k] = 0.0;
// Element A[i][j] mnożony przez B[j][k]
                for (int j = 0; j<N; j++)
                    C[i][k] = C[i][k] + A[i][j] * B[j][k];
            }
        }
//Offset zostanie wysłany do Root
        MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
//Liczba wierszy zostanie wysłana do Root
        MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
//Wyznaczony fragment macierzy C zostanie wysłany do Root
        MPI_Send(&C, rows*N, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}