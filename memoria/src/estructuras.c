#include "estructuras.h"

void iniciar_estructuras(t_config* config){
    int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    int tam_segmento_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
    int cant_segmentos = config_get_int_value(config, "CANT_SEGMENTOS");
    int retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    int retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
    char* algoritmo_asignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");

    MEMORIA_PRINCIPAL = malloc(tam_memoria);
    //log_info(LOGGER_MEMORIA, "Se creo el espacio contiguo de memoria");

    CONFIG = malloc(sizeof(t_configuracion));
    CONFIG->tam_memoria = tam_memoria;
    CONFIG->tam_segmento_0 = tam_segmento_0;
    CONFIG->cant_segmentos = cant_segmentos;
    CONFIG->retardo_memoria = retardo_memoria;
    CONFIG->retardo_compactacion = retardo_compactacion;
    CONFIG->algoritmo_asignacion = algoritmo_asignacion;

    TABLA_SEGMENTOS_GLOBAL = list_create();

    LISTA_HUECOS = list_create();

    t_hueco* hueco_segmento_0 = malloc(sizeof(t_hueco));
    hueco_segmento_0->base = MEMORIA_PRINCIPAL;
    hueco_segmento_0->tamanio = tam_segmento_0;
    hueco_segmento_0->libre = false;
    list_add(LISTA_HUECOS, hueco_segmento_0);
    log_info(LOGGER_MEMORIA, "Se creo el hueco del segmento 0 de tamanio %d en la direccion %p", hueco_segmento_0->tamanio, hueco_segmento_0->base);

    t_hueco* hueco_libre_inicial = malloc(sizeof(t_hueco));
    hueco_libre_inicial->base = MEMORIA_PRINCIPAL + tam_segmento_0;
    hueco_libre_inicial->tamanio = tam_memoria - tam_segmento_0;
    hueco_libre_inicial->libre = true;
    list_add(LISTA_HUECOS, hueco_libre_inicial);
    log_info(LOGGER_MEMORIA, "Se creo el hueco libre inicial de tamanio %d en la direccion %p", hueco_libre_inicial->tamanio, hueco_libre_inicial->base);
}

t_list* crear_tabla_segmentos(){
    t_list* tabla_segmentos = list_create();
    //se me orccure algo, hacer el malloc afuera 
    
    for (int i = 0; i < CONFIG->cant_segmentos; i++) {
        t_segmento* segmento = malloc(sizeof(t_segmento));
        segmento->id_segmento = i;
        segmento->base = NULL;
        segmento->tamanio = 0;
        list_add(tabla_segmentos, segmento);
    }
    
    agregar_segmento_0(tabla_segmentos);
    return tabla_segmentos;
}

void agregar_segmento_0(t_list* tabla_segmentos){
    t_segmento* segmento_0 = list_get(tabla_segmentos, 0);
    segmento_0->base = MEMORIA_PRINCIPAL;
    segmento_0->tamanio = CONFIG->tam_segmento_0;
}



