#include "exit.h"

void liberar_recursos(t_pcb *proceso)
{

    if (!list_is_empty(proceso->recursos_en_uso))
    {
        int i;
        char *aux;
        for (i = 0; i < list_size(proceso->recursos_en_uso); i++)
        {
            aux = list_get(proceso->recursos_en_uso, i);
            signal(proceso, aux);
        }
    }
    list_destroy(proceso->recursos_en_uso);
}

void terminar_proceso(t_pcb *proceso, char* motivo)
{
    int pid = proceso->contexto->PID;
    
    // Revisar si tiene un recurso asignado y eliminarlo
    liberar_recursos(proceso);

    // Enviar a memoria que se terminó el proceso
    liberar_segmentos_de_memoria(proceso);

    // Enviar a consola que se terminó el proceso
    finalizar_consola(proceso->socket_consola);

    // destruir proceso
    list_destroy_and_destroy_elements(EJECUTANDO->contexto->tabla_segmentos, (void *)liberar_segmento);
    liberar_contexto(proceso->contexto);
    list_destroy(proceso->archivos_abiertos);
    free(proceso);
    log_info(LOGGER_KERNEL, "Finaliza el proceso <%d> - Motivo: <%s>", pid, motivo);

    sem_post(&GRADO_MULTIPROGRAMACION);
    EJECUTANDO = NULL;
    reemplazar_exec_por_nuevo();
}

void finalizar_consola(int socket_consola)
{
    t_paquete *paquete = crear_paquete(TERMINAR);
    enviar_paquete(paquete, socket_consola);
    eliminar_paquete(paquete);
}

void liberar_segmentos_de_memoria(t_pcb *proceso)
{
    pthread_mutex_lock(&SOLICITUD_MEMORIA);
    t_paquete *paquete = crear_paquete(TERMINAR);
    agregar_a_paquete_dato_serializado(paquete, &proceso->contexto->PID, sizeof(int));
    serializar_tabla_segmentos(proceso->contexto->tabla_segmentos, paquete);
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);

    recibir_operacion(SOCKET_MEMORIA);

    char *buffer_temporal = recibir_mensaje(SOCKET_MEMORIA);
    if (string_equals_ignore_case(buffer_temporal, "OK"))
    {
        log_info(LOGGER_KERNEL, "Se eliminaron los segmentos de memoria del proceso PID: <%d>", proceso->contexto->PID);
    }
    pthread_mutex_unlock(&SOLICITUD_MEMORIA);
}
