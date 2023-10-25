#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <MPI.h>

int myId, nproc;
MPI_Status status;
void swap(int *a, int *b)
{
    int temporal = *a;
    *a = *b;
    *b = temporal;
}

int partition(int vect[], int indInic, int indFinal)
{

    int iswap = indInic - 1;
    int pivote = vect[indFinal];

    for (int j = indInic; j <= indFinal - 1; j++)
    {

        if (vect[j] < pivote)
        {
            iswap++;
            swap(&vect[iswap], &vect[j]);
        }
    }
    swap(&vect[iswap + 1], &vect[indFinal]);
    return iswap + 1;
}

void quicksort(int vect[], int indInic, int indFinal)
{

    if (indInic < indFinal)
    {

        int pi = partition(vect, indInic, indFinal);

        quicksort(vect, indInic, pi - 1);
        quicksort(vect, pi + 1, indFinal);
    }
}

void main(int argc, char **argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    int nworkers = nproc - 1;                              // nro workers
    int dimworker = (int)(dim / nworkers);                 // cant de elem para cada worker
    int dimbigworker = dim - (dimworker * (nworkers - 1)); // cant de elem para big worker
    if (myId == 0)
    {
        // leer el arreglo
        int dim = argc - 1;
        int vect[dim];
        for (int i = 0; i < dim; i++)
        {
            vect[i] = atoi(argv[i + 1]);
        }
        printf("Arreglo Inicial:\n");
        for (int i = 0; i < dim; i++)
        {
            printf(" %d ", vect[i]);
        }
        // dividir el arreglo
        // 130/9=14

        int refindex = 0;

        for (int i = 1; i < nproc; i++)
        {

            // Codigo para enviar al worker que labura mas
            if (i == nproc - 1)
            {
                int auxvect[dimbigworker];
                for (int j = 0; j < dimbigworker; j++)
                {
                    auxvect[j] = vect[refindex];
                    refindex++;
                }
                MPI_Isend(&auxvect, dimbigworker, MPI_INT, i, i, MPI_COMM_WORLD);
            }
            // Codigo para el resto de workers
            else
            {
                int auxvect[dimworker];
                for (int j = 0; j < dimworker; j++)
                {
                    auxvect[j] = vect[refindex];
                    refindex++;
                }
                MPI_Isend(&auxvect, dimworker, MPI_INT, i, i, MPI_COMM_WORLD);
            }
        }
        // reconstruir el arreglo pero ordenado xd
    }
    else if (myId < nproc) // codigo worker normal
    {
        int vectworker[dimworker];

        MPI_Recv(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);
    }
    else if (myId == nproc) // codigo big worker
    {
        int vectworker[dimbigworker];
        MPI_Recv(&vectworker, dimbigworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);
    }

    quicksort(vect, 0, dim - 1);

    printf("\nArreglo sorteado:\n");

    for (int i = 0; i < dim; i++)
    {
        printf(" %d ", vect[i]);
    }

    end = clock();
    tmpejec = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("\nPrograma terminado en %fs\n", tmpejec);
}