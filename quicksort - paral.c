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
    int dim = argc - 1;                    // cantidad de nros del arreglo
    int nworkers = nproc - 1;              // nro workers
    int dimworker = (int)(dim / nworkers); // cant de elem para cada worker
    double resultado = ceil((log(nworkers) / log(2)));
    int cantvueltas = (int)resultado;

    if (myId == 0)
    {
        // leer el arreglo
        int vect[dim];
        for (int i = 0; i < dim; i++)
        {
            vect[i] = atoi(argv[i + 1]);
        }

        int refindex = 0; // indice de referencia para poder recorrer el arreglo total

        for (int i = nworkers; i >= 1; i--) // enviar porciones de arreglo a los workers, primero mandamos la porcion mas grande
        {
            int auxvect[dimworker];
            for (int j = 0; j < dimworker; j++)
            {
                auxvect[j] = vect[refindex];
                refindex++;
            }
            MPI_Isend(&auxvect, dimworker, MPI_INT, i, i, MPI_COMM_WORLD);
        }
        // reconstruir el arreglo, ordenandolo
        // la reconstruccion del arreglo se hara en n vueltas, que se calculan al principio en base a la cant de workers totales
        // ya que esa cant de workers totales es la cantidad de filas desde la cual debemos partir, hasta llegar a 1
        int cantfilas = nworkers;
        int cantcolumnas = dimworker;

        int *matriz = (int *)malloc(cantfilas * cantcolumnas * sizeof(int));

        for (int v = 0; v < cantvueltas; v++)
        {

            for (int i = 0; i < cantfilas; i++) // recibir elementos de los workers
            {
                int offset = i * cantcolumnas;
                MPI_Recv(matriz[offset], cantcolumnas, MPI_INT, (i + 1), (i + 1), MPI_COMM_WORLD, &status);
            }

            int proxfilas = 0;
            if (cantfilas % 2 == 0)
            {
                proxfilas = (cantfilas / 2);
            }
            else
            {
                proxfilas = (int)(cantfilas / 2);
                proxfilas += 1;
            }

            // queremos enviar elementos a los workers, las filas se envían de a pares
            // para poder calcular el offset de cada una, incrementamos i en 2 unidades por cada iteración
            // por eso multiplicamos a proxfilas por 2, para que haya una cantidad de iteraciones = proxfilas
            int workerid = 1;
            for (int i = 0; i < (2 * proxfilas); i += 2) // enviar elementos a los workers, ACA PUEDE HABER ERROR CON PROXFILAS
            {
                int offset = i * cantcolumnas;
                MPI_Isend(matriz[offset], cantcolumnas, MPI_INT, workerid, workerid, MPI_COMM_WORLD);
                MPI_Isend(matriz[offset + cantcolumnas], cantcolumnas, MPI_INT, workerid, workerid, MPI_COMM_WORLD);
                workerid++;
            }
            cantfilas = proxfilas;
            cantcolumnas = cantcolumnas * 2;
            matriz = (int *)realloc(matriz, cantfilas * cantcolumnas * sizeof(int));
        }
        // codigo para mandar el arreglo ordenado a un archivo
    }
    else // codigo worker
    {
        // poner variables para que los workers sepan cuanto van a recibir y cuantas veces van a recibir
        // esas cosas las pueden calcular, ya saben la cantvueltas y nworkers
        int vectworker[dimworker];

        MPI_Recv(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);

        quicksort(vectworker, 0, (dimworker - 1));

        MPI_Send(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD);
        // malloc inicial
        for (int v = 0; v < cantvueltas; ++)
        {
            // recibir
            // combinacion
            // mandar
            // calcular nuevos tamaños de fila y col
            // realloc
        }
    }

    // Arreglo ya ordenado
    printf("\nArreglo sorteado:\n");

    for (int i = 0; i < dim; i++)
    {
        printf(" %d ", vect[i]);
    }

    end = clock();
    tmpejec = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("\nPrograma terminado en %fs\n", tmpejec);
}
