#ifndef GLOBALES_H_
#define GLOBALES_H_

    #include <commons/collections/list.h>
    #include <commons/log.h>
    #include <estructuras.h>

    extern void* MEMORIA_PRINCIPAL;
    extern t_list* LISTA_HUECOS;
    extern t_list* TABLA_SEGMENTOS_GLOBAL;
    
    extern t_log* LOGGER_MEMORIA;
    extern int SOCKET_KERNEL;


#endif