#ifndef NEW_H_
#define NEW_H_

#include <utils/planificacion/pcb.h>
#include <commons/string.h>
#include "globales.h"
#include "ready.h"


    t_pcb *crear_pcb(t_list*, int );
    void nuevo_proceso(t_list *, int);
    t_pcb *get_proceso_desde_new();

#endif