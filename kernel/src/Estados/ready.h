#ifndef READY_H_
#define READY_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <commons/collections/list.h>
    #include "new.h"
    #include "exec.h"
    #include <globales.h>
    #include <utils/mensajes.h>

    int get_grado_de_multiprogramacion(int);
    bool hay_espacio_ready();
    void agregar_a_ready_si_hay_espacio();
    void admitir_proceso();
    int get_procesos_en_cpu();
    t_pcb *get_proceso_por_fifo();
    t_pcb* get_proceso_por_hrrn();
    t_pcb *ceder_proceso_a_exec();
    

#endif