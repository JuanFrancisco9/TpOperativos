#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "planificacion/pcb.h"


// make necessary includes
// typedef enum
// {
//     INSTRUCCIONES, // Se utiliza de consola a kernel
//     MENSAJE, // Se utiliza de kernel a consola
//     CONTEXTO, // Se utiliza de kernel a cpu
//     COMPACTAR,
//     CREAR_TABLA_SEGMENTOS,
//     TERMINAR,
//     NO_EXISTE, //Se utiliza para la comunicacion entre kernel y fs
//     EXISTE,    //Se utiliza para la comunicacion entre kernel y fs
//     OP_TERMINADA, //Se utiliza para la comunicacion entre kernel y fs
//     F_CREATES, //Se utiliza cuando el Kernel intenta abrir un archivo que no existe entonces le dice a fs que lo cree
//     OUT_OF_MEMORY,
// } op_code;


typedef struct
{
    int size;
    void *stream;
} t_buffer;

typedef struct
{
    t_operacion codigo_operacion;
    t_buffer *buffer;
} t_paquete;

typedef struct {
    int PID;
    t_list* segmentos;
} t_tabla_segmentos;

// Funciones del tp0
t_paquete *crear_paquete(int);
void crear_buffer(t_paquete *);
void *serializar_paquete(t_paquete *, int);
void enviar_mensaje(char *, int);
int recibir_operacion(int);
char* recibir_mensaje(int);
void agregar_a_paquete_dato_serializado(t_paquete *, void *, int);
void enviar_paquete(t_paquete *, int);
void* recibir_buffer(int* , int);
void eliminar_paquete(t_paquete *paquete);



// Serializaciones + Deserealizaciones que usan las funciones de arriba
void serializar_contexto(t_ctx*, t_paquete*);
void serializar_instrucciones(t_list *, t_paquete* );
void serializar_instruccion(t_instruccion *, t_paquete *);
void serializar_registros(t_registros *, t_paquete *);
void serializar_motivos_desalojo(t_parametros_variables *, t_paquete *);
void serializar_tabla_segmentos(t_list *, t_paquete *);
t_ctx *deserializar_contexto(void *, int*);
t_registros deserealizar_registros(void *, int*);
t_instruccion* deserealizar_instruccion(void*, int* );
t_parametros_variables* deserealizar_motivos_desalojo(void *, int*);
t_list* deserializar_tabla_segmentos(void*, int*);
t_list* recibir_tabla_segmentos(int);
int deserializar_int(void *, int *);
int recibir_int(int );
void serializar_todas_las_tablas_segmentos(t_list* , t_paquete* );
t_list* deserealizar_todas_las_tablas_segmentos(void* , int* );
t_list* recibir_todas_las_tablas_segmentos(int );


#endif
