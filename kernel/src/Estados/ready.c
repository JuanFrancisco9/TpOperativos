#include "ready.h"

void admitir_proceso()
{
    t_pcb * proceso_en_new = sacar_de_lista_new(0);
    
    pthread_mutex_lock(&SOLICITUD_MEMORIA);

    t_paquete* paquete = crear_paquete(CREAR_TABLA_SEGMENTOS);
    agregar_a_paquete_dato_serializado(paquete, &proceso_en_new->contexto->PID, sizeof(int));
    enviar_paquete(paquete, SOCKET_MEMORIA);
    eliminar_paquete(paquete);

    recibir_operacion(SOCKET_MEMORIA);
    t_list* tabla_segmentos = recibir_tabla_segmentos(SOCKET_MEMORIA);
    
    pthread_mutex_unlock(&SOLICITUD_MEMORIA);
    
    cambio_de_estado(proceso_en_new->contexto->PID,"New","Ready");
    
    proceso_en_new->contexto->tabla_segmentos = tabla_segmentos;
    agregar_a_lista_ready(proceso_en_new);

}

t_pcb *get_proceso_por_fifo()
{
    return sacar_de_lista_ready(0);
}

t_pcb *get_proceso_por_hrrn()
{
    t_pcb *proceso = get_de_lista_ready(0);
    float tiempo_actual = temporal_gettime(TIEMPO_CORRIENDO);
    float RR = 1 + ((tiempo_actual - proceso->tiempo_llegada_ready) / proceso->estimado_prox_rafaga);
    // tiempo_actual-proceso->tiempo_llegada_ready me da el tiempo de espera en ready
    // si pasa a ready en t=3 y replanifio en t=5 entonces tiempo espera es 5(tiempo_cuatal)- 3(tiempo_llegada_ready)
    int i = 0;
    float RR_aux;
    t_pcb *proceso_aux;

    while (i < tamnio_lista_ready())
    {
        proceso_aux = get_de_lista_ready(i);
        RR_aux = 1 + ((tiempo_actual - proceso_aux->tiempo_llegada_ready) / proceso_aux->estimado_prox_rafaga);
        if (RR_aux > RR)
        {
            proceso = proceso_aux;
        }
        i++;
    }
    sacar_elemento_de_lista_ready(proceso);
    return proceso;
}

t_pcb *ceder_proceso_a_exec()
{

    if (strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0)
    {
        return get_proceso_por_fifo();
    }
    else if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0)
    {
        return get_proceso_por_hrrn();
    }
    else
    {
        log_error(LOGGER_KERNEL, "No se reconoce el algoritmo de planificacion <%s>", ALGORITMO_PLANIFICACION);
        exit(EXIT_FAILURE);
    }
}