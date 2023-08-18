#include <cpu.h>

int main(int argc, char *argv[]) {
  
    if (argc != 2) {
        printf("Debe ingresar el archivo de configuracion\n");
        exit(1);
    }


    t_config* config = iniciar_config(argv[1]);
    char* IP = "127.0.0.1";
    inicializar_variables_globales(config);
    
    int conexion_memoria;
    inicializar_conexiones(&conexion_memoria, config, LOGGER_CPU);

    char *puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int socket_servidor = crear_servidor(IP, puerto_escucha);

    
    conectar_kernel(socket_servidor);
    

    terminar_programa(LOGGER_CPU, config);
    terminar_conexiones(1, conexion_memoria);

    return 0;
}

