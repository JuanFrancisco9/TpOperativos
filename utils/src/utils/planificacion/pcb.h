#ifndef PCB_H_
#define PCB_H_

#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char AX[4];
    char BX[4];
    char CX[4];
    char DX[4];

    char EAX[8];
    char EBX[8];
    char ECX[8];
    char EDX[8];

    char RAX[16];
    char RBX[16];
    char RCX[16];
    char RDX[16];
} t_registros;

typedef struct
{
    int cantidad_parametros;
    char **parametros;
} t_parametros_variables;

typedef enum
{
    INSTRUCCIONES, // Se utiliza de consola a kernel
    MENSAJE, // Se utiliza de kernel a consola
    CONTEXTO, // Se utiliza de kernel a cpu
    COMPACTAR,
    CREAR_TABLA_SEGMENTOS,
    TERMINAR,
    NO_EXISTE, //Se utiliza para la comunicacion entre kernel y fs
    EXISTE,    //Se utiliza para la comunicacion entre kernel y fs
    OP_TERMINADA, //Se utiliza para la comunicacion entre kernel y fs
    F_CREATES, //Se utiliza cuando el Kernel intenta abrir un archivo que no existe entonces le dice a fs que lo cree
    OUT_OF_MEMORY,
    SET,
    F_READ,
    MOV_IN,
    MOV_OUT,
    YIELD,
    IO,
    WAIT,
    SIGNAL,
    CREATE_SEGMENT, 
    DELETE_SEGMENT, 
    F_WRITE, 
    F_TRUNCATE,
    F_SEEK,
    F_OPEN,
    F_CLOSE,
    EXIT, 
    SEG_FAULT
} t_operacion;


typedef struct
{
    t_operacion operacion;
    int cantidad_parametros;
    char **parametros;
} t_instruccion;

typedef struct {
    int id_segmento;
    void* base;
    int tamanio;
} t_segmento;


typedef struct
{
    int PID;
    int cant_instrucciones;
    t_list *instrucciones;
    int program_counter;
    t_registros registros;
    t_parametros_variables *motivos_desalojo;
    t_list* tabla_segmentos;
} t_ctx;

typedef struct
{
    t_ctx* contexto;
    float estimado_prox_rafaga;
    int64_t tiempo_llegada_ready;       // cuando pasa a ready usar  = temporal_gettime(tiempo_desde_ult_ready)
    t_list *archivos_abiertos;
    t_list *recursos_en_uso;
    int socket_consola;
} t_pcb;

void agregar_parametro_desalojo(t_ctx* ctx, char* parametro);
void agregar_parametro_variable(t_parametros_variables* , char* );

char get_pid(t_pcb*);
char* mostrar_pids(t_list* lista);
void concatenarCharACadena(char c, char *cadena);

void liberar_contexto(t_ctx *ctx);
void liberar_elementos_contexto(t_ctx *ctx);
void liberar_instruccion(t_instruccion *instruccion);
void liberar_segmento(t_segmento* segmento);
void liberar_parametros_desalojo(t_parametros_variables *parametros_variables);
void vaciar_parametros_desalojo(t_parametros_variables *);






#endif
