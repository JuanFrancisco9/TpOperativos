#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

    #include <utils/logs.h>
    #include <utils/configs.h>
    #include <commons/collections/list.h>
    #include <commons/bitarray.h>
    #include <utils/mensajes.h>


    typedef struct {
        int tam_memoria;
        int tam_segmento_0;
        int cant_segmentos;
        int retardo_memoria;
        int retardo_compactacion;
        char* algoritmo_asignacion;
    } t_configuracion;

    typedef struct {
        void* base; 
        int tamanio;
        bool libre;
    } t_hueco;

    extern t_log* LOGGER_MEMORIA;
    extern void* MEMORIA_PRINCIPAL;
    extern t_configuracion* CONFIG;
    extern t_list* LISTA_HUECOS;
    extern t_list* TABLA_SEGMENTOS_GLOBAL;

    void iniciar_estructuras(t_config* config);
    t_list* crear_tabla_segmentos();
    void agregar_segmento_0(t_list* tabla_segmentos);
    void serializar_tabla_segmentos(t_list* tabla_segmentos, t_paquete* paquete);

#endif