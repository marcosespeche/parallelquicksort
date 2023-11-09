#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

int myId, nproc;

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
// el nro de elementos debe ser mulitplo de la cantidad de workers
// la cantidad de workers debe ser potencia de 2
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Status status;
    MPI_Request request;
    int dim = atoi(argv[1]);               // cantidad de numeros que vamos a leer
    int nworkers = nproc - 1;              // nro workers
    int dimworker = (int)(dim / nworkers); // cant de elem para cada worker
    double resultado = ceil((log(nworkers) / log(2)));
    int cantvueltas = (int)resultado;
    int i = 0, j = 0, v = 0;

    if (myId == 0) // codigo master
    {
        // leer el arreglo
        int vect[dim];
        char *nomarchv = argv[2];
        FILE *fileptr;

        if (!(fileptr = fopen("datos.txt", "r")))
        {
            printf("No se pudo abrir el archivo.\n");
            return 1;
        }

        for (i = 0; i < dim; i++)
        {
            if (fscanf(fileptr, "%d", &vect[i]) != 1)
            {
                printf("Error al leer el archivo.\n");
                return 1;
            }
        }
        fclose(fileptr);
        // Iniciamos el cronometro
        double tiempoinicio = MPI_Wtime();

        int refindex = 0; // indice de referencia para poder recorrer el arreglo total

        for (i = 0; i < nworkers; i++) // enviar porciones de arreglo a los workers, primero mandamos la porcion mas grande
        {
            int auxvect[dimworker];
            for (j = 0; j < dimworker; j++)
            {
                auxvect[j] = vect[refindex];
                refindex++;
            }
            MPI_Isend(&auxvect, dimworker, MPI_INT, (i + 1), (i + 1), MPI_COMM_WORLD, &request);
        }
        int cantfilas = nworkers;
        int cantcolumnas = dimworker;

        int *matriz = (int *)malloc(cantfilas * cantcolumnas * sizeof(int));

        for (v = 0; v < cantvueltas; v++)
        {

            for (i = 0; i < cantfilas; i++) // recibir elementos de los workers
            {
                int offset = i * cantcolumnas;
                MPI_Recv(&matriz[offset], cantcolumnas, MPI_INT, (i + 1), (i + 1), MPI_COMM_WORLD, &status);
            }

            int proxfilas = (cantfilas / 2);

            int cantworkers = (int)(cantfilas / 2);

            int workerid = 1;
            int duplafilas = 0;               // esto se utiliza para identificar cada dupla de filas que se enviará
            for (i = 0; i < cantworkers; i++) // iteramos por cada worker que debe recibir una dupla de filas para combinarlas
            {
                int offset = duplafilas * cantcolumnas;
                MPI_Isend(&matriz[offset], cantcolumnas, MPI_INT, workerid, workerid, MPI_COMM_WORLD, &request);
                MPI_Isend(&matriz[offset + cantcolumnas], cantcolumnas, MPI_INT, workerid, workerid, MPI_COMM_WORLD, &request);
                workerid++;
                duplafilas = duplafilas + 2;
            }
            cantfilas = proxfilas;
            cantcolumnas = cantcolumnas * 2;
            matriz = (int *)realloc(matriz, cantfilas * cantcolumnas * sizeof(int));
        }

        MPI_Recv(&matriz[0], cantcolumnas, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        double tiempofinal = MPI_Wtime();
        double tiempoejecucion = tiempofinal - tiempoinicio;
        // codigo para mandar el arreglo ordenado a un archivo
        if (!(fileptr = fopen(nomarchv, "w")))
        {
            printf("No se pudo abrir el archivo.\n");
            return 1;
        }

        fprintf(fileptr, "Cantidad de numeros: %d\n", dim);
        fprintf(fileptr, "Tiempo de ejecucion: %fs\n", tiempoejecucion);
        for (i = 0; i < dim; i++)
        {
            fprintf(fileptr, "%d ", matriz[i]);
        }
        fclose(fileptr);
        free(matriz);
        MPI_Finalize();
    }
    else // codigo worker
    {
        int cantfilas = nworkers;
        int cantcolumnas = dimworker;
        int vectworker[dimworker];

        MPI_Recv(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);

        quicksort(vectworker, 0, (dimworker - 1));

        MPI_Send(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD);
        // malloc inicial
        int *fila1 = (int *)malloc(cantcolumnas * sizeof(int));
        if (fila1 == NULL)
        {
            printf("Error al asignar fila1");
        }
        int *fila2 = (int *)malloc(cantcolumnas * sizeof(int));
        if (fila2 == NULL)
        {
            printf("Error al asignar fila2");
        }
        int *filaresultado = (int *)malloc(2 * cantcolumnas * sizeof(int));
        if (filaresultado == NULL)
        {
            printf("Error al asignar filaresultado");
        }
        for (v = 0; v < cantvueltas; v++)
        {
            int limiteworker = nworkers;
            for (h = 0; h <= v; h++)
            {
                limiteworker = limiteworker / 2;
            }
            if (myId > limiteworker)
            {
                break;
            }
            // recibir las 2 filas
            MPI_Recv(fila1, cantcolumnas, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(fila2, cantcolumnas, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
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
            MPI_Isend(filaresultado, (2 * cantcolumnas), MPI_INT, 0, myId, MPI_COMM_WORLD, &request);
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
            // realloc
            fila1 = (int *)realloc(fila1, cantcolumnas * sizeof(int));
            if (fila1 == NULL)
            {
                printf("Error realloc fila1 vuelta %d", v);
            }
            fila2 = (int *)realloc(fila2, cantcolumnas * sizeof(int));
            if (fila2 == NULL)
            {
                printf("Error realloc fila2 vuelta %d", v);
            }
            filaresultado = (int *)realloc(filaresultado, 2 * cantcolumnas * sizeof(int));
            if (filaresultado == NULL)
            {
                printf("Error realloc filaresultado vuelta %d", v);
            }
        } // end for vueltas
        free(fila1);
        free(fila2);
        free(filaresultado);
        MPI_Finalize();
    }
    return 0;
}
