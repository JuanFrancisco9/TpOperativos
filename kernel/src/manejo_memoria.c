#include "manejo_memoria.h"

void crear_segmento(t_pcb *proceso)
{ 
    // enviar a memoria CREATE_SEGMENT con sus 2 parametros (id del segmento y tamanio)
    // se solicita la creacion del segmento
    pthread_mutex_lock(&SOLICITUD_MEMORIA);
    t_paquete *paquete = crear_paquete(CREATE_SEGMENT);
    serializar_contexto(proceso->contexto, paquete);
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);

    int cod_op = recibir_operacion(SOCKET_MEMORIA);

    switch (cod_op)
    {
    case CREATE_SEGMENT:
        int size;
        void *buffer = recibir_buffer(&size, SOCKET_MEMORIA);

        t_segmento *segmento = list_get(proceso->contexto->tabla_segmentos, atoi(proceso->contexto->motivos_desalojo->parametros[0]));

        memcpy(&(segmento->base), buffer, sizeof(segmento->base));
        segmento->tamanio = atoi(proceso->contexto->motivos_desalojo->parametros[1]);
        free(buffer);

        pthread_mutex_unlock(&SOLICITUD_MEMORIA);
        log_info(LOGGER_KERNEL, "PID: <%d> - Crear Segmento - Id: <%d> - Tamaño: <%d>", proceso->contexto->PID, segmento->id_segmento, segmento->tamanio);
        break;
    case COMPACTAR:

        log_info(LOGGER_KERNEL, "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>");
        pthread_mutex_lock(&SOLICITUD_FS);
        log_info(LOGGER_KERNEL, "Inicia la compactacion");

        enviar_compactacion();
        recibir_operacion(SOCKET_MEMORIA);
        recibir_operacion(SOCKET_MEMORIA);


        t_list *tablas_de_segmentos_actualizadas = recibir_todas_las_tablas_segmentos(SOCKET_MEMORIA);
        actualizar_todas_las_tablas_de_segmentos(tablas_de_segmentos_actualizadas);

        log_info(LOGGER_KERNEL, "Se finalizó el proceso de compactación");
        pthread_mutex_unlock(&SOLICITUD_FS);
        pthread_mutex_unlock(&SOLICITUD_MEMORIA);
        crear_segmento(proceso);
        break;
    case OUT_OF_MEMORY:
        pthread_mutex_unlock(&SOLICITUD_MEMORIA);
        terminar_proceso(proceso, "OUT_OF_MEMORY");
        break;
    }
}

// t_list -> [t_tablas_segmentos, t_tablas_segmentos, t_tablas_segmentos]
// t_tablas_segmentos -> int pid | t_list segmentos
void actualizar_todas_las_tablas_de_segmentos(t_list* nuevas_tablas){
    for (int i = 0; i < nuevas_tablas->elements_count; i++){
        t_tabla_segmentos* tabla_actualizada = list_get(nuevas_tablas, i);
        t_pcb* proceso = buscar_proceso(tabla_actualizada->PID);
        if (proceso != NULL){
            list_clean_and_destroy_elements(proceso->contexto->tabla_segmentos, free);
            proceso->contexto->tabla_segmentos = tabla_actualizada->segmentos;
        }
    }
}

void enviar_compactacion()
{
    t_paquete *paquete = crear_paquete(COMPACTAR);
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);
}

void eliminar_segmento(t_pcb *proceso)
{
    // enviar a memoria DELETE_SEGMENT con su parametro (id del segmento)
    pthread_mutex_lock(&SOLICITUD_MEMORIA);
    t_paquete *paquete = crear_paquete(DELETE_SEGMENT);
    serializar_contexto(proceso->contexto, paquete);
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);

    recibir_operacion(SOCKET_MEMORIA);
    t_list *tabla_segmentos_actualizada = recibir_tabla_segmentos(SOCKET_MEMORIA);

    //cambiar la tabla de segmentos del proceso por la nueva
    list_clean_and_destroy_elements(proceso->contexto->tabla_segmentos, free);
    proceso->contexto->tabla_segmentos = tabla_segmentos_actualizada;

    pthread_mutex_unlock(&SOLICITUD_MEMORIA);

    log_info(LOGGER_KERNEL, "PID: <%d> - Eliminar Segmento - Id Segmento: <%d>", proceso->contexto->PID, atoi(proceso->contexto->motivos_desalojo->parametros[0]));
}

t_pcb *buscar_proceso(int pid_buscado)
{
    int resulatdo_buesqueda; // en -1 entoces no lo encontro
    if (EJECUTANDO->contexto->PID == pid_buscado)
        return EJECUTANDO;

    resulatdo_buesqueda = buscar_ready(pid_buscado);
    if (resulatdo_buesqueda != -1)
    {
        return get_de_lista_ready(resulatdo_buesqueda);
    }
    t_pcb *proceso = buscar_block(pid_buscado);
    if (proceso != NULL)
        return proceso;

    resulatdo_buesqueda = buscar_block_fs(pid_buscado);
    if (resulatdo_buesqueda != -1)
        return get_de_lista_blockfs(resulatdo_buesqueda);

    log_info(LOGGER_KERNEL, "No hay proceso con PID: <%d> al que se le pueda actualizar la tabla de segmetnos", pid_buscado);

    return NULL;
}

int buscar_ready(int pid_buscado)
{
    int i = 0;
    t_pcb *proceso_aux;
    while (i < tamnio_lista_ready())
    {
        proceso_aux = get_de_lista_ready(i);
        if (proceso_aux->contexto->PID == pid_buscado)
            return i;
        i++;
    }
    return -1;
}
t_pcb *buscar_block(int pid_buscado)
{
    int i = 0;
    int j = 0;
    t_list *aux;
    t_pcb *proceso_aux;
    while (i < list_size(LISTAS_BLOCK))
    {
        aux = list_get(LISTAS_BLOCK, i);
        while (j < list_size(aux))
        {
            proceso_aux = list_get(aux, j);
            if (proceso_aux->contexto->PID == pid_buscado)
                return proceso_aux;
            j++;
        }
        j = 0;
        i++;
    }
    return NULL;
}
int buscar_block_fs(int pid_buscado)
{
    int i = 0;
    t_pcb *proceso_aux;
    while (i < tamnio_lista_blockfs())
    {
        proceso_aux = get_de_lista_blockfs(i);
        if (proceso_aux->contexto->PID == pid_buscado)
            return i;
        i++;
    }
    return -1;
}