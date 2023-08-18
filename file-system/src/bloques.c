#include <bloques.h>

u_int32_t buscar_bloque_libre()
{
    int i, estado;
    for (i = 0; i < bitarray_get_max_bit(BITMAP_BLOQUES); i++)
    {

        estado = bitarray_test_bit(BITMAP_BLOQUES, i);
        log_info(LOGGER_FILE_SYSTEM, "Acceso a Bitmap - Bloque: %d - Estado: %d", i, estado);
        if (!estado)
        {
            bitarray_set_bit(BITMAP_BLOQUES, i);
            // msync(BITMAP_BLOQUES->bitarray, BITMAP_BLOQUES->size, MS_SYNC);

            return i;
        }
    }
    return -1;
}

void marcar_bloque_como_libre(uint32_t bloque)
{

    log_info(LOGGER_FILE_SYSTEM, "Acceso a Bitmap - Se marca al Bloque: %d como Libre", bloque);
    bitarray_clean_bit(BITMAP_BLOQUES, bloque);
    // msync(BITMAP_BLOQUES->bitarray, BITMAP_BLOQUES->size, MS_SYNC);
}

void asignar_bloques_al_puntero_indirecto(char *archivo, int cantidad_bloques_necesarios, int bloques_actuales)
{

    u_int32_t bloque_libre;

    void *puntero = mmap(NULL, TAMANIO_BLOQUES, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(ARCHIVO_BLOQUES), 0);
    puntero = puntero + obtener_puntero_indirecto(archivo) * TAMANIO_BLOQUES;

    // Como el primer bloque es el puntero directo es - 1
    int punteros_actuales = MAX(bloques_actuales - 1, 0);
    for (int i = 0; i < cantidad_bloques_necesarios; i++)
    {
        bloque_libre = buscar_bloque_libre();
        if (bloque_libre == -1)
        {
            log_error(LOGGER_FILE_SYSTEM, "No hay bloques libres");
            return;
        }
        else
        {
            memcpy(puntero + sizeof(u_int32_t) * (punteros_actuales + i), &bloque_libre, sizeof(uint32_t));
        }
    }

    munmap(puntero, TAMANIO_BLOQUES);
}

void liberar_bloques_del_puntero_indirecto(char *archivo, int cantidad_bloques_a_liberar, int bloques_actuales)
{
    void *puntero = mmap(NULL, TAMANIO_BLOQUES, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(ARCHIVO_BLOQUES), 0);
    puntero = puntero + obtener_puntero_indirecto(archivo) * TAMANIO_BLOQUES;

    for (int i = 0; i < cantidad_bloques_a_liberar; i++)
    {
        int bloque_fs = leer_puntero_indirecto(puntero, bloques_actuales - 1 - i);
        marcar_bloque_como_libre(bloque_fs);
    }

    munmap(puntero, TAMANIO_BLOQUES);
}

void cargar_puntero_indirecto(char *nombre_archivo)
{
    log_info(LOGGER_FILE_SYSTEM, "Cargando puntero indirecto para el archivo: %s", nombre_archivo);
    retrasar_bloque(nombre_archivo, -1, obtener_puntero_indirecto(nombre_archivo));
}

int leer_puntero_indirecto(void *puntero, int numero_de_bloque)
{
    // El bloque 1 del archivo es el bloque 0 del puntero indirecto
    numero_de_bloque--;

    int bloque_a_leer;
    memcpy(&bloque_a_leer, puntero + numero_de_bloque * sizeof(u_int32_t), sizeof(uint32_t));
    return bloque_a_leer;
}

void *leer_bloque(char *archivo, int bloque_archivo, int offset, int tamanio)
{
    void *buffer = malloc(tamanio);
    void *archivo_de_bloques = mmap(NULL, TAMANIO_BLOQUES, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(ARCHIVO_BLOQUES), 0);

    int bloque_fs;
    if (bloque_archivo == 0)
    {
        bloque_fs = obtener_puntero_directo(archivo);
        retrasar_bloque(archivo, bloque_archivo, bloque_fs);
        memcpy(buffer, archivo_de_bloques + bloque_fs * TAMANIO_BLOQUES + offset, tamanio);
    }

    // Debo leer bloques del puntero indirecto
    // Ya se asume que esta cargado en memoria
    else
    {
        void *puntero_indirecto = archivo_de_bloques + TAMANIO_BLOQUES * obtener_puntero_indirecto(archivo);
        bloque_fs = leer_puntero_indirecto(puntero_indirecto, bloque_archivo);
        retrasar_bloque(archivo, bloque_archivo, bloque_fs);
        memcpy(buffer, archivo_de_bloques + bloque_fs * TAMANIO_BLOQUES + offset, tamanio);
    }

    msync(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES, MS_SYNC);
    munmap(archivo_de_bloques, TAMANIO_BLOQUES);
    return buffer;
}

void escribir_bloque(char *archivo, int bloque_archivo, int offset, int tamanio, void *contenido)
{
    void *archivo_de_bloques = mmap(NULL, TAMANIO_BLOQUES, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(ARCHIVO_BLOQUES), 0);

    int bloque_fs;
    if (bloque_archivo == 0)
    {
        bloque_fs = obtener_puntero_directo(archivo);
        retrasar_bloque(archivo, bloque_archivo, bloque_fs);
        memcpy(archivo_de_bloques + bloque_fs * TAMANIO_BLOQUES + offset, contenido, tamanio);
    }

    // Debo leer bloques del puntero indirecto
    // Ya se asume que esta cargado en memoria
    else
    {
        void *puntero_indirecto = archivo_de_bloques + TAMANIO_BLOQUES * obtener_puntero_indirecto(archivo);
        bloque_fs = leer_puntero_indirecto(puntero_indirecto, bloque_archivo);
        retrasar_bloque(archivo, bloque_archivo, bloque_fs);

        memcpy(archivo_de_bloques + bloque_fs * TAMANIO_BLOQUES + offset, contenido, tamanio);
    }
    
    msync(archivo_de_bloques, CANTIDAD_BLOQUES * TAMANIO_BLOQUES, MS_SYNC);
    munmap(archivo_de_bloques, TAMANIO_BLOQUES);
}

void retrasar_bloque(char *nombre_archivo, int bloque_archivo, int bloque_fs)
{
    if (bloque_archivo == -1)
    {
        log_info(LOGGER_FILE_SYSTEM, "Retardo - Archivo: %s - Bloque Archivo: Bloque de Punteros - Bloque File System %d", nombre_archivo, bloque_fs);
    }
    else
    {
        log_info(LOGGER_FILE_SYSTEM, "Retardo - Archivo: %s - Bloque Archivo: %d - Bloque File System %d", nombre_archivo, bloque_archivo, bloque_fs);
    }
    sleep(RETARDO / 1000);
}