#ifndef BLOCKED_H_
#define BLOCKED_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <commons/collections/list.h>
    #include <globales.h>
    #include "exec.h"
    
    void wait(t_pcb*, char*);
    void signal(t_pcb*, char*);
    void io(t_pcb*);
    int get_id_recurso(char[]);

#endif