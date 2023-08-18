#include "pcb.h"

void liberar_contexto(t_ctx *ctx)
{
    liberar_elementos_contexto(ctx);
    free(ctx);
}

void liberar_elementos_contexto(t_ctx *ctx)
{
    liberar_parametros_desalojo(ctx->motivos_desalojo);
    list_destroy_and_destroy_elements(ctx->instrucciones, (void *)liberar_instruccion);
    // list_destroy_and_destroy_elements(ctx->tabla_segmentos, (void*) liberar_segmento);
}

void liberar_segmento(t_segmento *segmento)
{
    free(segmento);
}

void liberar_instruccion(t_instruccion *instruccion)
{
    for (int i = 0; i < instruccion->cantidad_parametros; i++)
    {
        free(instruccion->parametros[i]);
    }
    free(instruccion->parametros);
    free(instruccion);
}

void liberar_parametros_desalojo(t_parametros_variables *parametros_variables)
{
    vaciar_parametros_desalojo(parametros_variables);
    free(parametros_variables->parametros);
    free(parametros_variables);
}

void vaciar_parametros_desalojo(t_parametros_variables *parametros)
{
    for (int i = 0; i < parametros->cantidad_parametros; i++)
    {
        free(parametros->parametros[i]);
    }

    parametros->cantidad_parametros = 0;
}

void agregar_parametro_desalojo(t_ctx *ctx, char *parametro)
{

    agregar_parametro_variable(ctx->motivos_desalojo, parametro);
}

void agregar_parametro_variable(t_parametros_variables *parametros_variables, char *parametro)
{

    // Si el puntero aún no ha sido inicializado, asignamos memoria inicial.
    if (parametros_variables->parametros == NULL)
    {
        parametros_variables->parametros = malloc(sizeof(char *));
        parametros_variables->cantidad_parametros = 0; // Inicializar la cantidad de parámetros a 0
    }
    else
    {
        // Si ya ha sido inicializado, realocamos para el nuevo tamaño.
        parametros_variables->parametros = realloc(parametros_variables->parametros, sizeof(char *) * (parametros_variables->cantidad_parametros + 1));
    }

    parametros_variables->parametros[parametros_variables->cantidad_parametros] = malloc(strlen(parametro) + 1);
    strcpy(parametros_variables->parametros[parametros_variables->cantidad_parametros], parametro);

    parametros_variables->cantidad_parametros++;
}
