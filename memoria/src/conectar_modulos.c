#include "conectar_modulos.h"

void conectar_modulos(int socket_servidor)
{
    log_info(LOGGER_MEMORIA, "Esperando conexiones de modulos...");

    pthread_t hilo_kernel, hilo_cpu, hilo_fs;
    int socket_cpu, socket_fs, socket_cliente;

    for (int i = 0; i < 3; i++)
    {
        socket_cliente = esperar_cliente(socket_servidor);
        switch (recibir_operacion(socket_cliente))
        {
        case 0:
            log_info(LOGGER_MEMORIA, "Se conecto el file system");
            socket_fs = socket_cliente;
            pthread_create(&hilo_fs, NULL, (void *)recibir_fs, &socket_fs);
            pthread_detach(hilo_fs);
            break;
        case 1:
            log_info(LOGGER_MEMORIA, "Se conecto el kernel");
            SOCKET_KERNEL = socket_cliente;
            pthread_create(&hilo_kernel, NULL, (void *)recibir_kernel, &SOCKET_KERNEL);
            pthread_join(hilo_kernel, NULL);
            break;
        case 2:
            log_info(LOGGER_MEMORIA, "Se conecto el cpu");
            socket_cpu = socket_cliente;
            pthread_create(&hilo_cpu, NULL, (void *)recibir_cpu, &socket_cpu);
            pthread_detach(hilo_cpu);
            break;
        }
    }
}

void recibir_kernel(int *socket_modulo)
{
    while (1)
    {
        int cod_op = recibir_operacion(*socket_modulo);
        t_paquete *paquete;
        switch (cod_op)
        {
        case CREAR_TABLA_SEGMENTOS:
            // recibe
            int PID = recibir_int(*socket_modulo);

            // crea
            t_tabla_segmentos *tabla_segmentos = malloc(sizeof(t_tabla_segmentos));
            tabla_segmentos->PID = PID;
            tabla_segmentos->segmentos = crear_tabla_segmentos();
            list_add(TABLA_SEGMENTOS_GLOBAL, tabla_segmentos);
            log_info(LOGGER_MEMORIA, "Creación de Proceso PID: <%d>", PID);

            // envia
            paquete = crear_paquete(CREAR_TABLA_SEGMENTOS);
            serializar_tabla_segmentos(tabla_segmentos->segmentos, paquete);
            enviar_paquete(paquete, *socket_modulo);

            // libera
            eliminar_paquete(paquete);

            break;

        case TERMINAR:
            // recibe
            int size;
            void *buffer = recibir_buffer(&size, *socket_modulo);

            int *desplazamiento = malloc(sizeof(int));
            *desplazamiento = 0;

            PID = deserializar_int(buffer, desplazamiento);
            t_list* tabla_de_segmentos = deserializar_tabla_segmentos(buffer, desplazamiento);

            // eliminar
            free(buffer);
            free(desplazamiento);
            finalizar_proceso(tabla_de_segmentos, PID);

            log_info(LOGGER_MEMORIA, "Eliminacion de Proceso PID: <%d>", PID);
            enviar_mensaje("OK", *socket_modulo);
            break;

        case CREATE_SEGMENT:
            // recibe
            t_ctx *ctx = recibir_contexto(*socket_modulo);

            // crea
            paquete = crear_segmento(atoi(ctx->motivos_desalojo->parametros[0]), atoi(ctx->motivos_desalojo->parametros[1]), ctx);

            // envia
            enviar_paquete(paquete, *socket_modulo);

            // libera
            eliminar_paquete(paquete);
            liberar_contexto(ctx);
            break;

        case DELETE_SEGMENT:
            // recibe
            ctx = recibir_contexto(*socket_modulo); // Porque no está inicializada??

            // elimina
            eliminar_segmento(ctx->tabla_segmentos, atoi(ctx->motivos_desalojo->parametros[0]), ctx->PID);

            // envia
            paquete = crear_paquete(DELETE_SEGMENT);
            serializar_tabla_segmentos(ctx->tabla_segmentos, paquete);
            enviar_paquete(paquete, *socket_modulo);
            
            // libera
            eliminar_paquete(paquete);
            liberar_contexto(ctx);
            break;

        case COMPACTAR:
            compactar();
            paquete = crear_paquete(COMPACTAR);
            serializar_todas_las_tablas_segmentos(TABLA_SEGMENTOS_GLOBAL, paquete);
            enviar_paquete(paquete, *socket_modulo);
            eliminar_paquete(paquete);
            break;

        case -1:
            log_info(LOGGER_MEMORIA, "Se desconecto un modulo");
            return;

        default:
            //log_error(LOGGER_MEMORIA, "Operacion desconocida");
            break;
        }
    }
}
void recibir_cpu(int *socket_modulo)
{
    while (1)
    {
        int cod_op = recibir_operacion(*socket_modulo);
        switch (cod_op)
        {

        case MOV_IN:
            t_parametros_variables *parametros = NULL;
            int PID = -1;
            recibir_acceso(&parametros, &PID, *socket_modulo);
            char *valor_leido = leer_valor_direccion_fisica(strtol(parametros->parametros[1], NULL, 10), atoi(parametros->parametros[0]), PID, "CPU");
            enviar_mensaje(valor_leido, *socket_modulo);
            free(valor_leido);
            liberar_parametros_desalojo(parametros);
            break;

        case MOV_OUT:
            recibir_acceso(&parametros, &PID, *socket_modulo);
            escribir_valor_direccion_fisica(parametros->parametros[0], strtol(parametros->parametros[1], NULL, 10), PID, "CPU");
            enviar_mensaje("OK", *socket_modulo);
            liberar_parametros_desalojo(parametros);
            break;

        case -1:
            log_info(LOGGER_MEMORIA, "Se desconecto un modulo");
            return;

        default:
            log_error(LOGGER_MEMORIA, "Operacion desconocida");
            return;
        }
    }
}

void recibir_fs(int *socket_modulo)
{
    while (1)
    {
        int cod_op = recibir_operacion(*socket_modulo);
        switch (cod_op)
        {

        case F_READ:
            int PID = -1;
            t_parametros_variables *parametros = NULL;
            recibir_acceso(&parametros, &PID, *socket_modulo);
            escribir_valor_direccion_fisica(parametros->parametros[0], strtol(parametros->parametros[1], NULL, 10), PID, "FS");
            enviar_mensaje("OK", *socket_modulo);
            liberar_parametros_desalojo(parametros);

            break;

        case F_WRITE:
            recibir_acceso(&parametros, &PID, *socket_modulo);
            char *valor_leido = leer_valor_direccion_fisica(strtol(parametros->parametros[0], NULL, 10), atoi(parametros->parametros[1]), PID, "FS");
            enviar_mensaje(valor_leido, *socket_modulo);
            free(valor_leido);
            liberar_parametros_desalojo(parametros);

            break;

        case -1:
            log_info(LOGGER_MEMORIA, "Se desconecto un modulo");
            return;

        default:
            log_error(LOGGER_MEMORIA, "Operacion desconocida");
            return;
        }
    }
}

t_ctx *recibir_contexto(int socket)
{
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    t_ctx *ctx = deserializar_contexto(buffer, desplazamiento);

    free(desplazamiento);
    free(buffer);
    return ctx;
}

t_parametros_variables *recibir_parametros_variables(int socket)
{
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = sizeof(int);

    t_parametros_variables *parametros = deserealizar_motivos_desalojo(buffer, desplazamiento);

    free(buffer);
    free(desplazamiento);
    return parametros;
}

void recibir_acceso(t_parametros_variables **parametros, int *PID, int socket){
    int size;
    void *buffer = recibir_buffer(&size, socket);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    *PID = deserializar_int(buffer, desplazamiento);
    *parametros = deserealizar_motivos_desalojo(buffer, desplazamiento);

    free(buffer);
    free(desplazamiento);
}
