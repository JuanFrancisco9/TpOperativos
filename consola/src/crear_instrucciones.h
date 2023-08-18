#ifndef CREAR_INSTRUCCIONES_H
#define CREAR_INSTRUCCIONES_H

    #include <utils/mensajes.h>
    #include <commons/string.h>

    extern t_log* logger_consola;

    t_operacion obtener_codigo_instruccion(char *);
    t_instruccion* crear_estructura_instruccion(char*);
    void eliminar_instruccion(t_instruccion*);

#endif