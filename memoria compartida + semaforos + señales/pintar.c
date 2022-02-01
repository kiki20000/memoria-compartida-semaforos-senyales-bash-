#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>   
#include <signal.h>   
#include <string.h>
#include <sys/ipc.h>   
#include <sys/shm.h>  
#include <stdbool.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "func.h"
#include "sem.h"

#define SSEED 99
#define SHMSEED 24 
#define SHMPERMISOS 0644
#define MINHIJOS 2
#define MAXHIJOS 12
#define MINPIEZASHIJO 1
#define MAXPIEZASHIJO 10
#define MINTIEMPO 5
#define MAXTIEMPO 10
#define PARENTDELAY 1

struct datos_pieza{
	double tPrevisto;
	double tFinal; 
	double tDelay;
};

typedef struct datos_pieza t_datos_pieza;

int numHijos; // para almacenar el numero de hijos
int piezasPendientes; //Para el numero de piezas que quedan por pintar.
bool doWork; //Para coordinar que el hijo comienze el trabajo de pintura
int *pids; //Para almacenar los pids de los hijos conforme los creo.

//Declaramos el identificador del semáforo que juega el rol mutex para el acceso a la memoria compartida
int shmMutex;

bool sigurs1recevied;

//*****************************************************************
//FUNCIÓN que envía la señal al padre. Hijo ha terminado de pintar la pieza.
//*****************************************************************
void SignalWorkDone(int numHijo){
	//(debug) printf("  ->Signaling WordDone a padre %d por hijo %d\n",getppid(),numHijo);
	//BEGIN TODO
		//Asignamos el valor numHijo en la unión sigval definida en func.h
		//nuHijo es el numero de hijo en orden de creación. Indice del array shmTiempos
		value.sival_int = numHijo; 
		//Enviamos una señal SIGRTMIN con sigqueue() al padre. value es la unión sigval a enviar con los datos.
		sigqueue(getppid(), SIGRTMIN, value);  //Usamos sigqueue para enviar señales en tiempo real
	//END TODO
}

//*****************************************************************
//FUNCIÓN Señal que envía padre al hijo para que empieze a pintar
//*****************************************************************
void SignalWorkStart(int pid){
	//BEGIN TODO
		//Enviamos señal SIGUSR1 al hijo para que comienze a pintar la pieza.
		//No es una señal de tiempo real.
		kill(pid, SIGUSR1);  //usamos kill para enviar señales normales
	//END TODO
	//(debug) printf("->Signaling WorkStart pid(%d)\n",pid);fflush(stdout); 
}


//***************************************************
//MANEJADORA que gestiona las señales recibidas tanto por el padre como por los hijos
//***************************************************
void Handler(int signo, siginfo_t *info, void *context){
	//BEGIN TODO
		//Debemos discriminar la señal que llega. signo tiene el múnero de señal que llega
		//Si es la señal recibida por el hijo, pondremos la variable sigusr1received=true
		//Si es la señal recibida por el padre, decrementaremos la variable piezasPendientes en una unidad.

		if(signo == SIGUSR1){ //SIGUSR1 es la señal que recibe el hijo
			sigurs1recevied=true;
		}
		else if(signo == SIGRTMIN){ //SIGRTMIN es la señal que recibe el padre
			piezasPendientes--;
		}

	//END TODO
}


//***************************************************
//SelectWork Función que se encarga de escoger el candidato a pintar el siguiente
//En esta versión se selecciona el mayor de todos los tiempos simplemente
//En posteriores versiones se podrá modificar este algoritmo y comparar tiempos.
//***************************************************
int SelectWork(int *array, int length){
	int k, selected=-1;
	int maximo=0;
	for (k=0; k<length; k++){
		if (array[k]>maximo){
			selected=k;
			maximo=array[k];
		}
	}
	//(debug) printf("Selected %d\n",selected);fflush(stdout);
	return selected; 
}


//***************************************************
//LaunchWork - Función que selecciona un pintor y le envía la orden de trabajo.
//***************************************************
void LaunchWork(){
	int selected; 
	//El padre utiliza la función SelectWork para seleccionar un trabajo del array compartido shmTiempos.
	//El padre pone a WORKING en el array de tiempos al hijo seleccionado
	//Si SelectWork devuelve >0 enviamos una señal al hijo seleccionado usando la funcion SignalWorkStart
	//El pid del hijo lo tenemos que haber guardado en un array cuando los creo el padre. Recuperarlo de ahi.
	//Debemos acceder en exclusión mutua con el semáforo shmMutex al array compartido shmTiempos.

	//Wait sobre el semáforo para entrar
	sWait(shmMutex);

		//(debug) printf("**Pendientes %d **\n",piezasPendientes);fflush(stdout);
		//(debug) printf("A->");fflush(stdout);
		//(debug) PrintArray("shmTiempos", shmTiempos, numHijos);

		//El padre selecciona un trabajo con SelectWork (Esta versión selecciona al mayor tiempo)
		selected = SelectWork(shmTiempos, numHijos);

		//Si selected>0 se ha elegido un trabajo correto.
		//Establecemos en shmTiempos ese valor a WORKING (definido en func.h)
		if (selected>=0){
			shmTiempos[selected]=WORKING;
			//(debug) printf("D->");fflush(stdout);
			//(debug) PrintArray("shmTiempos", shmTiempos, numHijos);
    	}

	//Signal sobre el semáforo para salir
	sSignal(shmMutex);

	//Si ae ha elegido un trabajo correto, usamos 
	//SignalWorkStart() para enviar la señal al hijo seleccionado
	if (selected>=0){
		SignalWorkStart(pids[selected]);
	}
}

//***************************************************
//PAINT - Funcion que simula el pintado. 
//***************************************************
void Paint(int tiempo){
	sleep(tiempo); //duerme dureante tiempo segundos.
}


//***************************************************
//SaveChildData Funcion que genera un fichero con nombre ChildNNData.txt
//Con los datos de numero de piezas y para cada pieza el tiempo previsto, el final y el delay acumulado.
//***************************************************
void SaveChildData(int ixChild, int piezasHijo, t_datos_pieza *tPiezas){
   FILE * fp;
   char fileName[200]="";
   int j;

   sprintf(fileName,"Child%02dData.txt",ixChild);

   fp = fopen (fileName, "w+");

 		fprintf(fp,"%02d\n",piezasHijo);
 		for (j=0; j<piezasHijo; j++) {
 			fprintf(fp, "%0.6lf; %0.6lf; %0.6lf\n",tPiezas[j].tPrevisto, tPiezas[j].tFinal, tPiezas[j].tDelay);
 		}
   
   fclose(fp);
}

//***************************************************
//CODIGO HIJOS
//ixChild    : es el ínidce de creación de los hijos. El numero de orden del hijo. Su posicíon en el array compartido.
//piezasHijo : es el numero de piezas que el hijo debe de pintar. 
//***************************************************
void CodigoHijo(int ixChild, int piezasHijo)
{	//El hijo, en bucle, procesará cada pieza.
	//   1-Insertara tiempo (aleatorio) de la pieza en su posicion del array compartido
	//   2-Esperara la señal del padre para pintarla (SignalWorkStart)
	//   3-Pintará la pieza llamando a Paint
	//   4-Pondrá a cero (IDLE en func.h) el tiempo de la pieza recien pintada en el array compartido
	//   5-El hijo enviará señal al padre (SignalWorkDone) indicando que ha terminado la pieza 
	//Al finalizar el bucle el hijo termina

    int j; //iterador generico. No usar el iterador i puesto que e usado por el padre para crear los hijos.
    double tiempo; //tiempo que tardará en pintar una pieza. Lo calculara aleatoriamente.
    struct timeval tIni,tFin;
    double tPieza; //tiempo que tarda en una pieza
    t_datos_pieza *tPiezas; //Array de tiempos que ha utilizado en cada pieza

    //Inicializamos el array para llevar las cuentas
    tPiezas = (t_datos_pieza*) calloc(piezasHijo,sizeof(t_datos_pieza));

    //Bucle de procesamiento de piezas
    for (j=0; j<piezasHijo; j++){
    //Begin TODO---------------------------------------------
    	//--------------------------------------------------------------------
    	//1-Insertara tiempo (aleatorio) de la pieza en su posicion del array compartido
    	//--------------------------------------------------------------------
    		//Obtengo el tiempo necesario para la pieza que obtengo aleatoriamente usando RandInt definida en func.h 
    		//entre MINTIEMPO y MAXTIEMPO segundos definidos en la cabecera.
    		tiempo=RandInt(MINTIEMPO, MAXTIEMPO);
    		tPiezas[j].tPrevisto=tiempo;

    		gettimeofday(&tIni, NULL);

    		//(debug) printf("  Hijo %d pid(%d): Tiempo pieza %d : %0.3lf segundos\n", ixChild, getpid(), j, tiempo);fflush(stdout); 

	    	//Acceso por semaforo al segmento compartido
	    	sWait(shmMutex);
	    		//Actualizo el tiempo en mi posición del array compartido
	    		shmTiempos[ixChild] = tiempo;
	    	//Salgo del segmento compartido
	    	sSignal(shmMutex);
    //End TODO---------------------------------------------

    	//--------------------------------------------------------------------
    	//2-Esperara la señal del padre para pintarla (SignalWorkStart)
	    //  Este paso ya está completo. NO TOCAR.
    	//--------------------------------------------------------------------
	    	//Espera la llegada de la señal del padre
    		//(debug) printf("  Hijo %d pid(%d) espera\n",ixChild,getpid());fflush(stdout); 
    		while(!sigurs1recevied){
    			pause();
    		}
    		sigurs1recevied=false;

    //Begin TODO---------------------------------------------
    	//--------------------------------------------------------------------
    	//3-Pintará la pieza llamando a Paint
    	//--------------------------------------------------------------------
    		//(debug) printf("  Hijo %d pid(%d) compienza a pintar pieza %d durante %0.3lf segundos\n",ixChild, getpid(), j, tiempo); fflush(stdout); 
	    	//Llama a la función Paint
	    	Paint(tiempo);

    	//--------------------------------------------------------------------
    	//4-Pondrá a cero (IDLE) el tiempo de la pieza recien pintada en el array compartido
    	//--------------------------------------------------------------------
	    	//Acceso por semaforo al segmento compartido
	    	sWait(shmMutex);
	    		//Actualizo el tiempo a IDLE (0) en mi posición del array compartido
	    		shmTiempos[ixChild] = 0;
	    	//Salgo del segmento compartido
	    	sSignal(shmMutex);

    //End TODO---------------------------------------------

    	//--------------------------------------------------------------------
    	//5-El hijo enviará señal al padre (SignalWorkDone) indicando que ha terminado la pieza 
    	//--------------------------------------------------------------------
	    	//Envía al padre la señal usando SignalWorkDone
	    	SignalWorkDone(ixChild);

	    gettimeofday(&tFin, NULL);
	    tPieza=Seconds(tIni,tFin);
	    tPiezas[j].tFinal = tPieza;
	    tPiezas[j].tDelay = tPiezas[j].tFinal - tPiezas[j].tPrevisto;
	    SaveChildData(ixChild, piezasHijo, tPiezas);
    }

    //El hijo ha terminado con todas sus piezas.
    for (j=0; j<piezasHijo; j++){
    	//(debug) printf("[%0.6lf]->[%0.6lf]:[%0.6f]\n",tPiezas[j].tPrevisto, tPiezas[j].tFinal, tPiezas[j].tDelay);fflush(stdout);
    }


    //Begin TODO *************************************
		//El hijo se desvincula del segmentos shmTiempos
		shmdt(shmTiempos);
    //End TODO ***************************************

	exit(0);
}
 

int main( int args, char *argv[] )
{
    //Begin NoTocar-------------------------------------
	    int i; //iterador generico
        int pidHijo;    //para registrar el pid del hijo tras fork     
	    int status;     //para capturar el valor de retorno del hijo
	    int maxPiezas;  //para el parametro que pasa el usuario.
	    int piezasHijo; //numero de piezas que tiene que pintar un hijo
	    int tiempo;     //tiempo que el hijo tardará en pintar la pieza. 
	    struct sigaction action;

	    //Declaramos los identificadores para el segmento de memoria compartida
	    //Para el array de tiempos donde cada hijo usará una posición del array
        int shmId_tiempos;

        //Declaramos el tamaño del segmento
        int shmSize_tiempos;

        //Inicializamos la semilla de numeros aleatorios con el pid del padre
        srand(getpid());

        //Inicializamos la variable que servirá para controlar el avance del hijo al llegar la señal.
        sigurs1recevied=false;

    //End NoTocar-------------------------------------

    //Begin TODO-------------------------------------
        //Declarar los maejadores de las señales con sigaction.
        //Usar la función Handler como manejadora de las señales para el padre y para el hjio.
        //El padre recibirá la señal SIGRTMIN y el hijo la señal SIGUSR1

	    //Instanciamos el manejador la señal que usará el padre en la estrucutra action y registramos el manejador de esa señal
		 action.sa_sigaction = Handler;
		 action.sa_flags = SA_SIGINFO;
		 sigaction(SIGRTMIN, &action, NULL); //Usaremos una señal de tiempo real

		 //Instanciamos el manejador la señal que usará el hijo en la estrucutra action y registramos el manejador de esa señal
		 action.sa_sigaction = Handler;
		 action.sa_flags = SA_SIGINFO;
		 sigaction(SIGUSR1, &action, NULL); //Usaremos una señal normal    
    //End TODO-------------------------------------

    //Begin TODO-------------------------------------
		//Inicializaciones y control de argumentos

 		//Creamos el semáforo shmMutex inicializado a uno. Solo puede haber un proceso dentro.
 		shmMutex=sCreate(SSEED, 1);

        //Comprobamos los parametros. 
        //Error y salir si el numero de argumentos no es 3
		if(args != 3){

			printf("ERROR numero de argumentos\n");
			return 0;

		}

        //Usar la variable numHijos para almacenar el número de hijos pasado por parámetro
	    numHijos=atoi(argv[1]);

        //Usar la variable maxPiezas para almacenar el número de piezas máximo que se asignan a un hijo que se pasa por parametro
	    maxPiezas=atoi(argv[2]);

        //Error y salir si el numero de hijos no esta entre MINHIJOS y MAXHIJOS (definidos en la cabecera)
		if(numHijos<MINHIJOS || numHijos>MAXHIJOS){
			printf("ERROR parametro fuera de rango\n");
			return 0;
		}

        //Error y salir si el numero maxPiezas no esta entre MINPIEZASHIJO y MAXPIEZASHIJO (definidos en la cabecera)
	    if(maxPiezas<MINPIEZASHIJO || maxPiezas>MAXPIEZASHIJO){
			printf("ERROR parametro fuera de rango\n");
			return 0;
		}

	    //Creamos (reservamos memoria) el array de pids que usará el padre para llevar control de los hijos creados. 
	    //Usar calloc, ya que inicializa a ceros el array.
	    pids=calloc(numHijos, sizeof(int));

	    //Se define el tamaño del segmento de memoria compartida. Un array de enteros de longitud numero de hijos.
	    shmSize_tiempos = numHijos*sizeof(int);

	    //Creamos los segmento de memoria commpartida 
	    shmId_tiempos = shmget(ftok(".",SHMSEED), shmSize_tiempos, IPC_CREAT | SHMPERMISOS);
		
		//Se asingna el segmento compartido (vinculan, attach) a la variable shmTiempos definida en func.h
	    shmTiempos = shmat(shmId_tiempos, 0, 0);

    //End TODO *************************************

    //Inicializamos el segmento de memoria compartida mediante la función InitializeSharedMemmory ver func.h
    InitializeSharedMemory(shmTiempos,numHijos);

    //Mostramos un mensaje de inicialización e imprimimos el array compartido.
    //(debug) printf("Inicializado shmTiempos\n");fflush(stdout); 
    //(debug) PrintArray("shmTiempos", shmTiempos, numHijos);


    //---***Bucle para crear a los hijos***---
    //Se crean numHijos  hijos, comporbando que fork no provoca errores
    //Se llama al código del Hijo con la función CodigoHijo (definida ariba) pasandole los parametros necesarios.

    //Inicializamos piezasPendientes que usara el padre para saber cuando debe terminar.
	piezasPendientes=0; //Numero de piezas que quedan por pintar.

	printf("Pintando, espere por favor...\n");

    for(i=0; i < numHijos; i++)	{ //El padre itera creando hijos
	    //Begin TODO *************************************
	    	//El padre calcula aleatoriamente (usar RandInt) el numero de piezas que tiene que pintar el hijo en curso
    		//El numero de piezas deberá estar entre MINPIEZASHIJO y las que se indicara como parametro (maxPiezas)
	    	piezasHijo=RandInt(MINPIEZASHIJO, maxPiezas);
	    	//Incrementa el numero de piezasPendientes con las que asigna a este hijo.
	    	piezasPendientes=piezasPendientes+piezasHijo;
	    //End TODO *************************************

	    //Se realiza el fork y capturamos el pid devuelto.	
		pidHijo = fork();

    	//Código de gestión de error del fork
		if (pidHijo==-1) {
			printf("Error: no se ha podido realizar fork\n");fflush(stdout); 
			exit(-1);
		}

		//Codigo para el padre tras el fork
		if (pidHijo != 0){
			//Guardamos el pid del hijo en el array de pids.
			pids[i]=pidHijo; 
    		//(debug) printf("Creado hijo %d pid (%d) con piezas %d, totales %d\n",i, pidHijo, piezasHijo, piezasPendientes);fflush(stdout); 
    	}

    	//Código para el hijo tras el fork
		if (pidHijo==0) {
		    //Hijo es el que hace algo. Le pasamos el iterador (su orden de creación) y las piezas que le toca pintar.
			CodigoHijo(i, piezasHijo); //se llama a código de hijo.
		}

    }

    //Aquí sólo llega el padre
	//(debug) printf("\nTotal piezas a pintar %d\n",piezasPendientes);

    //Begin TODO *************************************
		//Bucle para lanzar trabajos. Usar un bucle while.
		//En cada iteración el padre lanzara trabajos con LaunchWork() y se pondra a dormir PARENTDELAY segundos
		//Estará en ese bule mientras haya piezas pendientes de pintar.

		while(piezasPendientes > 0){

			LaunchWork();
			sleep(PARENTDELAY);

		}

    //End TODO *************************************


    //Begin TODO *************************************
	    //Bucle para recoger la terminación de los hijos. Bucle for que usa waitpid.
	    //(debug) printf("Bucle de terminación de los hijos\n");fflush(stdout); 
	    for(int i = 0; i<numHijos; i++){
	    	waitpid(pids[i], &status, 0);
	    }

    //End TODO *************************************

	//(debug) PrintArray("shmTiempos", shmTiempos, numHijos);

    //Begin TODO *************************************
	    //El padre se desvincula del array de memoria compartida
		//(debug) printf("Padre se desvincula de memoria compartida\n");fflush(stdout); 
        shmdt(shmTiempos);

    //End TODO *************************************
	
    //Begin TODO *************************************
	    //El padre destruye el array de memoria compartida 
	    //(debug) printf("Padre destruye memoria compartida\n");fflush(stdout); 
	    shmctl(shmId_tiempos, IPC_RMID, 0);

	    //El padre destruye el mutex
	    //(debug) printf("Padre destruye semaforo\n");fflush(stdout); 
        sDestroy(shmMutex);

    //End TODO *************************************
    //(debug) printf("Todo destruido\n");fflush(stdout); 

	return 0;
}
