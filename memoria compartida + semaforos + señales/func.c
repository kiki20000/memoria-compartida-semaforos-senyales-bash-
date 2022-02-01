#include <stdio.h>
#include <stdlib.h>
#include <signal.h>   
#include <unistd.h>   
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/time.h>
#include "func.h"


//***************************************************
//INICIALIZACIÓN DEL SEGMENTO COMPARTIDO DE DATOS 
// shmTiempos : Segmento de memoria compartida para los datos de los hijos
// numHijos   : Número de hijos. Tamaño del segmento
//***************************************************
void InitializeSharedMemory(int *shmTiempos, int numHijos){
	//Inicializar el segmendo de memoria compartida a ceros.
	//Begin TODO *************************************
	
	for(int i = 0; i<numHijos; i++){
		shmTiempos[i]=0;
	}
	
	//End TODO *************************************
}

//***************************************************
//BUSQUEDA DE UN ENTERO EN UN ARRAY DE ENTEROS
// v          : puntero al array donde buscar
// len        : longitud del array 
// num        : numero a buscar dentro del array
//***************************************************
bool InArray(int *v, int len, int num){
	int i;
	bool encontrado=false; 
	//printf("Buscando %d\n",num);
	for (i=0; i<len; i++){
		if (v[i]==num){
			encontrado=true;
			break;
		}
	}
	return encontrado;
}


//***************************************************
//LECTURA DE UN ARRAY DE ENTEROS DE UN PIPE 
// fdPipe     : descriptor del pipe del que leer
// arrayDatos : array de enteros donde dejar los datos leidos del pipe
// longitud   : longitud del array de enteros que se lee del pipe (numero de enteros a leer)
//***************************************************
void LeerArrayPipe(int fdPipe, int *arrayDatos, int longitud){
    read(fdPipe, arrayDatos, longitud*sizeof(int));
}

//***************************************************
//ESCRITURA DE UN ARRAY DE ENTEROS DE UN PIPE 
// fdPipe     : descriptor del pipe en el que escribir
// arrayDatos : array de enteros del que coger los datos para ecribirlos en el pipe
// longitud   : longitud del array de enteros que se escribe en el pipe (numero de enteros a escribir)
//***************************************************
void EscribirArrayPipe(int fdPipe, int *arrayDatos, int longitud){
    write(fdPipe, arrayDatos, longitud*sizeof(int));
}

//***************************************************
//MOSTRAR UN ARRAY DE ENTEROS POR STDOUT
// name   : nombre que le damos al array para mostrarlo en stdout
// array  : array del que se muestran los datos
// length : Numero de elementos  mostrar 
// endline: Si true pone \n (endline) al final. Si false no lo pone
//***************************************************
void PrintArray(char *name, int *array, int length){
    int i;
    printf("%s=[",name);fflush(stdout); 
    for (i=0;i<length-1;i++)
        printf("%3d,",array[i]);fflush(stdout); 
   	printf("%3d]\n",array[length-1]);fflush(stdout); 
}


//***************************************************
//ENTERO ALEATORIO ENTRE DOS VALORES
// M    : valor bajo del rango de valores
// N    : valor alto del rango de valores
//***************************************************
int RandInt(int M, int N){
	return rand () % (N-M+1) + M;
}

//***************************************************
//MILISEGUNDOS ENTRE DOS TIEMPOS
// clock_t tIni : tiempo de inicio
// clock_t tFin : tiempo de fin
//Devuelve los milisegundos que han pasado entre esos dos tiempos
//***************************************************
double Seconds(struct timeval start, struct timeval stop){
	return (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
}
