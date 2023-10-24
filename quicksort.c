#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

void swap (int* a, int* b){
 int temporal = *a;
 *a = *b;
 *b = temporal;
}


int partition (int vect[], int indInic, int indFinal){

 int iswap = indInic -1;
 int pivote = vect[indFinal];
 
 for (int j=indInic; j<=indFinal-1; j++){
  
  if (vect[j]<pivote){
   iswap++;
   swap(&vect[iswap],&vect[j]);
  }	
  					
 }
 swap(&vect[iswap+1],&vect[indFinal]);
 return iswap + 1;
}


void quicksort(int vect[], int indInic, int indFinal){
 
 if (indInic < indFinal){
 
  int pi = partition(vect, indInic, indFinal);
 	
  quicksort(vect, indInic, pi -1);
  quicksort(vect, pi+1, indFinal);
 }
}

void main(int argc,char* argv[]){
 clock_t start, end;
 double tmpejec;
 start = clock();
 
 int dim = argc - 1;
 int vect[dim];
 for (int i=0; i<dim; i++){
  vect[i]=atoi(argv[i+1]);
 }
 printf("Arreglo Inicial:\n");
 for (int i=0; i<dim; i++){
 printf(" %d ", vect[i]);
 }
 
 quicksort(vect,0,dim-1);
 
 printf("\nArreglo sorteado:\n");
 
 for (int i=0; i<dim; i++){
 printf(" %d ", vect[i]);
 }
 
 end = clock();
 tmpejec = ( (double) (end - start)) /CLOCKS_PER_SEC;
 
 printf("\nPrograma terminado en %fs\n", tmpejec);
}
