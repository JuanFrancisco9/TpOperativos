#ifndef INIT_CPU_H
#define INIT_CPU_H

#include <utils/conexiones.h>
#include <utils/configs.h>
#include "globales.h"

void inicializar_conexiones(int *, t_config* , t_log* );
void inicializar_variables_globales(t_config* config);
#endif