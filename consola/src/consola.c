#include <consola.h>

int main(int argc, char *argv[])
{

  logger_consola = iniciar_logger("./consola.log", "CONSOLA");

  FILE *f = verificar_argumentos(argc, argv);
  if (f == NULL)
  {
    return EXIT_FAILURE;
  }

  config = iniciar_config(argv[1]);

  int conexion_kernel;
  inicializar_conexiones(&conexion_kernel, config);

  char buffer[100];

  t_paquete *paquete = crear_paquete(INSTRUCCIONES);

  t_instruccion *instruccion;
  while ((fgets(buffer, 100, f)) != NULL)
  {
    buffer[strcspn(buffer, "\n")] = 0;
    
    instruccion = crear_estructura_instruccion(buffer);
    
    serializar_instruccion(instruccion, paquete);
    
    eliminar_instruccion(instruccion);
  }
  
  enviar_paquete(paquete, conexion_kernel);
  eliminar_paquete(paquete);


  log_info(logger_consola, "Paquete enviado");

  finalizar_consola(conexion_kernel);
  fclose(f);
  return 0;
}

void finalizar_consola(int conexion_kernel){
  int cod_op = recibir_operacion(conexion_kernel);
  if (cod_op == TERMINAR)
    log_info(logger_consola, "El kernel finalizo las instrucciones. Cerrando la consola");

  else
    log_error(logger_consola, "Operacion desconocida");

  terminar_conexiones(1, conexion_kernel);
  terminar_programa(logger_consola, config);
}

