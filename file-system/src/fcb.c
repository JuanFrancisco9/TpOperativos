#include <fcb.h>

char *path_fcb;

void levantar_diccionario_fcb(t_config *config)
{
    path_fcb = config_get_string_value(config, "PATH_FCB");

    DICCIONARIO_FCB = dictionary_create();
    DIR *directorio = opendir(path_fcb);
    if (directorio == NULL)
    {
        log_error(LOGGER_FILE_SYSTEM, "Error al abrir el directorio de FCB.\n");
        abort();
    }

    struct dirent *archivo;
    while ((archivo = readdir(directorio)) != NULL)
    {
        if (archivo->d_type == DT_REG)
        {
            // Solo procesar archivos regulares (ignorar directorios y otros tipos)
            char* ruta_archivo = string_from_format("%s/%s", path_fcb, archivo->d_name);
            char* nombre_archivo = string_from_format("%s", archivo->d_name);

            t_config *config_archivo = iniciar_config(ruta_archivo);
            dictionary_put(DICCIONARIO_FCB, nombre_archivo, config_archivo);

            // Realizar operaciones con el archivo
            // log_info(LOGGER_FILE_SYSTEM, "Archivo encontrado: %s\n", ruta_archivo);

            free(ruta_archivo);
            free(nombre_archivo);
        }
    }
    closedir(directorio);
}

void crear_fcb(char *nombre_archivo)
{

    if (dictionary_has_key(DICCIONARIO_FCB, nombre_archivo))
    {
        log_error(LOGGER_FILE_SYSTEM, "El archivo %s ya existe.\n", nombre_archivo);
    }

    else
    {
        char *path_archivo = string_from_format("%s/%s", path_fcb, nombre_archivo);

        t_config *nuevo = iniciar_config(path_archivo);

        config_set_value(nuevo, "NOMBRE_ARCHIVO", nombre_archivo);
        config_set_value(nuevo, "TAMANIO_ARCHIVO", "0");

        config_save(nuevo);

        dictionary_put(DICCIONARIO_FCB, nombre_archivo, nuevo);
        free(path_archivo);
    }
}

void asignar_puntero_directo(char *nombre_archivo)
{
    log_info(LOGGER_FILE_SYSTEM, "Asignacion de puntero directo - Archivo: %s", nombre_archivo);
    char* puntero_directo = string_itoa(buscar_bloque_libre());

    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    config_set_value(fcb, "PUNTERO_DIRECTO", puntero_directo);
    config_save(fcb);

    free(puntero_directo);
}

void asignar_puntero_indirecto(char *nombre_archivo)
{
    log_info(LOGGER_FILE_SYSTEM, "Asignacion de puntero indirecto - Archivo: %s", nombre_archivo);
    char* puntero_indirecto = string_itoa(buscar_bloque_libre());
    
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    config_set_value(fcb, "PUNTERO_INDIRECTO", puntero_indirecto);
    config_save(fcb);

    free(puntero_indirecto);
}

void liberar_puntero_directo(char *nombre_archivo)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    marcar_bloque_como_libre(config_get_int_value(fcb, "PUNTERO_DIRECTO"));
    config_remove_key(fcb, "PUNTERO_DIRECTO");
    config_save(fcb);

    // free(fcb);
}

void liberar_puntero_indirecto(char *nombre_archivo)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    marcar_bloque_como_libre(config_get_int_value(fcb, "PUNTERO_INDIRECTO"));
    config_remove_key(fcb, "PUNTERO_INDIRECTO");
    config_save(fcb);

    // free(fcb);
}

void actualizar_fcb(char *nombre_archivo, char *clave, char *valor)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    config_set_value(fcb, clave, valor);
    config_save(fcb);
}

int obtener_tamanio(char *nombre_archivo)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    return config_get_int_value(fcb, "TAMANIO_ARCHIVO");
}

int obtener_puntero_indirecto(char *nombre_archivo)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    return config_get_int_value(fcb, "PUNTERO_INDIRECTO");
}

int obtener_puntero_directo(char *nombre_archivo)
{
    t_config *fcb = dictionary_get(DICCIONARIO_FCB, nombre_archivo);
    return config_get_int_value(fcb, "PUNTERO_DIRECTO");
}
