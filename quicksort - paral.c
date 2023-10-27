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
    //int color;
    //MPI_Comm commWorkersNormales;
    //comunicador para workers con igual dimensión
    //if(myId<nworkers) color=1;
    //MPI_Comm_split(MPI_COMM_WORLD, color, myId, &commWorkersNormales);

    int vectRecibidos[nworkers][dimbigworker];


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

        int refindex = 0; // indice de referencia para poder recorrer el arreglo total

        for (int i = nworkers; i >= 1; i--)
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
        for(int i=1; i<nworkers; i++){
            MPI_Recv(&vectRecibidos[i-1][0], dimworker, MPI_INT, i, i, MPI_COMM_WORLD, &status);
        }
        MPI_Recv(&vectRecibidos[nworkers-1][0], dimbigworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD, &status);

        if(nworkers%2){

            for(int i=nworkers/2; i>=1, i--){
                
                if(i==(nworkers/2)){
                    MPI_Isend(&vectRecibidos[((2*i)-2)][0], dimworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD);
                    MPI_Isend(&vectRecibidos[((2*i)-1)][0], dimbigworker, MPI_INT, nworkers, nworkers, MPI_COMM_WORLD);
                }else{
                    MPI_Isend(&vectRecibidos[((2*i)-1)][0], dimworker, MPI_INT, i+1, i+1, MPI_COMM_WORLD);
                    MPI_Isend(&vectRecibidos[((2*i)-2)][0], dimworker, MPI_INT, i+1, i+1, MPI_COMM_WORLD);
                }                
            }
        }else{
            //si es impar se envían el arreglo del worker que más labura se une con la unión de las demás listas en el master
        }

        /*
        TODO pensar como utilizar matriz dinámica para recibir los datos de los nodos al hacer la unión de las listas ordenadas
        
        */
    }
    else if (myId < nworkers) // codigo worker normal
    {
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
