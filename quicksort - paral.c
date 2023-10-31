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
    int dim = argc - 1;                                    // cantidad de nros del arreglo
    int nworkers = nproc - 1;                              // nro workers
    int dimworker = (int)(dim / nworkers);                 // cant de elem para cada worker
    int dimbigworker = dim - (dimworker * (nworkers - 1)); // cant de elem para big worker
    // int color;
    // MPI_Comm commWorkersNormales;
    // comunicador para workers con igual dimensión
    // if(myId<nworkers) color=1;
    // MPI_Comm_split(MPI_COMM_WORLD, color, myId, &commWorkersNormales);
    double resultado = ceil((log(nworkers) / log(2)));
    int cantvueltas = (int)resultado;
    // lo anterior seria reemplazado por ceil
    int vectRecibidos[nworkers][dimbigworker]; // filas = cant workers, columnas = maxdim de cada sub arreglo
    // definir las matrices que se usarán en cada vuelta, tenemos la cantidad de vueltas y la cantidad de filas de c/u
    int *ptrmatrx[cantvueltas];
    for (int i = 0; i < cantvueltas; i++)
    {
        int matriz[nworkers][dimbigworker];
        ptrmatrx[i] = &matriz;
    }

    if (myId == 0)
    {
        // leer el arreglo
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

        int refindex = 0; // indice de referencia para poder recorrer el arreglo total

        for (int i = nworkers; i >= 1; i--) // enviar porciones de arreglo a los workers, primero mandamos la porcion mas grande
        {

            // Codigo para enviar al worker que labura mas
            if (i == nworkers)
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
        // la reconstruccion del arreglo se hara en n vueltas, que se calculan al principio en base a la cant de workers totales
        // ya que esa cant de workers totales es la cantidad de filas desde la cual debemos partir, hasta llegar a 1
        for (int v = 0; v < cantvueltas; v++)
        {
            // ir definiendo la matriz con el nro de filas variable, usando variables que esten afuera del for de v
            //  usar malloc, y al final de la iteración, usar free
            for (int i = 1; i < nworkers; i++) // recibir cositas
            {
                MPI_Recv(&vectRecibidos[i - 1][0], dimworker, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            }
            MPI_Recv(&vectRecibidos[nworkers - 1][0], dimbigworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD, &status);

            if (!(nworkers % 2)) // caso en el que hay cantidad par de workers, ojo con la condicion(revisar)
            {

                for (int i = (nworkers / 2); i >= 1, i--)
                {

                    if (i == (nworkers / 2)) // esto enviará al big worker
                    {
                        MPI_Isend(&vectRecibidos[((2 * i) - 2)][0], dimworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD);
                        MPI_Isend(&vectRecibidos[((2 * i) - 1)][0], dimbigworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD); // aca viene a parar la fila del worker grande
                    }
                    else // envía al resto de los workers, no se usan todos
                    {
                        MPI_Isend(&vectRecibidos[((2 * i) - 1)][0], dimworker, MPI_INT, i + 1, i + 1, MPI_COMM_WORLD); // por que ese i+1?
                        MPI_Isend(&vectRecibidos[((2 * i) - 2)][0], dimworker, MPI_INT, i + 1, i + 1, MPI_COMM_WORLD);
                    }
                }
            }
            else
            {
                // si es impar se envían el arreglo del worker que más labura se une con la unión de las demás listas en el master
            }

            /*
            TODO pensar como utilizar matriz dinámica para recibir los datos de los nodos al hacer la unión de las listas ordenadas

            */
            // aca iria el free
        }
        // codigo para mandar el arreglo ordenado a un archivo
    }
    else if (myId < nworkers) // codigo worker normal
    {
        // poner variables para que los workers sepan cuanto van a recibir y cuantas veces van a recibir
        // esas cosas las pueden calcular, ya saben la cantvueltas y nworkers
        int vectworker[dimworker];

        MPI_Recv(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);

        quicksort(vectworker, 0, (dimworker - 1));

        MPI_Send(&vectworker, dimworker, MPI_INT, 0, myId, MPI_COMM_WORLD);
    }
    else if (myId == nworkers) // codigo big worker
    {
        int vectworker[dimbigworker];

        MPI_Recv(&vectworker, dimbigworker, MPI_INT, 0, myId, MPI_COMM_WORLD, &status);

        quicksort(vectworker, 0, (dimbigworker - 1));

        MPI_Send(&vectworker, dimbigworker, MPI_INT, 0, myId, MPI_COMM_WORLD);
    }

    printf("\nArreglo sorteado:\n");

    for (int i = 0; i < dim; i++)
    {
        printf(" %d ", vect[i]);
    }

    end = clock();
    tmpejec = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("\nPrograma terminado en %fs\n", tmpejec);
}
