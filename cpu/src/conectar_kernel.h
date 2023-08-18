#ifndef CONECTAR_CONSOLA_H
#define CONECTAR_CONSOLA_H

#include <commons/log.h>
#include <utils/mensajes.h>
#include <utils/planificacion/pcb.h>
#include <instruccionescpu.h>

extern t_log* LOGGER_CPU;


void conectar_kernel(int);
void enviado_de_kernel(int *);
t_ctx* recibir_paquete_kernel(int);

#endif