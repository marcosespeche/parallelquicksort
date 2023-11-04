#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char* argv[]) {

    if (argc != 2) {

        printf("Cantidad de parametros no valida\n");
        return 1;

    }

    int cantNumeros = atoi(argv[1]);

    srand(time(NULL));

    FILE* fileptr;

    fileptr = fopen("datos.txt", "w"); //El primer parametro es el nombre del archivo, ya deberia estar creado en la misma ruta que este programa

    int numerosGenerados[cantNumeros];

    for (int i = 0; i < cantNumeros; i++) {
        int numero = rand() % 101; // Genera un numero aleatorio entre 0 y 100
        fprintf(fileptr, "%d ", numero); // Escribe cada número como texto en el archivo
        printf("%d  ", numero);
    }

    fclose(fileptr);

	printf("\nSe han escrito los numeros en el archivo\n\n");

    //Lee del archivo generado anteriormente

    int numeros[cantNumeros];

    if (!(fileptr = fopen("datos.txt", "r"))) {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    for (int i = 0; i < cantNumeros; i++) {
        if (fscanf(fileptr, "%d", &numeros[i]) != 1) {
            printf("Error al leer el archivo.\n");
            return 1;
        }
    }

    // Mostrar los números leídos
    for (int i = 0; i < cantNumeros; i++) {
        printf("%d  ", numeros[i]);
    }

    fclose(fileptr);

    return 0;
}
