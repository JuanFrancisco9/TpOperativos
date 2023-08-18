#ifndef INIT_KERNEL_H
#define INIT_KERNEL_H

    #include <utils/conexiones.h>
    #include <utils/configs.h>
    #include "globales.h"
    #include <string.h>

    void inicializar_conexiones(t_config*);
    void inicializar_variables_globales(t_config*);
    void inicializar_semaforos();
    void inicializar_estados();
    int* convertirAArrayInt(char**);
#endif