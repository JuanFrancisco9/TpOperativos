#ifndef BLOQUES_H
#define BLOQUES_H

    #include <globales.h>
    #include <string.h>
    #include <stdint.h>
    #include <commons/bitarray.h>
    #include <math.h>
    #include <fcb.h>

    #include <sys/mman.h>

    #define MAX(x, y) (((x) > (y)) ? (x) : (y))
    #define MIN(x, y) (((x) < (y)) ? (x) : (y))

    u_int32_t buscar_bloque_libre();    
    void marcar_bloque_como_libre(uint32_t);
    void asignar_bloques_al_puntero_indirecto(char*, int, int);
    void liberar_bloques_del_puntero_indirecto(char*, int , int);
    
    void cargar_puntero_indirecto(char* nombre_archivo);

    int leer_puntero_indirecto(void* puntero, int numero_de_bloque);
    void* leer_bloque(char* , int , int , int );
    void escribir_bloque(char* archivo, int bloque, int offset, int tamanio, void* contenido);

    void retrasar_bloque(char* archivo, int bloque, int bloque_fs);

#endif