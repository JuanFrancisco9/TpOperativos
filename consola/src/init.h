#ifndef INIT_CONSOLA_H
#define INIT_CONSOLA_H

#include <utils/conexiones.h>
#include <utils/configs.h>

extern t_log* logger_consola;

void inicializar_conexiones(int *, t_config* );
FILE* verificar_argumentos(int, char**);

#endif