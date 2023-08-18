#ifndef MANEJO_MEMORIA_H
#define MANEJO_MEMORIA_H

    #include <globales.h>
    #include <utils/mensajes.h>
    #include <Estados/exit.h>

    void crear_segmento(t_pcb *);
    void eliminar_segmento(t_pcb *);
    t_pcb* buscar_proceso(int );
    int buscar_ready(int );
    t_pcb* buscar_block(int );
    int buscar_block_fs(int );
    void enviar_compactacion();
    void actualizar_todas_las_tablas_de_segmentos(t_list* );
    

#endif