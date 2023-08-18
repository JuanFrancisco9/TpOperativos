#ifndef CONSOLA_H_
#define CONSOLA_H_
    
    #include <utils/logs.h>
    #include <utils/conexiones.h>
    #include <utils/configs.h>
    #include <init.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <utils/mensajes.h>
    #include <commons/string.h> 
    #include <crear_instrucciones.h>

    t_log* logger_consola;
    t_config * config; 


    void finalizar_consola(int);

#endif
