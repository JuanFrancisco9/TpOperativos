#ifndef FUNCIONES_AUXILIARES_H
#define FUNCIONES_AUXILIARES_H

    #include <utils/logs.h>
    #include <utils/configs.h>
    #include <commons/collections/list.h>
    #include <utils/mensajes.h>
    #include "estructuras.h"

    t_hueco* get_hueco_con_first_fit(int);
    t_hueco* get_hueco_con_best_fit(int);
    t_hueco* get_hueco_con_worst_fit(int);
    void modificar_lista_huecos(t_hueco*, int);
    bool comprobar_compactacion(int);
    void comprobar_consolidacion_huecos_aledanios(int);
    int obtener_index_tabla_segmentos(int);
    void mostrar_lista_huecos();
    void actualizar_bases_huecos();
    void mostrar_tabla_global();
#endif