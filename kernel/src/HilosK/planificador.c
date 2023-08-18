#include "planificador.h"

void *planificador_corto()
{
    log_info(LOGGER_KERNEL, "Inicia el planificador de corto plazo");
    sem_wait(&CORTO_PLAZO);
    empezar_ciclo();
    while (1)
    {
        recibir_de_cpu(SOCKET_CPU);
    }
}
void *planificador_largo()
{
    log_info(LOGGER_KERNEL, "Inicia el planificador de largo plazo");
    while (1)
    {
        sem_wait(&PROCESO_EN_NEW);
        sem_wait(&GRADO_MULTIPROGRAMACION);

        admitir_proceso();

        sem_post(&CORTO_PLAZO);
    }
}

void *comunicacion_fs()
{

    while (1)
    {
        int cod_op = recibir_operacion(SOCKET_FILESYSTEM);

        switch (cod_op)
        {
        case EXISTE:
            pthread_mutex_unlock(&SOLICITUD_FS);
            sem_post(&RESPUESTA_FS);
            break;
        case NO_EXISTE:
            pthread_mutex_unlock(&SOLICITUD_FS);
            solicitar_fs(F_CREATES);
            break;
        case OP_TERMINADA: // esto es cuando termina el f truncate read y write,
            // necesito el nombre del archivo que termino de hacer eso para desbloquear
            // al proceso bloquedo por el archivo
            int size;
            void *buffer = recibir_buffer(&size, SOCKET_FILESYSTEM);
            int desplazamiento = 0;

            int tamanio_string;
            memcpy(&tamanio_string, buffer + desplazamiento, sizeof(int));
            desplazamiento += sizeof(int);

            char *nombre_archivo = malloc(tamanio_string);
            memcpy(nombre_archivo, buffer + desplazamiento, tamanio_string);
            desplazamiento += tamanio_string;

            desbloquear_de_fs(nombre_archivo);
            pthread_mutex_unlock(&SOLICITUD_FS);
            break;
        default:
            break;
        }
    }
}