#ifndef EXEC_H_
#define EXEC_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <commons/collections/list.h>
    #include <utils/mensajes.h>
    #include <manejo_fs.h>
    #include <manejo_memoria.h>
    #include "ready.h"
    #include "exit.h"
    #include "blocked.h"

    void reemplazar_proceso(t_pcb *);
    void reemplazar_ctx(t_ctx *);
    void enviar_a_cpu();
    void empezar_ciclo();
    void definir_accion(int, t_pcb *);
    void recibir_de_cpu(int);
    void reemplazar_exec_por_nuevo();
    void estimado_prox_rafaga();
    void crear_segmento(t_pcb *);
    void eliminar_segmento(t_pcb *);




#endif