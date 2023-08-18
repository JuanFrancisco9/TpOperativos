#include <init.h>

FILE* verificar_argumentos(int argc, char *argv[]) {
    // Verifico que se hayan ingresado los archivos de configuracion y de instrucciones
    if (argc < 3) {
        
        if (argc < 2) {
            log_error(logger_consola, "Debe ingresar el archivo de configuracion\n");
        }
        else {
            log_error(logger_consola, "Debe ingresar el archivo con las instrucciones\n");
        }
        
        return NULL;
    }

    FILE *f = fopen(argv[2], "r");
    if (f == NULL) {
        log_error(logger_consola, "No se pudo abrir el archivo de instrucciones");
        return NULL; 
    }

    return f;
}

void inicializar_conexiones(int *conexion_kernel, t_config* config) {
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    *conexion_kernel = inicializar_cliente(ip_kernel, puerto_kernel, logger_consola);
    
    if (*conexion_kernel == -1) {
        abort();
    }
}