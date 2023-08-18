#ifndef INSTRUCCIONES_MEMORIA_H
#define INSTRUCCIONES_MEMORIA_H

    #include <commons/collections/list.h>
    #include <utils/planificacion/pcb.h>
    #include "estructuras.h"
    #include "funciones_auxiliares.h"
    #include "globales.h"
    #include <commons/string.h>

    extern t_list* LISTA_HUECOS;
    extern t_configuracion* CONFIG;

    t_paquete* crear_segmento(int, int, t_ctx*);
    void eliminar_segmento(t_list*, int, int);
    void finalizar_proceso(t_list*, int);
    char* leer_valor_direccion_fisica(long, int, int, char*);
    void escribir_valor_direccion_fisica(char* , long, int, char*);
    void compactar();

#endif