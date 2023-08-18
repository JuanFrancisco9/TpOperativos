#include "blocked.h"
// tiene que esperar todos los signal de los semaforos en SEMAFOROS_RECURSOS

pthread_t hacer_IO;

void wait(t_pcb *proceso, char *nombre_recurso)
{
    int recurso_id = get_id_recurso(nombre_recurso);
    if (recurso_id != -1)
    {
        if (INSTANCIAS_RECURSOS[recurso_id] == 0)
        { // no hay recursos disponibles para darle
            log_info(LOGGER_KERNEL, "PID: <%d> - Bloqueado por: < %s >", proceso->contexto->PID, RECURSOS[recurso_id]);
            cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
            reemplazar_exec_por_nuevo();
            list_add(list_get(LISTAS_BLOCK, recurso_id), proceso);
        }
        else
        {
            // sem_wait(SEMAFOROS_RECURSOS[recurso_id]);
            list_add(proceso->recursos_en_uso, RECURSOS[recurso_id]);
            INSTANCIAS_RECURSOS[recurso_id]--;
        }
        log_info(LOGGER_KERNEL, "PID: <%d> - Wait: <%s> - Instancias: <%d>", proceso->contexto->PID, RECURSOS[recurso_id], INSTANCIAS_RECURSOS[recurso_id]);
    }
    else
    {
        log_info(LOGGER_KERNEL, "PID: <%d> realiza Wait de recurso inexistente: < %s > ", proceso->contexto->PID, proceso->contexto->motivos_desalojo->parametros[0]);
        cambio_de_estado(proceso->contexto->PID, "Exec", "Exit");
        terminar_proceso(proceso, "INVALID_RESOURCE");
    }
}
void signal(t_pcb *proceso, char *nombre_recurso)
{
    int recurso_id = get_id_recurso(nombre_recurso);
    if (recurso_id != -1)
    {
        t_list *lista_del_recurso = list_get(LISTAS_BLOCK, recurso_id);

        if (list_size(lista_del_recurso) > 0)
        {
            // desbloquear proceso
            list_remove_element(proceso->recursos_en_uso, RECURSOS[recurso_id]);
            t_pcb *proceso_a_desbloquear = list_remove(list_get(LISTAS_BLOCK, recurso_id), 0);
            list_add(proceso_a_desbloquear->recursos_en_uso, RECURSOS[recurso_id]);
            cambio_de_estado(proceso_a_desbloquear->contexto->PID, "Block", "Ready");
            agregar_a_lista_ready(proceso_a_desbloquear);
        }
        else
        {
            INSTANCIAS_RECURSOS[recurso_id]++;
            list_remove_element(proceso->recursos_en_uso, RECURSOS[recurso_id]);
            log_info(LOGGER_KERNEL, "PID: <%d> - Signal: <%s> - Instancias: <%d>", proceso->contexto->PID, RECURSOS[recurso_id], INSTANCIAS_RECURSOS[recurso_id]);
        }
    }
    else
    {
        log_info(LOGGER_KERNEL, "PID: <%d> realiza Signal de recurso inexistente: <%s>", proceso->contexto->PID,proceso->contexto->motivos_desalojo->parametros[0]);
        cambio_de_estado(proceso->contexto->PID, "Exec", "Exit");
        terminar_proceso(proceso, "INVALID_RESOURCE");
    }
}

void *instruccion_IO(t_pcb *proceso)
{
    t_instruccion *instruccion_utilizable = list_get(proceso->contexto->instrucciones, proceso->contexto->program_counter - 1);
    int tiempo = atoi(instruccion_utilizable->parametros[0]);
    log_info(LOGGER_KERNEL, "PID: <%d> - Ejecuta IO: %d", proceso->contexto->PID, tiempo);
    cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
    usleep(tiempo * 1000000);
    log_info(LOGGER_KERNEL, "PID: <%d> -Finaliza IO", proceso->contexto->PID);
    cambio_de_estado(proceso->contexto->PID, "Block", "Ready");
    agregar_a_lista_ready(proceso);
    return NULL;
}

void io(t_pcb *proceso)
{
    pthread_create(&hacer_IO, NULL, (void *)instruccion_IO, proceso);
    pthread_detach(hacer_IO);
    reemplazar_exec_por_nuevo();
}

int get_id_recurso(char String[])
{
    int i;
    for (i = 0; i < string_array_size(RECURSOS); i++)
    {
        if (strcmp(String, RECURSOS[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}
