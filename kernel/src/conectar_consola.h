#ifndef CONECTAR_CONSOLA_H
#define CONECTAR_CONSOLA_H

#include <commons/log.h>
#include <utils/mensajes.h>
#include <utils/conexiones.h>
#include <Estados/new.h>

extern t_log* LOGGER_KERNEL;

void conectar_consola(int);
void enviado_de_consola(int*);

void iterator(t_instruccion*);


t_list * recibir_paquete_consola(int);


#endif