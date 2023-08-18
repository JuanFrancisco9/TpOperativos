#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_
    
    #include <utils/logs.h>
    #include <utils/conexiones.h>
    #include <utils/configs.h>

    
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    
    #include "init.h"
    #include "globales.h"
    #include "conexiones_modulos.h"
    #include "fcb.h"

void liberar_config(t_config * config){
	config_destroy(config);
}

void finalizar(){
    dictionary_destroy_and_destroy_elements(DICCIONARIO_FCB, (void *) liberar_config);
	fclose(ARCHIVO_BLOQUES);
    bitarray_destroy(BITMAP_BLOQUES);
}


#endif