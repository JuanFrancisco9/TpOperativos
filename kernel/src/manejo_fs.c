#include "manejo_fs.h"
// tabla de archivos tiene que tener toda la info necesaria y una lista de procesos bloqueados
int busqueda_tabla_global(char *nombre_archivo)
{
  int i = 0;
  while (i < tamnio_tabla_global())
  {
    t_tabla_global *auxiliar = get_de_tabla_global(i);
    if (strcmp(nombre_archivo, auxiliar->identificador) == 0)
    {
      return i;
    }
    i++;
  }
  return -1;
}

int busqueda_tabla_proceso(t_pcb *proceso, char *nombre_archivo)
{
  int i = 0;
  while (i < list_size(proceso->archivos_abiertos))
  {
    t_file *auxiliar = list_get(proceso->archivos_abiertos, i);
    if (strcmp(nombre_archivo, auxiliar->nombre) == 0)
    {
      return i;
    }
    i++;
  }
  return -1;
}

void agregar_entrada_tabla(char *nombre, int pid)
{
  t_tabla_global *auxiliar = malloc(sizeof(t_tabla_global));
  strcpy(auxiliar->identificador, nombre);
  auxiliar->lista_de_procesos_bloqueados = list_create();
  auxiliar->PID = pid;
  agregar_a_tabla_global(auxiliar);
}

void agregar_archivo_abierto(t_list *lista_archivo_abierto, char *nombre)
{
  t_file *archivo = malloc(sizeof(t_file));
  strcpy(archivo->nombre, nombre);
  archivo->puntero = 0;
  list_add(lista_archivo_abierto, archivo);
}

void eliminar_entrada_tabla(int posicion)
{
  t_tabla_global *auxiliar = get_de_tabla_global(posicion);
  list_destroy(auxiliar->lista_de_procesos_bloqueados);
  free(auxiliar);
  sacar_de_tabla_global(posicion);
}

void existe_archivo()
{ // Solicita la apertura de un archivo para verificar si existe
  solicitar_fs(F_OPEN);
}

void solicitar_fs(int cod_op){
  pthread_mutex_lock(&SOLICITUD_FS);
  t_paquete *paquete = crear_paquete(cod_op);
  agregar_a_paquete_dato_serializado(paquete, &EJECUTANDO->contexto->PID, sizeof(int));
  serializar_motivos_desalojo(EJECUTANDO->contexto->motivos_desalojo, paquete);
  enviar_paquete(paquete, SOCKET_FILESYSTEM);
  eliminar_paquete(paquete);
}


int f_open(t_pcb *proceso, char *nombre_archivo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);

  if (busqueda == -1)
  {
    existe_archivo(); // pregunta a fs si existe dicho archivo
    sem_wait(&RESPUESTA_FS);
  
    agregar_entrada_tabla(nombre_archivo, proceso->contexto->PID);
    agregar_archivo_abierto(proceso->archivos_abiertos, nombre_archivo);
    
    log_info(LOGGER_KERNEL, "PID: %d - Abrir Archivo: %s", proceso->contexto->PID, nombre_archivo);

    return 0; // puede seguir
  }
  else
  {
    // si esta en la tabla
    t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

    agregar_archivo_abierto(proceso->archivos_abiertos, nombre_archivo);
    list_add(entrada_tabla->lista_de_procesos_bloqueados, proceso);
    log_info(LOGGER_KERNEL, "PID: <%d> - Bloqueado por: <%s>", proceso->contexto->PID, nombre_archivo);
    cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
    return 1; // bloqueado
  }
}

void f_close(t_pcb *proceso, char *nombre_archivo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: <%d> - Error al cerrar Archivo: <%s>", proceso->contexto->PID, nombre_archivo);
  }
  else
  {
    if (list_size(entrada_tabla->lista_de_procesos_bloqueados) == 0)
    {
      // no hay nadie mas
      eliminar_entrada_tabla(busqueda);
    }
    else
    {
      log_info(LOGGER_KERNEL, "PID: <%d> - Cerrar Archivo: <%s>", proceso->contexto->PID, nombre_archivo);
      // hay alguien mas
      // elimina el archivos de la lista del proceso
      list_remove_and_destroy_element(proceso->archivos_abiertos, busqueda_tabla_proceso(proceso, nombre_archivo), (void*) liberar_archivo);

      t_pcb *proceso_a_desbloquear = list_remove(entrada_tabla->lista_de_procesos_bloqueados, 0); // Saca el proceso de la lista de bloqueados
      // cambiar pid, cede el control sobre el archivo a otro proceso y lo desbloquea
      entrada_tabla->PID = proceso_a_desbloquear->contexto->PID;
      cambio_de_estado(proceso_a_desbloquear->contexto->PID, "Block", "Ready");
      agregar_a_lista_ready(proceso_a_desbloquear);

    }
  }
}

void liberar_archivo(t_file *archivo)
{
  free(archivo);
}

void f_seek(t_pcb *proceso, char *nombre_archivo, char *inicio)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

  int posicion = atoi(inicio);
  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: %d - Error al hacer seek en Archivo: %s", proceso->contexto->PID, nombre_archivo);
  }
  else
  {
    // cambiar puntero
    t_file *archivo = list_get(proceso->archivos_abiertos, busqueda_tabla_proceso(proceso, nombre_archivo));
    archivo->puntero = posicion;
    log_info(LOGGER_KERNEL, "PID: <%d> - Actualizar puntero Archivo: %s - Puntero %d", proceso->contexto->PID, nombre_archivo, archivo->puntero);
  }
}
// Truncate, READ y WRITE

void f_truncate(t_pcb *proceso, char *nombre_archivo, char *tamanio_nuevo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: %d - Error al hacer truncate en Archivo: %s", proceso->contexto->PID, nombre_archivo);
  }
  else
  {
    int tamanio = atoi(tamanio_nuevo);
    // mandar a file system nombre_archivo y tamanio
    agregar_a_lista_blockfs(proceso);
    solicitar_fs(F_TRUNCATE);
    log_info(LOGGER_KERNEL, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>", proceso->contexto->PID, nombre_archivo, tamanio);

    log_info(LOGGER_KERNEL, "PID: <%d> - Bloqueado por: <%s>", proceso->contexto->PID, nombre_archivo);
    cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
  }
}

int obtener_puntero(t_pcb *proceso, char *nombre_archivo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: %d - Error al obtener puntero Archivo: %s", proceso->contexto->PID, nombre_archivo);
    return -1;
  }
  else
  {
    t_file *aux = list_get(proceso->archivos_abiertos, busqueda_tabla_proceso(proceso, nombre_archivo));
    return aux->puntero;
  }
}

void f_read(t_pcb *proceso, char *nombre_archivo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);


  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: %d - Error al hacer read en Archivo: %s", proceso->contexto->PID, nombre_archivo);
  }
  else
  {
    int puntero = obtener_puntero(proceso, nombre_archivo);

    agregar_parametro_desalojo(proceso->contexto, string_itoa(puntero));
    agregar_a_lista_blockfs(proceso);
    solicitar_fs(F_READ);


    long dir_fisica = strtol(proceso->contexto->motivos_desalojo->parametros[1], NULL, 10);
    int cant_bytes = atoi(proceso->contexto->motivos_desalojo->parametros[2]);
    log_info(LOGGER_KERNEL, "PID: <%d> - Leer Archivo: <%s> - Puntero <%d> - Dirección Memoria <%p> - Tamaño <%d>", proceso->contexto->PID, nombre_archivo, puntero, (void*) dir_fisica, cant_bytes);
    log_info(LOGGER_KERNEL, "PID: <%d> - Bloqueado por: <%s>", proceso->contexto->PID, nombre_archivo);

    cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
  }
}

void f_write(t_pcb *proceso, char *nombre_archivo)
{
  int busqueda = busqueda_tabla_global(nombre_archivo);
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda);

  if (busqueda == -1 || entrada_tabla->PID != proceso->contexto->PID)
  {
    log_error(LOGGER_KERNEL, "PID: %d - Error al hacer read en Archivo: %s", proceso->contexto->PID, nombre_archivo);
  }
  else
  {
    // mandar a file system archivo y cant_bytes
    int puntero = obtener_puntero(proceso, nombre_archivo);

    
    agregar_parametro_desalojo(proceso->contexto, string_itoa(puntero));
    agregar_a_lista_blockfs(proceso);
    solicitar_fs(F_WRITE);


    long dir_fisica = strtol(proceso->contexto->motivos_desalojo->parametros[1], NULL, 10);
    int cant_bytes = atoi(proceso->contexto->motivos_desalojo->parametros[2]);
    log_info(LOGGER_KERNEL, "PID: <%d> - Escribir Archivo: <%s> - Puntero <%d> - Dirección Memoria <%p> - Tamaño <%d>", proceso->contexto->PID, nombre_archivo, puntero, (void*) dir_fisica, cant_bytes);
    log_info(LOGGER_KERNEL, "PID: <%d> - Bloqueado por: <%s>", proceso->contexto->PID, nombre_archivo);
    cambio_de_estado(proceso->contexto->PID, "Exec", "Block");
  }
}

void desbloquear_de_fs(char *nombre_archivo)
{
  t_tabla_global *entrada_tabla = get_de_tabla_global(busqueda_tabla_global(nombre_archivo));

  t_pcb *proceso = buscar_bloqueados_fs(entrada_tabla->PID);

  if (proceso != NULL) {
    cambio_de_estado(proceso->contexto->PID,"Block","Ready");
    agregar_a_lista_ready(proceso);
  }
}

t_pcb *buscar_bloqueados_fs(int pid)
{
  int i = 0;
  while (i < tamnio_lista_blockfs())
  {
    t_pcb *auxiliar = get_de_lista_blockfs(i);
    if (auxiliar->contexto->PID == pid)
    {
      sacar_elemento_de_lista_blockfs(auxiliar);
      return auxiliar;
    }
    i++;
  }
  return NULL;
}
