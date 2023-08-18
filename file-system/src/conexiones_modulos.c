#include <conexiones_modulos.h>

void conectar_kernel(int socket_servidor)
{
    log_info(LOGGER_FILE_SYSTEM, "Esperando kernel....");
    SOCKET_KERNEL = esperar_cliente(socket_servidor);
    log_info(LOGGER_FILE_SYSTEM, "Se conecto el kernel");
    enviado_de_kernel(&SOCKET_KERNEL);
}

void enviado_de_kernel(int *socket_kernel)
{
    while (1)
    {
        int cod_op = recibir_operacion(*socket_kernel);

        if (cod_op == -1)
        {
            log_info(LOGGER_FILE_SYSTEM, "Se desconecto el kernel");
            return;
        }

        int pid;
        t_parametros_variables *parametros_instruccion = recibir_paquete_kernel(*socket_kernel, &pid);
        atender_solicitudes(cod_op, parametros_instruccion, pid);
        liberar_parametros_desalojo(parametros_instruccion);
    }
}

t_parametros_variables *recibir_paquete_kernel(int socket_kernel, int* pid)
{
    int size;
    void *buffer = recibir_buffer(&size, socket_kernel);

    int *desplazamiento = malloc(sizeof(int));
    *desplazamiento = 0;

    memcpy(pid, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    t_parametros_variables *parametros = deserealizar_motivos_desalojo(buffer, desplazamiento);

    free(buffer);
    free(desplazamiento);

    return parametros;
}

char* leer_direccion_de_memoria(char *direccion_fisica, char* tamanio, int pid)
{
    t_parametros_variables *parametros = malloc(sizeof(t_parametros_variables));
    parametros->parametros = NULL;
    agregar_parametro_variable(parametros, direccion_fisica);
    agregar_parametro_variable(parametros, tamanio);

    t_paquete *paquete = crear_paquete(F_WRITE);
    agregar_a_paquete_dato_serializado(paquete, &pid, sizeof(int));
    serializar_motivos_desalojo(parametros, paquete);
    enviar_paquete(paquete, SOCKET_MEMORIA);

    liberar_parametros_desalojo(parametros);
    eliminar_paquete(paquete);


    recibir_operacion(SOCKET_MEMORIA);
    return recibir_mensaje(SOCKET_MEMORIA);
}

void escribir_valor_en_memoria(char *direccion_fisica, char *contenido, int pid)
{
    t_parametros_variables *parametros = malloc(sizeof(t_parametros_variables));
    parametros->parametros = NULL;
    agregar_parametro_variable(parametros, contenido);
    agregar_parametro_variable(parametros, direccion_fisica);

    t_paquete *paquete = crear_paquete(F_READ);
    agregar_a_paquete_dato_serializado(paquete, &pid, sizeof(int));
    serializar_motivos_desalojo(parametros, paquete);
    enviar_paquete(paquete, SOCKET_MEMORIA);

    liberar_parametros_desalojo(parametros);
    eliminar_paquete(paquete);

    recibir_operacion(SOCKET_MEMORIA);
    char *mensaje = recibir_mensaje(SOCKET_MEMORIA);

    if (strcmp(mensaje, "OK"))
        log_error(LOGGER_FILE_SYSTEM, "No se pudo escribir la direccion de memoria");

    free(mensaje);
}


void enviar_paquete_op_terminada(char* nombre_archivo){
    log_info(LOGGER_FILE_SYSTEM, "Operacion Terminada");
    t_paquete* paquete = crear_paquete(OP_TERMINADA);
    int len = strlen(nombre_archivo) + 1;
    agregar_a_paquete_dato_serializado(paquete, &len, sizeof(int));
    agregar_a_paquete_dato_serializado(paquete, nombre_archivo, len);
    enviar_paquete(paquete, SOCKET_KERNEL);
    eliminar_paquete(paquete);
}

void crear_y_enviar_paquete(int cod_op){
    t_paquete* paquete = crear_paquete(cod_op);
    enviar_paquete(paquete, SOCKET_KERNEL);
    eliminar_paquete(paquete);
}


