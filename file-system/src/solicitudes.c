#include <solicitudes.h>

char *nombre_archivo;
void *archivo_de_bloques;
int posicion_archivo;
char *memoria;

void atender_solicitudes(int cod_op, t_parametros_variables *parametros_instruccion, int pid)
{

    nombre_archivo = parametros_instruccion->parametros[0];

    switch (cod_op)
    {
    case F_CREATES:
        log_info(LOGGER_FILE_SYSTEM, "Crear Archivo: %s", nombre_archivo);
        crear_fcb(nombre_archivo);

        // Devolver un existe, lo hicimos asi
        crear_y_enviar_paquete(EXISTE);
        break;

    case F_OPEN:
        log_info(LOGGER_FILE_SYSTEM, "Abrir Archivo: %s", nombre_archivo);

        if (dictionary_get(DICCIONARIO_FCB, nombre_archivo) == NULL)
        {
            crear_y_enviar_paquete(NO_EXISTE);
        }
        else
        {
            crear_y_enviar_paquete(EXISTE);
        }
        break;

    case F_TRUNCATE:
        int tamanio_nuevo = atoi(parametros_instruccion->parametros[1]);
        int tamanio_actual = obtener_tamanio(nombre_archivo);
        actualizar_fcb(nombre_archivo, "TAMANIO_ARCHIVO", parametros_instruccion->parametros[1]);
        log_info(LOGGER_FILE_SYSTEM, "Truncar Archivo: %s - Tamaño: %d", nombre_archivo, tamanio_nuevo);

        // No se tiene en cuenta el puntero indirecto
        int cantidad_bloques_actual = (tamanio_actual + TAMANIO_BLOQUES - 1) / TAMANIO_BLOQUES;
        int cantidad_bloques_necesarios = (tamanio_nuevo + TAMANIO_BLOQUES - 1) / TAMANIO_BLOQUES;

        if (cantidad_bloques_actual == cantidad_bloques_necesarios)
        {
            // No se hace nada
            log_info(LOGGER_FILE_SYSTEM, "No se modifican bloques");
            enviar_paquete_op_terminada(nombre_archivo);
            break;
        }

        archivo_de_bloques = mmap(NULL, (TAMANIO_BLOQUES * CANTIDAD_BLOQUES), PROT_READ | PROT_WRITE, MAP_SHARED, fileno(ARCHIVO_BLOQUES), 0);

        // Se agranda el archivo
        if (cantidad_bloques_necesarios > cantidad_bloques_actual)
        {
            int cantidad_bloques_a_agregar = cantidad_bloques_necesarios - cantidad_bloques_actual;

            agrandar_archivo(archivo_de_bloques, cantidad_bloques_a_agregar, cantidad_bloques_actual, tamanio_actual == 0);
        }

        // Se achica el archivo
        else
        {
            int cantidad_bloques_a_liberar = cantidad_bloques_actual - cantidad_bloques_necesarios;

            achicar_archivo(archivo_de_bloques, cantidad_bloques_a_liberar, cantidad_bloques_actual, tamanio_nuevo == 0);
        }

        msync(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES, MS_SYNC);
        munmap(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES);
        enviar_paquete_op_terminada(nombre_archivo);
        break;

    case F_WRITE:
    case F_READ:
        char *memoria = parametros_instruccion->parametros[1];
        int posicion = atoi(parametros_instruccion->parametros[3]);
        int tamaño = atoi(parametros_instruccion->parametros[2]);

        if (cod_op == F_WRITE)
            f_write(memoria, posicion, tamaño, pid);

        else
            f_read(memoria, posicion, tamaño, pid);

        enviar_paquete_op_terminada(nombre_archivo);
        break;

    default:
        log_error(LOGGER_FILE_SYSTEM, "Operacion desconocida");
        return;
    }
}

void agrandar_archivo(void *archivo_de_bloques, int cantidad_bloques_a_agregar, int actuales, bool nuevo_archivo)
{

    // Si es la primera vez que se llama a truncate
    // Se asigna el puntero directo e indirecto
    if (nuevo_archivo)
    {
        asignar_puntero_directo(nombre_archivo);
        asignar_puntero_indirecto(nombre_archivo);
        cantidad_bloques_a_agregar--;
    }

    if (cantidad_bloques_a_agregar > 0)
    {

        cargar_puntero_indirecto(nombre_archivo);
        asignar_bloques_al_puntero_indirecto(nombre_archivo, cantidad_bloques_a_agregar, actuales);
    }
}

void achicar_archivo(void *archivo_de_bloques, int cantidad_bloques_a_liberar, int actuales, bool borrar_todo)
{
    if (borrar_todo)
    {
        liberar_puntero_directo(nombre_archivo);
        cantidad_bloques_a_liberar--;
    }

    if (cantidad_bloques_a_liberar > 0)
    {
        cargar_puntero_indirecto(nombre_archivo);
        liberar_bloques_del_puntero_indirecto(nombre_archivo, cantidad_bloques_a_liberar, actuales);
    }

    if (borrar_todo)
    {
        liberar_puntero_indirecto(nombre_archivo);
    }
}

void f_write(char *direccion, int posicion_archivo, int tamanio_a_escribir, int pid)
{
    long dir_fisica = strtol(direccion, NULL, 10); 
    
    log_info(LOGGER_FILE_SYSTEM, "Escribir Archivo: %s - Puntero: %d - Memoria: %p - Tamaño: %d", nombre_archivo, posicion_archivo, (void *)dir_fisica, tamanio_a_escribir);

    char *tamanio_a_escribir_string = string_itoa(tamanio_a_escribir);
    char *valor_leido = leer_direccion_de_memoria(direccion, tamanio_a_escribir_string, pid);
    free(tamanio_a_escribir_string);
    log_info(LOGGER_FILE_SYSTEM, "Valor leido de Memoria: %s", valor_leido);

    if (tamanio_a_escribir + posicion_archivo > 64)
    {
        // Se carga el puntero indirecto en la memoria
        cargar_puntero_indirecto(nombre_archivo);
    }

    void *contenido = malloc(tamanio_a_escribir);
    memcpy(contenido, valor_leido, tamanio_a_escribir);
    free(valor_leido);

    void *contenido_original = contenido; // Mantener una copia del puntero original

    while (tamanio_a_escribir > 0)
    {
        int tamanio_a_escribir_del_bloque = MIN(tamanio_a_escribir, TAMANIO_BLOQUES - posicion_archivo % TAMANIO_BLOQUES);
        int bloque_archivo = posicion_archivo / TAMANIO_BLOQUES;
        int offset = posicion_archivo % TAMANIO_BLOQUES;

        escribir_bloque(nombre_archivo, bloque_archivo, offset, tamanio_a_escribir_del_bloque, contenido);
        contenido = contenido + tamanio_a_escribir_del_bloque;

        posicion_archivo += tamanio_a_escribir_del_bloque;
        tamanio_a_escribir -= tamanio_a_escribir_del_bloque;
    }

    contenido = contenido_original; // Restaurar el puntero original antes de liberar la memoria
    free(contenido);
    munmap(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES);
}

void f_read(char *direccion, int posicion_archivo, int tamanio_a_leer, int pid)
{
    void *buffer = malloc(tamanio_a_leer);

    long dir_fisica = strtol(direccion, NULL, 10); 
    
    log_info(LOGGER_FILE_SYSTEM, "Leer Archivo: %s - Puntero: %d - Memoria: %p - Tamaño: %d", nombre_archivo, posicion_archivo, (void *)dir_fisica, tamanio_a_leer);

    if (tamanio_a_leer + posicion_archivo > 64)
    {
        // Se carga el puntero indirecto en la memoria
        cargar_puntero_indirecto(nombre_archivo);
    }

    int tamanio_total = tamanio_a_leer;

    while (tamanio_a_leer > 0)
    {
        int tamanio_a_leer_del_bloque = MIN(tamanio_a_leer, TAMANIO_BLOQUES - posicion_archivo % TAMANIO_BLOQUES);
        int bloque_archivo = posicion_archivo / TAMANIO_BLOQUES;
        int offset = posicion_archivo % TAMANIO_BLOQUES;

        void *leido = leer_bloque(nombre_archivo, bloque_archivo, offset, tamanio_a_leer_del_bloque);
        memcpy(buffer + (tamanio_total - tamanio_a_leer), leido, tamanio_a_leer_del_bloque);
        free(leido);

        
        posicion_archivo += tamanio_a_leer_del_bloque;
        tamanio_a_leer -= tamanio_a_leer_del_bloque;
    }

    char *buffer_como_char = malloc(tamanio_total + 1);
    memcpy(buffer_como_char, buffer, tamanio_total);
    buffer_como_char[tamanio_total] = '\0';

    log_info(LOGGER_FILE_SYSTEM, "Valor Leido de File System: %s", buffer_como_char);
    escribir_valor_en_memoria(direccion, buffer_como_char, pid);
    free(buffer);
    free(buffer_como_char);

    munmap(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES);
}
