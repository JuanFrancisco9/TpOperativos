#ifndef INIT_FILE_SYSTEM_H
#define INIT_FILE_SYSTEM_H

    #include <utils/conexiones.h>
    #include <utils/configs.h>
    #include <commons/bitarray.h>
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <globales.h>



    void inicializar_conexiones(int *, t_config * );
    
    void inicializar_archivos(t_config* );
    t_config* levantar_superbloque(t_config* config);
    t_bitarray* levantar_bitmap(char* );
    FILE* levantar_bloques(char* );

    // Auxiliares
    void truncar_archivo(FILE *file, int tamaño);
    FILE *abrir_o_crear_archivo(char *path, int cantidad_de_bytes_necesarios, bool *bitmap_recien_creado);




#endif