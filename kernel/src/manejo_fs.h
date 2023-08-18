#ifndef MANEJO_FS_H
#define MANEJO_FS_H

    #include <utils/planificacion/pcb.h>
    #include <commons/collections/list.h>
    #include <utils/mensajes.h>
    #include "globales.h"

    typedef struct{
        int puntero;
        char nombre[30];
    }t_file;

    int busqueda_tabla_global(char * );
    int busqueda_tabla_proceso(t_pcb * , char * );
    int obtener_puntero(t_pcb *, char* );
    void agregar_entrada_tabla(char* , int );
    void eliminar_entrada_tabla(int );
    void existe_archivo();
    void solicitar_fs(int );
    void solicitar_fs_r_w(int,int);
    int f_open(t_pcb *, char* );
    void f_close(t_pcb *, char* );
    void f_seek(t_pcb *, char* , char* );
    void f_truncate(t_pcb *, char* , char* );
    void f_read(t_pcb *, char* );
    void f_write(t_pcb *, char*);
    void desbloquear_de_fs(char* );
    t_pcb * buscar_bloqueados_fs(int );
    void liberar_archivo(t_file *archivo);



    
#endif