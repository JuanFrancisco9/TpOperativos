#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <utils/logs.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <utils/planificacion/pcb.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
        char identificador[30];
        int PID; //para verificar que solo el archivo abierto lo pueda operar
        t_list * lista_de_procesos_bloqueados;
    }t_tabla_global;

extern t_log* LOGGER_KERNEL;
extern int SOCKET_CPU;
extern int SOCKET_MEMORIA;
extern int SOCKET_FILESYSTEM;

//Contador de procesos generados
extern int PID_COUNT;

//Tabla global de archivos abiertos
extern t_list* TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS; 

//Listas para los estados
extern t_list* LISTA_NEW;
extern t_list* LISTA_READY;
extern t_pcb* EJECUTANDO;
extern t_list* LISTAS_BLOCK;
extern t_list* BLOQUEADOS_FS;


// Variables de configuracion
extern char* ALGORITMO_PLANIFICACION;
extern int ESTIMACION_INICIAL;
extern float HRRN_ALFA;
extern int GRADO_MAX_MULTIPROGRAMACION;
extern char** RECURSOS;
extern int* INSTANCIAS_RECURSOS;

//Semaforos
extern sem_t PROCESO_EN_NEW;
extern sem_t PROCESO_EN_READY;
extern sem_t PROCESO_EN_BLOCKFS;
extern sem_t GRADO_MULTIPROGRAMACION;
extern sem_t CORTO_PLAZO;
extern sem_t ARCHIVO_ABIERTO;
extern sem_t RESPUESTA_FS;
extern sem_t ENTRADA_EN_TABLA_GLOBAL;
extern pthread_mutex_t MUTEX_LISTA_NEW;
extern pthread_mutex_t MUTEX_LISTA_READY;
extern pthread_mutex_t MUTEX_TABLA_ARCHIVOS;
extern pthread_mutex_t MUTEX_LISTA_BLOCKFS;
extern pthread_mutex_t SOLICITUD_FS;
extern pthread_mutex_t SOLICITUD_MEMORIA;


extern t_temporal* TIEMPO_CORRIENDO;
extern t_temporal* TIEMPO_EN_CPU;

//Declaro las funciones de los monitores
void agregar_a_lista_new(t_pcb* );
void agregar_a_lista_ready(t_pcb* );
void agregar_a_lista_exec(t_pcb* );
void agregar_a_lista_blockfs(t_pcb *);
void agregar_a_tabla_global(t_tabla_global *);



t_pcb* sacar_de_lista_new(int);
t_pcb* sacar_de_lista_ready(int);
void sacar_elemento_de_lista_ready(t_pcb* );
void sacar_elemento_de_lista_blockfs(t_pcb *);
t_pcb* sacar_de_lista_exec(int);
t_tabla_global *sacar_de_tabla_global(int );


t_pcb* get_de_lista_ready(int);
t_pcb *get_de_lista_blockfs(int);
t_tabla_global *get_de_tabla_global(int );

int tamnio_lista_ready();
int tamnio_lista_blockfs();
int tamnio_tabla_global();
//cambios de estado
void cambio_de_estado();
char* concatenarLista(t_list* );



#endif