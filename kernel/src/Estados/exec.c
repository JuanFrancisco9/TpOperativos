#include "exec.h"

void reemplazar_proceso(t_pcb *nuevo_pcb)
{

    EJECUTANDO = nuevo_pcb;
}

void reemplazar_ctx(t_ctx *nuevo_ctx)
{
    list_destroy_and_destroy_elements(EJECUTANDO->contexto->tabla_segmentos, (void *)liberar_segmento);
    liberar_contexto(EJECUTANDO->contexto);
    EJECUTANDO->contexto = nuevo_ctx;
}

void enviar_a_cpu()
{
    t_paquete *paquete = crear_paquete(CONTEXTO);

    serializar_contexto(EJECUTANDO->contexto, paquete);

    enviar_paquete(paquete, SOCKET_CPU);

    eliminar_paquete(paquete);

    log_info(LOGGER_KERNEL, "Se envia el proceso PID: <%d> al CPU", EJECUTANDO->contexto->PID);

    if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0)
    {
        TIEMPO_EN_CPU = temporal_create();
    }
}

void empezar_ciclo()
{
    EJECUTANDO = ceder_proceso_a_exec();
    enviar_a_cpu();
}

void definir_accion(int cod_op, t_pcb *proceso)
{
    switch (cod_op)
    {

    case YIELD:
        log_info(LOGGER_KERNEL, "Yield PID: <%d>", proceso->contexto->PID);
        cambio_de_estado(proceso->contexto->PID, "Exec", "Ready");
        agregar_a_lista_ready(proceso);
        reemplazar_exec_por_nuevo();
        break;
    case EXIT:
        cambio_de_estado(proceso->contexto->PID, "Exec", "Exit");
        terminar_proceso(proceso, "SUCCESS");
        break;

    case SEG_FAULT:
        cambio_de_estado(proceso->contexto->PID, "Exec", "Exit");
        terminar_proceso(proceso, "SEG_FAULT");
        break;

    case WAIT:
        wait(proceso, proceso->contexto->motivos_desalojo->parametros[0]);
        break;
    case SIGNAL:
        signal(proceso, proceso->contexto->motivos_desalojo->parametros[0]);
        break;
    case IO:
        io(proceso);
        break;
    case F_OPEN:
        // si esta en mi tabla lo bloqueo
        // se le pregunta a fs si existe
        // si existe se crea la entrada
        // si no se le pide que lo cree y dsp se crea la entrada
        if (f_open(proceso, proceso->contexto->motivos_desalojo->parametros[0]))
        { // 1 bloqueado, 0 desbloqueado

            reemplazar_exec_por_nuevo();
        }
        break;
    case F_SEEK:
        f_seek(proceso, proceso->contexto->motivos_desalojo->parametros[0], proceso->contexto->motivos_desalojo->parametros[1]);
        break;

    case F_CLOSE:
        f_close(proceso, proceso->contexto->motivos_desalojo->parametros[0]);
        break;

    case F_READ:

        f_read(proceso, proceso->contexto->motivos_desalojo->parametros[0]);

        reemplazar_exec_por_nuevo();
        break;

    case F_WRITE:
        f_write(proceso, proceso->contexto->motivos_desalojo->parametros[0]);
        reemplazar_exec_por_nuevo();
        break;

    case F_TRUNCATE:
        f_truncate(proceso, proceso->contexto->motivos_desalojo->parametros[0], proceso->contexto->motivos_desalojo->parametros[1]);
        reemplazar_exec_por_nuevo();
        break;
    case CREATE_SEGMENT:
        crear_segmento(proceso);
        break;
    case DELETE_SEGMENT:
        eliminar_segmento(proceso);
        break;
    default:
        log_info(LOGGER_KERNEL, "No implementamos esta función");
        break;
    }
}

void reemplazar_exec_por_nuevo()
{
    if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0)
    {
        if(EJECUTANDO !=NULL){estimado_prox_rafaga();
        }
        else{
            temporal_destroy(TIEMPO_EN_CPU);
        }
    }
    t_pcb *proceso_entrante = ceder_proceso_a_exec(); // pide un proceso a ready segun el algoritmo
    reemplazar_proceso(proceso_entrante);
    cambio_de_estado(proceso_entrante->contexto->PID, "Ready", "Exec");
}

void recibir_de_cpu(int conexion_cpu)
{

    int cod_op = recibir_operacion(conexion_cpu);

    int size;
    void *buffer = recibir_buffer(&size, conexion_cpu);
    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    t_ctx *ctx = deserializar_contexto(buffer, desplazamiento);
    free(buffer);
    free(desplazamiento);
    log_info(LOGGER_KERNEL, "Se recibe de CPU el proceso PID: <%d>", ctx->PID);
    reemplazar_ctx(ctx);

    definir_accion(cod_op, EJECUTANDO);

    vaciar_parametros_desalojo(EJECUTANDO->contexto->motivos_desalojo);

    enviar_a_cpu();
}
void estimado_prox_rafaga()
{
    int64_t tiempo_en_cpu = temporal_gettime(TIEMPO_EN_CPU);
    temporal_destroy(TIEMPO_EN_CPU);

    float proxima_rafaga = HRRN_ALFA * tiempo_en_cpu + (1 - HRRN_ALFA) * EJECUTANDO->estimado_prox_rafaga;

    EJECUTANDO->estimado_prox_rafaga = proxima_rafaga;

    log_info(LOGGER_KERNEL, "Se realizo el estimado de proxima rafaga para el PID: <%d>, nuevo estimado: %f", EJECUTANDO->contexto->PID, EJECUTANDO->estimado_prox_rafaga);
}