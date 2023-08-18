#ifndef GLOBALES_H_
#define GLOBALES_H_

    #include <utils/logs.h>
    #include <commons/bitarray.h>
    #include <commons/collections/dictionary.h>

    extern t_log* LOGGER_FILE_SYSTEM;

    extern t_bitarray* BITMAP_BLOQUES;

    extern FILE * ARCHIVO_BLOQUES;

    extern t_dictionary * DICCIONARIO_FCB;

    extern int TAMANIO_BLOQUES;
    extern int CANTIDAD_BLOQUES;

    extern int SOCKET_MEMORIA;
    extern int SOCKET_KERNEL;

    extern int RETARDO;

#endif