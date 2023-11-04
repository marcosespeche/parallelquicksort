#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <MPI.h>

int myId, nproc;
MPI_Status status;
int a = 0;
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

    for (a = indInic; a <= indFinal - 1; a++)
    {

        if (vect[a] < pivote)
        {
            iswap++;
            swap(&vect[iswap], &vect[a]);
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
    int i = 0, j = 0, v = 0;
    if (myId == 0) // codigo master
    {
        // leer el arreglo
        int vect[dim];
        for (i = 0; i < dim; i++)
        {
            vect[i] = atoi(argv[i + 1]);
        }

        int refindex = 0; // indice de referencia para poder recorrer el arreglo total

        for (i = nworkers; i >= 1; i--) // enviar porciones de arreglo a los workers, primero mandamos la porcion mas grande
        {
            int auxvect[dimworker];
            for (j = 0; j < dimworker; j++)
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

        for (v = 0; v < cantvueltas; v++)
        {

            for (i = 0; i < cantfilas; i++) // recibir elementos de los workers
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
            for (i = 0; i < (2 * proxfilas); i += 2) // enviar elementos a los workers, ACA PUEDE HABER ERROR CON PROXFILAS
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
        int cantfilas = nworkers;
        int cantcolumnas = dimworker;
        // poner variables para que los workers sepan cuanto van a recibir y cuantas veces van a recibir
        // esas cosas las pueden calcular, ya saben la cantvueltas y nworkers
        int vectworker[dimworker];

        MPI_Recv(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);

        quicksort(vectworker, 0, (dimworker - 1));

        MPI_Send(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD);
        // malloc inicial
        int *fila1 = (int *)malloc(cantcolumnas * sizeof(int));
        int *fila2 = (int *)malloc(cantcolumnas * sizeof(int));
        int *filaresultado = (int *)malloc(2 * cantcolumnas * sizeof(int));
        for (v = 0; v < cantvueltas; ++)
        {
            // recibir las 2 filas
            MPI_Recv(fila1, cantcolumnas, MPI_INT, 0, MPI_ANY_TAG, &status);
            MPI_Recv(fila2, cantcolumnas, MPI_INT, 0, MPI_ANY_TAG, &status);
            int f1index = 0;
            int f2index = 0;
            for (i = 0; i < (2 * cantcolumnas); i++) // hacer la combinacion binaria
            {
                if (f1index == (cantcolumnas - 1))
                {
                    for (j = i; j < (2 * cantcolumnas); j++)
                    {
                        filaresultado[j] = fila2[f2index];
                        f2index++;
                    }
                    break;
                }

                if (f2index == (cantcolumnas - 1))
                {
                    for (j = i; j < (2 * cantcolumnas); j++)
                    {
                        filaresultado[j] = fila1[f1index];
                        f1index++;
                    }
                    break;
                }

                if (fila1[f1index] <= fila2[f2index])
                {
                    filaresultado[i] = fila1[f1index];
                    f1index++;
                }
                else
                {
                    filaresultado[i] = fila2[f2index];
                    f2index++;
                }
            }
            // enviar resultado al master
            MPI_Isend(filaresultado, (2 * cantcolumnas), MPI_INT, 0, myId);
            // recalcular estructuras para el proximo bucle
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
            cantfilas = proxfilas;
            cantcolumnas = 2 * cantcolumnas;
            fila1 = (int *)realloc(fila1, cantcolumnas * sizeof(int));
            fila2 = (int *)realloc(fila2, cantcolumnas * sizeof(int));
            filaresultado = (int *)realloc(filaresultado, 2 * cantcolumnas * sizeof(int));
        }
    }
}
