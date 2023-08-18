
#ifndef INSTRUCCIONESCPU_H_
#define INSTRUCCIONESCPU_H_

#include <commons/log.h>
#include <utils/conexiones.h>
#include <utils/planificacion/pcb.h>
#include <utils/mensajes.h>
#include <unistd.h>
#include <commons/string.h>
#include "globales.h"


extern t_log* LOGGER_CPU;
extern int TIEMPO_RETARDO;
extern int TAM_MAX_SEGMENTO;

void ciclo_de_instruccion(t_ctx* );
char* obtenerRegistro(t_registros* , const char*);
t_instruccion* fetch(t_ctx *);
void decode(t_instruccion* );
int floor_div(int, int);
long MMU(int, int,t_ctx* );


#endif