#include "configs.h"

t_config *iniciar_config(char *path)
{
	t_config *nuevo_config;

	nuevo_config = config_create(path);

	if (nuevo_config == NULL)
	{

		FILE *file = fopen(path, "w"); // Abre el archivo en modo escritura
		if (file == NULL)
		{
			printf("Error al crear el archivo de configuración.\n");
			abort();
		}

		nuevo_config = config_create(path);

	}

	return nuevo_config;
}