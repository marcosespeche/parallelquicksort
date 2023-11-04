#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

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

void main(int argc, char *argv[])
{
    // recibir por parametro, cuantos nros voy a leer y el nombre del archivo
    int dim = atoi(argv[1]);
    int vect[dim];
    char *nomarchv = argv[2];
    // leer datos del archivo
    FILE *fileptr;

    if (!(fileptr = fopen("datos.txt", "r")))
    {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    int i = 0;
    for (i = 0; i < dim; i++)
    {
        if (fscanf(fileptr, "%d", &vect[i]) != 1)
        {
            printf("Error al leer el archivo.\n");
            return 1;
        }
    }

    fclose(fileptr);

    // ordenamiento
    clock_t start, end;
    double tmpejec;
    start = clock();

    quicksort(vect, 0, dim - 1);

    end = clock();
    tmpejec = ((double)(end - start)) / CLOCKS_PER_SEC;

    // escritura del resultado
    if (!(fileptr = fopen(nomarchv, "w")))
    {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    fprintf(fileptr, "Cantidad de numeros: %d\n", dim);
    fprintf(fileptr, "Tiempo de ejecucion: %f\n", tmpejec);
    for (i = 0; i < dim; i++)
    {
        fprintf(fileptr, "%d ", vect[i]);
    }

    printf("\nPrograma terminado en %fs\n", tmpejec);
}
