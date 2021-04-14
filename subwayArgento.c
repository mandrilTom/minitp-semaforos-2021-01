#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50

pthread_mutex_t sarten_mutex;
pthread_mutex_t salero_mutex;
sem_t sem_horno;
pthread_mutex_t salida_mutex;

int ganadores[4];
int indice_ganador;

//creo estructura de semaforos 
struct semaforos {
	sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_agregar_a_mezcla;
	sem_t sem_empanar;
	sem_t sem_cocinar;
	sem_t sem_armar_sandwich_milanesa;
	sem_t sem_armar_sandwich_pan;
	sem_t sem_armar_sandwich_extras;
};

//creo los pasos con los ingredientes
struct paso {
   char accion [LIMITE];
   char ingredientes[4][LIMITE];
};

//creo los parametros de los hilos 
struct parametro {
	int equipo_param;
	struct semaforos semaforos_param;
	struct paso pasos_param[8];
	FILE* receta;
	FILE* resultado;
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* realizarAccion(int equipo, char *accionIn, char *ingrediente, FILE* salida) {
	pthread_mutex_lock(&salida_mutex);
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "Equipo : %d, Accion : %s, Ingrediente : %s", equipo, accionIn, ingrediente);
	salida = fopen("resultado.txt", "at");
	fprintf(salida, "%s\n", buffer);
	fclose(salida);
	printf("%s\n", buffer);
	pthread_mutex_unlock(&salida_mutex);
}

void *accionTerminada(void *data, char *accionIn) {
	pthread_mutex_lock(&salida_mutex);
	struct parametro *mydata = data;
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "Equipo : %d, Terminado : %s\n", mydata->equipo_param, accionIn);
	mydata->resultado = fopen("resultado.txt", "at");
	fprintf(mydata->resultado, "%s\n", buffer);
	fclose(mydata->resultado);
	printf("%s\n", buffer);
	pthread_mutex_unlock(&salida_mutex);
}

//funcion para tomar de ejemplo
void* picar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "Picar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[0].ingredientes[0], mydata->resultado);
	//uso sleep para simular que que pasa tiempo
	usleep( 10000 );
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[0].ingredientes[1], mydata->resultado);
	usleep( 10000 );
	//doy la señal a la siguiente accion (picar me habilita mezclar)
	sem_post(&mydata->semaforos_param.sem_mezclar);
	accionTerminada(mydata, accion);
	sem_post(&mydata->semaforos_param.sem_mezclar);
	pthread_exit(NULL);
}

void* mezclar(void *data) {
	char *accion = "Mezclar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_mezclar);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[1].ingredientes[0], mydata->resultado);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[1].ingredientes[1], mydata->resultado);
	usleep(2000000);
	accionTerminada(mydata, accion);
	sem_post(&mydata->semaforos_param.sem_salar);
	pthread_exit(NULL);
}

void* salar(void *data) {
	char *accion = "Salar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_salar);
	pthread_mutex_lock(&salero_mutex);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[2].ingredientes[0], mydata->resultado);
	usleep(1000000);
	accionTerminada(mydata, accion);
	pthread_mutex_unlock(&salero_mutex);
	sem_post(&mydata->semaforos_param.sem_agregar_a_mezcla);
	pthread_exit(NULL);
}

void* agregar_a_mezclar(void *data) {
	char *accion = "Agregar a mezcla";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_agregar_a_mezcla);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[3].ingredientes[0], mydata->resultado);
	usleep(3000000);
	accionTerminada(mydata, accion);
	sem_post(&mydata->semaforos_param.sem_empanar);
	pthread_exit(NULL);
}

void* empanar(void *data) {
	char *accion = "Empanar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_empanar);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[4].ingredientes[0], mydata->resultado);
	usleep(2000000);
	accionTerminada(mydata, accion);
	sem_post(&mydata->semaforos_param.sem_cocinar);
	pthread_exit(NULL);
}

void* cocinar(void *data) {
	char *accion = "Cocinar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_cocinar);
	pthread_mutex_lock(&sarten_mutex);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[5].ingredientes[0], mydata->resultado);
	usleep( 5000000 );
	accionTerminada(mydata, accion);
	pthread_mutex_unlock(&sarten_mutex);
	sem_post(&mydata->semaforos_param.sem_armar_sandwich_milanesa);
	pthread_exit(NULL);
}

void* hornear_pan(void *data) {
	char *accion ="Hornear";
	struct parametro *mydata = data;
	sem_wait(&sem_horno);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[6].ingredientes[0], mydata->resultado);
	usleep(10000000);
	accionTerminada(mydata, accion);
	sem_post(&sem_horno);
	sem_post(&mydata->semaforos_param.sem_armar_sandwich_pan);
	pthread_exit(NULL);
}

void* cortar_extras(void *data) {
	char *accion = "Cortar";
	struct parametro *mydata = data;
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[0], mydata->resultado);
	usleep(1000000);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[1], mydata->resultado);
	usleep(1000000);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[2], mydata->resultado);
	usleep(1000000);
	realizarAccion(mydata->equipo_param, accion, mydata->pasos_param[7].ingredientes[3], mydata->resultado);
	usleep(1000000);
	accionTerminada(mydata, accion);
	sem_post(&mydata->semaforos_param.sem_armar_sandwich_extras);
	pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_mezclar;
	//crear variables semaforos aqui
	
	//variables hilos
	pthread_t p1; 
	//crear variables hilos aqui
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);
	
	printf("Ejecutando equipo %d \n", p);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
	//setear demas semaforos al struct aqui
	

	//seteo las acciones y los ingredientes (Faltan acciones e ingredientes) ¿Se ve hardcodeado no? ¿Les parece bien?
     	strcpy(pthread_data->pasos_param[0].accion, "cortar");
	    strcpy(pthread_data->pasos_param[0].ingredientes[0], "ajo");
      strcpy(pthread_data->pasos_param[0].ingredientes[1], "perejil");


	    strcpy(pthread_data->pasos_param[1].accion, "mezclar");
	    strcpy(pthread_data->pasos_param[1].ingredientes[0], "ajo");
      strcpy(pthread_data->pasos_param[1].ingredientes[1], "perejil");
 	    strcpy(pthread_data->pasos_param[1].ingredientes[2], "huevo");
	    strcpy(pthread_data->pasos_param[1].ingredientes[3], "carne");
	
	
	//inicializo los semaforos

	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	//inicializar demas semaforos aqui


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	//crear demas hilos aqui
	
	
	//join de todos los hilos
	pthread_join (p1,NULL);
	//crear join de demas hilos


	//valido que el hilo se alla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     }

	 
	//destruccion de los semaforos 
	sem_destroy(&sem_mezclar);
	//destruir demas semaforos 
	
	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	//creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
//faltan equipos
  
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;

	//creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
//faltan hilos
  
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre2);
  //faltn inicializaciones


   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
//.. faltan joins


    pthread_exit(NULL);
}


//Para compilar:   gcc subwayArgento.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
