#include <memoria.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Debe ingresar el archivo de configuracion\n");
        exit(1);
    }

    LOGGER_MEMORIA = iniciar_logger("./memoria.log", "MEMORIA");
    t_config* config = iniciar_config(argv[1]);
    char* IP = "127.0.0.1";
    iniciar_estructuras(config);
    
    char *puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int socket_servidor = crear_servidor(IP, puerto_escucha);
    conectar_modulos(socket_servidor);

    terminar_programa(LOGGER_MEMORIA, config);

    return 0;
}
