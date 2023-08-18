#include <init.h>

void inicializar_conexiones(int *conexion_memoria, t_config* config, t_log* logger) {
    char *ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char *puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    *conexion_memoria = inicializar_cliente(ip_memoria, puerto_memoria, logger);
    SOCKET_MEMORIA = *conexion_memoria;
    int *identificador = malloc(sizeof(int));
    *identificador = 2;
    send(*conexion_memoria, identificador, sizeof(int), 0);
    free(identificador);
}

void inicializar_variables_globales(t_config* config) {

    LOGGER_CPU = iniciar_logger("./cpu.log", "CPU");
    TIEMPO_RETARDO = atoi(config_get_string_value(config, "RETARDO_INSTRUCCION"));
    TAM_MAX_SEGMENTO = atoi(config_get_string_value(config, "TAM_MAX_SEGMENTO"));
}