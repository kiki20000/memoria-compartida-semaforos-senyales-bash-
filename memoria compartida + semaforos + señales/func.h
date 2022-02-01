#define IDLE 0
#define WORKING -1


//Definimos una unión usada en sigqueue
union sigval value;

//Puntero del proceso donde queremos ubicar el segmento de memoria compartida.
int *shmTiempos; 

//tamaño de un entero
int intSize; 


//***************************************************
//INICIALIZACIÓN DEL SEGMENTO COMPARTIDO DE DATOS 
//***************************************************
void InitializeSharedMemory(int *shmTiempos, int numHijos);

//***************************************************
//LECTURA DE UN ARRAY DE ENTEROS DE UN PIPE 
//***************************************************
void LeerArrayPipe(int fdPipe, int *arrayDatos, int longitud);

//***************************************************
//ESCRITURA DE UN ARRAY DE ENTEROS DE UN PIPE 
//***************************************************
void EscribirArrayPipe(int fdPipe, int *arrayDatos, int longitud);

//***************************************************
//BUSQUEDA DE UN ENTERO EN UN ARRAY DE ENTEROS
//***************************************************
bool InArray(int *v, int len, int num);

//***************************************************
//MOSTRAR UN ARRAY DE ENTEROS POR STDOUT
//***************************************************
void PrintArray(char *name, int *array, int length);

//***************************************************
//ENTERO ALEATORIO ENTRE DOS VALORES
//***************************************************
int RandInt(int M, int N);

//***************************************************
//MILISEGUNDOS ENTRE DOS TIEMPOS
//***************************************************
double Seconds(struct timeval start, struct timeval stop);

