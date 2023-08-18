#include "globales.h"

t_log *LOGGER_KERNEL;
int SOCKET_CPU;
int SOCKET_MEMORIA;
int SOCKET_FILESYSTEM;

// Contador de procesos generados
int PID_COUNT = 0;

// Tabla global de archivos abiertos
t_list *TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS;

// Listas para los estados
t_list *LISTA_NEW;
t_list *LISTA_READY;
t_pcb *EJECUTANDO;
t_list *LISTAS_BLOCK;
t_list *BLOQUEADOS_FS;

// Variables de configuracion
char *ALGORITMO_PLANIFICACION;
int ESTIMACION_INICIAL;
float HRRN_ALFA;
int GRADO_MAX_MULTIPROGRAMACION;
char **RECURSOS;
int *INSTANCIAS_RECURSOS;

// Semaforos
sem_t PROCESO_EN_NEW;
sem_t PROCESO_EN_READY;
sem_t PROCESO_EN_BLOCKFS;
sem_t GRADO_MULTIPROGRAMACION;
sem_t CORTO_PLAZO;
sem_t ARCHIVO_ABIERTO;
sem_t RESPUESTA_FS;
sem_t ENTRADA_EN_TABLA_GLOBAL;
pthread_mutex_t MUTEX_LISTA_NEW;
pthread_mutex_t MUTEX_LISTA_READY;
pthread_mutex_t MUTEX_TABLA_ARCHIVOS;
pthread_mutex_t MUTEX_LISTA_BLOCKFS;
pthread_mutex_t SOLICITUD_FS;
pthread_mutex_t SOLICITUD_MEMORIA;

// Temporales

t_temporal *TIEMPO_CORRIENDO;
t_temporal *TIEMPO_EN_CPU;

// Declaracion de monitores para las variables globales

// Para agregar a las listas

void agregar_a_lista_new(t_pcb *nuevo)
{
    pthread_mutex_lock(&MUTEX_LISTA_NEW);
    list_add(LISTA_NEW, nuevo);
    pthread_mutex_unlock(&MUTEX_LISTA_NEW);
    sem_post(&PROCESO_EN_NEW);
}

void agregar_a_lista_ready(t_pcb *nuevo)
{
    pthread_mutex_lock(&MUTEX_LISTA_READY);

    nuevo->tiempo_llegada_ready = temporal_gettime(TIEMPO_CORRIENDO);
    list_add(LISTA_READY, nuevo);

    char *result = concatenarLista(LISTA_READY);
    log_info(LOGGER_KERNEL, "Cola Ready <%s>: [%s]", ALGORITMO_PLANIFICACION, result);
    free(result);

    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    sem_post(&PROCESO_EN_READY);
}
void agregar_a_lista_blockfs(t_pcb *nuevo)
{
    pthread_mutex_lock(&MUTEX_LISTA_BLOCKFS);
    list_add(BLOQUEADOS_FS, nuevo);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOCKFS);
    sem_post(&PROCESO_EN_BLOCKFS);
}
void agregar_a_tabla_global(t_tabla_global *nuevo)
{
    pthread_mutex_lock(&MUTEX_TABLA_ARCHIVOS);
    list_add(TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS, nuevo);
    pthread_mutex_unlock(&MUTEX_TABLA_ARCHIVOS);
    sem_post(&ENTRADA_EN_TABLA_GLOBAL);
}

// Para sacar elemento X de la lista
t_pcb *sacar_de_lista_new(int posicion)
{
    pthread_mutex_lock(&MUTEX_LISTA_NEW);
    t_pcb *pcb = list_remove(LISTA_NEW, posicion);
    pthread_mutex_unlock(&MUTEX_LISTA_NEW);
    return pcb;
}

t_pcb *sacar_de_lista_ready(int posicion)
{
    sem_wait(&PROCESO_EN_READY); // Me parece que es este semaforo el
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    t_pcb *pcb = list_remove(LISTA_READY, posicion);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return pcb;
}
t_tabla_global *sacar_de_tabla_global(int posicion)
{
    sem_wait(&ENTRADA_EN_TABLA_GLOBAL);
    pthread_mutex_lock(&MUTEX_TABLA_ARCHIVOS);
    t_tabla_global *elemento = list_remove(TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS, posicion);
    pthread_mutex_unlock(&MUTEX_TABLA_ARCHIVOS);
    return elemento;
}
void sacar_elemento_de_lista_ready(t_pcb *elemento)
{
    sem_wait(&PROCESO_EN_READY);
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    list_remove_element(LISTA_READY, elemento);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
}
void sacar_elemento_de_lista_blockfs(t_pcb *elemento)
{
    sem_wait(&PROCESO_EN_BLOCKFS);
    pthread_mutex_lock(&MUTEX_LISTA_BLOCKFS);
    list_remove_element(BLOQUEADOS_FS, elemento);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOCKFS);
}
// Monitores para los get
t_pcb *get_de_lista_ready(int posicion)
{
    sem_wait(&PROCESO_EN_READY);
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    t_pcb *pcb = list_get(LISTA_READY, posicion);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    sem_post(&PROCESO_EN_READY);

    return pcb;
}
t_pcb *get_de_lista_blockfs(int posicion)
{
    sem_wait(&PROCESO_EN_BLOCKFS);
    pthread_mutex_lock(&MUTEX_LISTA_BLOCKFS);
    t_pcb *pcb = list_get(BLOQUEADOS_FS, posicion);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOCKFS);
    sem_post(&PROCESO_EN_BLOCKFS);

    return pcb;
}
t_tabla_global *get_de_tabla_global(int posicion)
{
    sem_wait(&ENTRADA_EN_TABLA_GLOBAL);
    pthread_mutex_lock(&MUTEX_TABLA_ARCHIVOS);
    t_tabla_global *elemento = list_get(TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS, posicion);
    pthread_mutex_unlock(&MUTEX_TABLA_ARCHIVOS);
    sem_post(&ENTRADA_EN_TABLA_GLOBAL);

    return elemento;
}
int tamnio_lista_ready()
{
    int i;
    pthread_mutex_lock(&MUTEX_LISTA_NEW);
    i = list_size(LISTA_READY);
    pthread_mutex_unlock(&MUTEX_LISTA_NEW);
    return i;
}
int tamnio_lista_blockfs()
{
    int i;
    pthread_mutex_lock(&MUTEX_LISTA_BLOCKFS);
    i = list_size(BLOQUEADOS_FS);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOCKFS);
    return i;
}
int tamnio_tabla_global()
{
    int i;
    pthread_mutex_lock(&MUTEX_TABLA_ARCHIVOS);
    i = list_size(TABLA_GLOBAL_DE_ARCHIVOS_ABIERTOS);
    pthread_mutex_unlock(&MUTEX_TABLA_ARCHIVOS);
    return i;
}
// cambios de estado
void cambio_de_estado(int pid, char *anterior, char *nuevo)
{
    log_info(LOGGER_KERNEL, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", pid, anterior, nuevo);
}
char *concatenarLista(t_list *lista)
{
    // Calcula el tamaño total del string resultante
    int totalSize = 0;
    int i = 0;
    t_pcb *proceso;
    while (i < list_size(lista))
    {
        // Asumiendo que cada número puede tener un máximo de 3 dígitos
        totalSize += 4; // Considera el espacio para el número, la coma y el espacio
        i++;
    }

    // Crea el string resultante
    char *result = malloc(totalSize * sizeof(char));
    result[0] = '\0'; // Inicializa el string como vacío

    // Concatena los números con comas
    i = 0;
    while (i < list_size(lista))
    {
        char numStr[4]; // Para almacenar el número como string (3 dígitos + 1 para el terminador null)
        proceso = list_get(lista, i);
        sprintf(numStr, "%d", proceso->contexto->PID); // Convierte el número a string

        strcat(result, numStr);
        if (i != list_size(lista) - 1)
        {
            strcat(result, ", ");
        }
        i++;
    }

    return result;
}
