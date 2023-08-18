#include "instrucciones_memoria.h"

// void mostrar_hueco(t_hueco* hueco){
//     log_info(LOGGER_MEMORIA, "Hueco: <%p> - TAMAÑO: <%d> - LIBRE: <%d>", hueco->base, hueco->tamanio, hueco->libre);
// }

void mostrar_segmento(t_segmento *segmento)
{
    log_info(LOGGER_MEMORIA, "Segmento: <%p> - TAMAÑO: <%d>", segmento->base, segmento->tamanio);
}

// void mostrar_tablas(t_tabla_segmentos* ts){
//     log_info(LOGGER_MEMORIA, "PID: <%d>", ts->PID);
//     list_iterate(ts->segmentos, (void*)mostrar_segmento);

/*

TABLA_SEGMENTOS_GLOBAL -> t_list* t_tabla_segmentos
t_tabla_segmentos -> int pid | t_list* t_segmento
t_segmento -> void* base | int tamanio | int id

*/

// }

void liberar_segmentoo(t_segmento *segmento)
{
    free(segmento);
}

void liberar_tabla_segmentos(t_tabla_segmentos *ts)
{
    list_destroy_and_destroy_elements(ts->segmentos, (void *)liberar_segmentoo);
    free(ts);
}

t_paquete *crear_segmento(int id_segmento, int tamanio, t_ctx *ctx)
{

    t_hueco *hueco = NULL;
    if (strcmp(CONFIG->algoritmo_asignacion, "FIRST") == 0)
    {
        hueco = get_hueco_con_first_fit(tamanio);
    }
    else if (strcmp(CONFIG->algoritmo_asignacion, "BEST") == 0)
    {
        hueco = get_hueco_con_best_fit(tamanio);
    }
    else if (strcmp(CONFIG->algoritmo_asignacion, "WORST") == 0)
    {
        hueco = get_hueco_con_worst_fit(tamanio);
    }
    else
    {
        log_error(LOGGER_MEMORIA, "Algoritmo de asignacion no valido");
        return NULL;
    }

    if (!hueco && comprobar_compactacion(tamanio))
    {
        return crear_paquete(COMPACTAR);
    }
    else if (!hueco)
    {
        log_error(LOGGER_MEMORIA, "No hay hueco disponible para crear el segmento");
        return crear_paquete(OUT_OF_MEMORY);
    }

    modificar_lista_huecos(hueco, tamanio);

    t_segmento *segmento = list_get(ctx->tabla_segmentos, id_segmento);

    memcpy(&(segmento->base), &hueco->base, sizeof(void *));
    memcpy(&(segmento->tamanio), &tamanio, sizeof(int));

    t_tabla_segmentos *ts = malloc(sizeof(t_tabla_segmentos));
    ts->PID = ctx->PID;
    ts->segmentos = ctx->tabla_segmentos;

    int index = obtener_index_tabla_segmentos(ctx->PID);

    list_replace_and_destroy_element(TABLA_SEGMENTOS_GLOBAL, index, ts, (void *)liberar_tabla_segmentos);
    t_paquete *paquete = crear_paquete(CREATE_SEGMENT);
    agregar_a_paquete_dato_serializado(paquete, &(segmento->base), sizeof(segmento->base));
    log_info(LOGGER_MEMORIA, "PID: <%d> - Crear Segmento: <%d> - Base: <%p> - TAMAÑO: <%d>", ctx->PID, id_segmento, hueco->base, tamanio);
    return paquete;
}

void eliminar_segmento(t_list *tabla_segmentos, int id_segmento, int PID)
{
    t_segmento *segmento = list_get(tabla_segmentos, id_segmento);
    if (PID != -1)
        log_info(LOGGER_MEMORIA, "PID: <%d> - Eliminar Segmento: <%d> - Base: <%p> - TAMAÑO: <%d>", PID, id_segmento, segmento->base, segmento->tamanio);

    // buscar hueco que tenga la misma base que el segmento y marcarlo como libre
    int index_hueco = 0;
    for (int i = 0; i < list_size(LISTA_HUECOS); i++)
    {
        t_hueco *hueco = list_get(LISTA_HUECOS, i);
        if (hueco->base == segmento->base)
        {
            hueco->libre = true;
            index_hueco = i;
            break;
        }
    }
    comprobar_consolidacion_huecos_aledanios(index_hueco);

    segmento->base = NULL;
    segmento->tamanio = 0;

    t_tabla_segmentos *ts = malloc(sizeof(t_tabla_segmentos));
    ts->PID = PID;
    ts->segmentos = tabla_segmentos;

    int index = obtener_index_tabla_segmentos(PID);

    list_replace_and_destroy_element(TABLA_SEGMENTOS_GLOBAL, index, ts, (void *)liberar_tabla_segmentos);
}

// t_list -> [t_segmento, t_segmento, t_segmento, ..., t_segmento]
// t_segmento -> void* base | int tamanio | int id
// t_segmento 0 no hay que eliminarlo
void finalizar_proceso(t_list *tabla_segmentos, int PID)
{
    free(list_remove(tabla_segmentos, 0));
    for (int i = 0; i < list_size(tabla_segmentos); i++)
    {
        t_segmento *segmento = list_get(tabla_segmentos, i);
        if (segmento->base != NULL)
        {
            // buscar hueco que tenga la misma base que el segmento y marcarlo como libre
            int index_hueco = 0;
            for (int i = 0; i < list_size(LISTA_HUECOS); i++)
            {
                t_hueco *hueco = list_get(LISTA_HUECOS, i);
                if (hueco->base == segmento->base)
                {
                    hueco->libre = true;
                    index_hueco = i;
                    break;
                }
            }
            comprobar_consolidacion_huecos_aledanios(index_hueco);
        }
        list_remove_and_destroy_element(tabla_segmentos, i, (void *)liberar_segmentoo);
        i--;
    }

    for (int i = 0; i < TABLA_SEGMENTOS_GLOBAL->elements_count; i++){
        t_tabla_segmentos* ts = list_get(TABLA_SEGMENTOS_GLOBAL, i);
        if (ts->PID == PID){
            list_remove_and_destroy_element(TABLA_SEGMENTOS_GLOBAL, i, (void *)liberar_tabla_segmentos);
            break;
        }
    }
}

char *leer_valor_direccion_fisica(long direccion_fisica, int tamanio, int pid, char *origen)
{
    sleep(CONFIG->retardo_memoria / 500);
    char *valor = malloc(tamanio * sizeof(char *));
    memcpy(valor, (void *)direccion_fisica, tamanio * sizeof(char *));
    log_info(LOGGER_MEMORIA, "PID: <%d> - Acción: <LEER> - Dirección física: <%p> - Tamaño: <%d> - Origen: <%s>", pid, (void *)direccion_fisica, tamanio, origen);
    return valor;
}

void escribir_valor_direccion_fisica(char *valor, long direccion_fisica, int pid, char *origen)
{

    sleep(CONFIG->retardo_memoria / 500);
    void *direccion = (void *)direccion_fisica;
    int tamanio = strlen(valor) + 1;
    memcpy(direccion, valor, tamanio);
    log_info(LOGGER_MEMORIA, "PID: <%d> - Acción: <ESCRIBIR> - Dirección física: <%p> - Tamaño: <%d> - Origen: <%s>", pid, direccion, tamanio, origen);
}

void compactar()
{
    log_info(LOGGER_MEMORIA, "Se solicita compactacion");
    usleep(CONFIG->retardo_compactacion * 250);
    int nuevo_tamanio = 0;
    void *base_del_primer_hueco = NULL;

    for (int i = 1; i < list_size(LISTA_HUECOS); i++)
    {
        t_hueco *hueco = list_get(LISTA_HUECOS, i);

        if (base_del_primer_hueco == hueco->base)
        {
            hueco->tamanio = nuevo_tamanio;
            // eliminar los huecos que estan a la derecha del hueco que se esta compactando
            for (int j = i + 1; j < list_size(LISTA_HUECOS); j++)
            {
                list_remove(LISTA_HUECOS, j);
            }
            break;
        }

        if (hueco->libre)
        {
            // agregarlo al final de la lista de huecos
            list_remove(LISTA_HUECOS, i);
            list_add(LISTA_HUECOS, hueco);
            nuevo_tamanio += hueco->tamanio;
            if (!base_del_primer_hueco)
                base_del_primer_hueco = hueco->base;
            i--;
        }
    }

    // modificar las bases de los huecos en relacion a su tamanio (solo los ocupados)
    t_hueco *hueco = list_get(LISTA_HUECOS, 0);
    void *base_actual = hueco->base;
    int tamanio_actual = hueco->tamanio;

    for (int i = 1; i < list_size(LISTA_HUECOS); i++)
    {
        t_hueco *hueco = list_get(LISTA_HUECOS, i);
        if (!hueco->libre)
        {
            // modificar la tabla de segmentos
            // buscar el segmento en todas las tablas de segmentos
            for (int j = 0; j < list_size(TABLA_SEGMENTOS_GLOBAL); j++)
            {
                t_tabla_segmentos *tabla_segmentos = list_get(TABLA_SEGMENTOS_GLOBAL, j);
                for (int k = 1; k < list_size(tabla_segmentos->segmentos); k++)
                {
                    t_segmento *segmento = list_get(tabla_segmentos->segmentos, k);
                    if (segmento->base == hueco->base)
                    {
                        segmento->base = base_actual + tamanio_actual;
                        log_info(LOGGER_MEMORIA, "PID: <%d> - Segmento: <%d> - Base: <%p> - TAMAÑO: <%d>", tabla_segmentos->PID, k, segmento->base, segmento->tamanio);
                        break;
                    }
                }
            }

            hueco->base = base_actual + tamanio_actual;
            base_actual = hueco->base;
            tamanio_actual = hueco->tamanio;
        }
        else
        {
            hueco->base = base_actual + tamanio_actual;
            tamanio_actual = hueco->tamanio;
        }
    }

}