#include "logs.h"

t_log* iniciar_logger(char* path, char* nombre) {
	t_log* nuevo_logger;

	nuevo_logger = log_create(path, nombre, 1, LOG_LEVEL_INFO);

	if (!nuevo_logger) {
		printf("error en el logger create\n");
		exit(1);
	}

	return nuevo_logger;
}