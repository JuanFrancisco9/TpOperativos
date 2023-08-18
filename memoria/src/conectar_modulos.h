#ifndef CONECTAR_MODULOS_H
#define CONECTAR_MODULOS_H

    #include <commons/log.h>
    #include <pthread.h>
    #include <utils/conexiones.h>
    #include <utils/mensajes.h>
    #include <utils/planificacion/pcb.h>
    #include "globales.h"
    #include "instrucciones_memoria.h"

    extern t_log* LOGGER_MEMORIA;
    extern t_configuracion* CONFIG;

    void conectar_modulos(int);
    void recibir_kernel(int*);
    void recibir_cpu(int*);
    void recibir_fs(int*);
    t_ctx* recibir_contexto(int);
    t_parametros_variables* recibir_parametros_variables(int);
    void recibir_acceso(t_parametros_variables**, int*, int);
#endif