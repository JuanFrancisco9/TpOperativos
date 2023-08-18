#include <instruccionescpu.h>
bool primera_instruccion = 0;

// //Cuando recibe un ctx

t_instruccion *fetch(t_ctx *ctx)
{
	t_instruccion *instruccion_nueva = list_get(ctx->instrucciones, ctx->program_counter); // Busca la instrucción y la guarda.
	switch (instruccion_nueva->operacion)
	{
	case EXIT:
		break;
	default:
		ctx->program_counter++;
		break;
	}
	return instruccion_nueva;
}

void decode(t_instruccion *instruccion)
{
	switch (instruccion->operacion)
	{
	case SET:
		usleep(TIEMPO_RETARDO * 1000);
		break;
	// case WRITE:
	// 	break;
	// case READ:
	// 	break;
	default:
		break;
	}
}

char *obtenerRegistro(t_registros *registros, const char *nombreRegistro)
{
	if (strcmp(nombreRegistro, "AX") == 0)
		return registros->AX;
	else if (strcmp(nombreRegistro, "BX") == 0)
		return registros->BX;
	else if (strcmp(nombreRegistro, "CX") == 0)
		return registros->CX;
	else if (strcmp(nombreRegistro, "DX") == 0)
		return registros->DX;
	else if (strcmp(nombreRegistro, "EAX") == 0)
		return registros->EAX;
	else if (strcmp(nombreRegistro, "EBX") == 0)
		return registros->EBX;
	else if (strcmp(nombreRegistro, "ECX") == 0)
		return registros->ECX;
	else if (strcmp(nombreRegistro, "EDX") == 0)
		return registros->EDX;
	else if (strcmp(nombreRegistro, "RAX") == 0)
		return registros->RAX;
	else if (strcmp(nombreRegistro, "RBX") == 0)
		return registros->RBX;
	else if (strcmp(nombreRegistro, "RCX") == 0)
		return registros->RCX;
	else if (strcmp(nombreRegistro, "RDX") == 0)
		return registros->RDX;
	else
		return NULL; // Si el nombre del registro no coincide, devuelve NULL o puedes manejarlo de otra manera según tus necesidades.
}

// if the register is AX, BX, CX, DX, it returns 4. If it is EAX, EBX, ECX, EDX it returns 8. If RAX, RBX, RCX, RDX it returns 16.
int tamanio_registro(const char *nombre)
{

	if (strcmp(nombre, "AX") == 0 || strcmp(nombre, "BX") == 0 || strcmp(nombre, "CX") == 0 || strcmp(nombre, "DX") == 0)
		return 4;
	else if (strcmp(nombre, "EAX") == 0 || strcmp(nombre, "EBX") == 0 || strcmp(nombre, "ECX") == 0 || strcmp(nombre, "EDX") == 0)
		return 8;
	else if (strcmp(nombre, "RAX") == 0 || strcmp(nombre, "RBX") == 0 || strcmp(nombre, "RCX") == 0 || strcmp(nombre, "RDX") == 0)
		return 16;
	else
		return 0;
}

t_operacion execute(t_instruccion *instruccion_actual, t_ctx *ctx)
{
	switch (instruccion_actual->operacion)
	{
	case SET:
		log_info(LOGGER_CPU, "PID : %d - <SET> - <%s %s>", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		char *registro = obtenerRegistro(&ctx->registros, instruccion_actual->parametros[0]);
		strcpy(registro, instruccion_actual->parametros[1]);
		return 0;

	case MOV_IN:
		log_info(LOGGER_CPU, "PID : %d - <MOV_IN> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		long dir_fisica = MMU(atoi(instruccion_actual->parametros[1]), tamanio_registro(instruccion_actual->parametros[0]), ctx);
		if (dir_fisica == -1)
		{
			return SEG_FAULT;
		}
		char *dir_fisica_string = malloc(10);
		sprintf(dir_fisica_string, "%ld", dir_fisica);
		char *tamanio = string_itoa(tamanio_registro(instruccion_actual->parametros[0]));
		agregar_parametro_desalojo(ctx, tamanio);
		agregar_parametro_desalojo(ctx, dir_fisica_string);

		t_paquete *paquete = crear_paquete(MOV_IN);
		agregar_a_paquete_dato_serializado(paquete, &ctx->PID, sizeof(int));
		serializar_motivos_desalojo(ctx->motivos_desalojo, paquete);
		enviar_paquete(paquete, SOCKET_MEMORIA);
		eliminar_paquete(paquete);
		free(dir_fisica_string);
		free(tamanio);
		recibir_operacion(SOCKET_MEMORIA);
		char *valor_leido = recibir_mensaje(SOCKET_MEMORIA);
		log_info(LOGGER_CPU, "PID: %d  -Acción: LEER - Segmento: %d - Dirección Física: %p - Valor: %s", ctx->PID, floor_div(atoi(instruccion_actual->parametros[1]), TAM_MAX_SEGMENTO), (void *)dir_fisica, valor_leido);
		// acceder a registro en instruccion_actual->parametros[1] y guardar valor_leido
		registro = obtenerRegistro(&ctx->registros, instruccion_actual->parametros[0]);
		strcpy(registro, valor_leido);
		free(valor_leido);
		return 0;

	case MOV_OUT:
		log_info(LOGGER_CPU, "PID : %d - <MOV OUT> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		registro = obtenerRegistro(&ctx->registros, instruccion_actual->parametros[1]);
		dir_fisica = MMU(atoi(instruccion_actual->parametros[0]), tamanio_registro(instruccion_actual->parametros[1]), ctx);
		if (dir_fisica == -1)
		{
			return SEG_FAULT;
		}
		dir_fisica_string = malloc(13);
		sprintf(dir_fisica_string, "%ld", dir_fisica);

		agregar_parametro_desalojo(ctx, registro);
		agregar_parametro_desalojo(ctx, dir_fisica_string);
		paquete = crear_paquete(MOV_OUT);
		agregar_a_paquete_dato_serializado(paquete, &ctx->PID, sizeof(int));
		serializar_motivos_desalojo(ctx->motivos_desalojo, paquete);
		enviar_paquete(paquete, SOCKET_MEMORIA);
		eliminar_paquete(paquete);
		free(dir_fisica_string);

		// recibir ok memoria
		recibir_operacion(SOCKET_MEMORIA);
		char *mensaje = recibir_mensaje(SOCKET_MEMORIA);
		if (strcmp(mensaje, "OK") != 0)
		{
			log_error(LOGGER_CPU, "PID: %d  -Error al escribir en memoria", ctx->PID);
			return SEG_FAULT;
		}

		log_info(LOGGER_CPU, "PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %p - Valor: %s", ctx->PID, floor_div(atoi(instruccion_actual->parametros[1]), TAM_MAX_SEGMENTO), (void *)dir_fisica, registro);
		free(mensaje);
		return 0;

	case WAIT:
		log_info(LOGGER_CPU, "PID : %d - <WAIT> - <%s> ", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return WAIT;

	case SIGNAL:
		log_info(LOGGER_CPU, "PID : %d - <SIGNAL> - <%s> ", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return SIGNAL;

	case YIELD:
		log_info(LOGGER_CPU, "PID : %d - <YIELD>", ctx->PID);
		return YIELD;

	case EXIT:
		log_info(LOGGER_CPU, "PID : %d - <EXIT> ", ctx->PID);
		return EXIT;

	case IO:
		log_info(LOGGER_CPU, "PID : %d - <IO> - <%s>", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return IO;

	case F_OPEN:
		log_info(LOGGER_CPU, "PID : %d - <F OPEN> - <%s> ", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return F_OPEN;

	case F_WRITE:
		log_info(LOGGER_CPU, "PID : %d - <F WRITE> - <%s %s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1], instruccion_actual->parametros[2]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		dir_fisica = MMU(atoi(instruccion_actual->parametros[1]), atoi(instruccion_actual->parametros[2]), ctx);
		if (dir_fisica == -1)
		{
			return SEG_FAULT;
		}
		dir_fisica_string = malloc(10);
		sprintf(dir_fisica_string, "%ld", dir_fisica);
		agregar_parametro_desalojo(ctx, dir_fisica_string);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[2]);
		return F_WRITE;

	case F_READ:
		log_info(LOGGER_CPU, "PID : %d - <F_READ> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		dir_fisica = MMU(atoi(instruccion_actual->parametros[1]), atoi(instruccion_actual->parametros[2]), ctx);
		if (dir_fisica == -1)
		{

			return SEG_FAULT;
		}
		dir_fisica_string = malloc(10);
		sprintf(dir_fisica_string, "%ld", dir_fisica);
		agregar_parametro_desalojo(ctx, dir_fisica_string);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[2]);
		return F_READ;

	case F_TRUNCATE:
		log_info(LOGGER_CPU, "PID : %d - <F TRUNCATE> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[1]);
		return F_TRUNCATE;

	case F_CLOSE:
		log_info(LOGGER_CPU, "PID : %d - <F CLOSE> - <%s> ", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return F_CLOSE;

	case F_SEEK:
		log_info(LOGGER_CPU, "PID : %d - <F SEEK> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[1]);
		return F_SEEK;

	case CREATE_SEGMENT:
		log_info(LOGGER_CPU, "PID : %d - <CREATE_SEGMENT> - <%s %s> ", ctx->PID, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[1]);
		return CREATE_SEGMENT;

	case DELETE_SEGMENT:
		log_info(LOGGER_CPU, "PID : %d - <DELETE_SEGMENT> - <%s> ", ctx->PID, instruccion_actual->parametros[0]);
		agregar_parametro_desalojo(ctx, instruccion_actual->parametros[0]);
		return DELETE_SEGMENT;

	default:
		return 0;
	}
};

long MMU(int direccion_logica, int bytes, t_ctx *ctx)
{
	int num_segmento = floor_div(direccion_logica, TAM_MAX_SEGMENTO);
	int offset = direccion_logica % TAM_MAX_SEGMENTO /*TAMANIO_MAX_SEG*/;

	log_info(LOGGER_CPU, "OFFSET: %d, BYTEs: %d", offset, bytes);

	if (offset + bytes > TAM_MAX_SEGMENTO /*TAMANIO_MAX_SEG*/)
	{
		// “PID: <PID> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>”
		log_error(LOGGER_CPU, "PID: %d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamaño: %d", ctx->PID, num_segmento, offset, bytes);
		return -1;
	}

	t_segmento *segmento = list_get(ctx->tabla_segmentos, num_segmento);
	long direccion_fisica = (long)(segmento->base + offset);
	return direccion_fisica;
}

int floor_div(int a, int b)
{
	return (a - (a % b)) / b;
}

void ciclo_de_instruccion(t_ctx *ctx)
{
	t_instruccion *instruccion_actual;

	while (ctx != NULL && ctx->program_counter <= ctx->cant_instrucciones)
	{
		instruccion_actual = fetch(ctx);
		decode(instruccion_actual);
		int cod_op = execute(instruccion_actual, ctx);

		// Devuelve 0 si debe seguir ejecutando
		// Devuelve un codigo_operacion si debe enviarlo al kernel
		// el codigo de operacion son los enums de enviar_paquete y son siempre > 0

		if (cod_op > 0)
		{
			t_paquete *paquete = crear_paquete(cod_op);
			serializar_contexto(ctx, paquete);
			enviar_paquete(paquete, SOCKET_KERNEL);
			eliminar_paquete(paquete);
			list_destroy_and_destroy_elements(ctx->tabla_segmentos, free);
			liberar_contexto(ctx);
			ctx = NULL;
		}
		else
		{
			vaciar_parametros_desalojo(ctx->motivos_desalojo);
		}
	}
}
