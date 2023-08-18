#include <file-system.h>

int main(int argc, char *argv[])
{

    LOGGER_FILE_SYSTEM = iniciar_logger("./file-system.log", "FILE SYSTEM");

    if (argc != 2)
    {
        log_error(LOGGER_FILE_SYSTEM, "Debe ingresar el archivo de configuracion\n");
        return EXIT_FAILURE;
    }

    t_config *config = iniciar_config(argv[1]);
    char* IP = "127.0.0.1";

    inicializar_conexiones(&SOCKET_MEMORIA, config);
    
    inicializar_archivos(config);
    // Caragamos todos los fcb en la memoria
    levantar_diccionario_fcb(config);

  
    char *puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int socket_servidor = crear_servidor(IP, puerto_escucha);

    conectar_kernel(socket_servidor);

    terminar_programa(LOGGER_FILE_SYSTEM, config);
    finalizar();
    terminar_conexiones(1, SOCKET_MEMORIA);
    

    return 0;
}