/* @(#)sb_jugador.c
   AUTOR: Sergio Botero Uribe
   Creado para la practica de sistemas operativos (Manotazo).
   EAFIT 2010-1
*/

#include "sb_jugador.h"

/*
 */
void init_recursos_jugadores() {
  ronda_jugada = 1;
  poner_manos = 0;
  quitar_manos = 0;
  manos_en_centro = 0;
  cartas_recogidas = 0;
  pthread_mutex_init( &manotazo , NULL );
  pthread_mutex_init( &poner_mano , NULL );
  pthread_mutex_init( &quitar_mano , NULL );
  pthread_mutex_unlock( &manotazo );
  pthread_mutex_unlock( &poner_mano );
  pthread_mutex_unlock( &quitar_mano );

}

/*
 */
void liberar_recursos_jugadores() {
  pthread_mutex_destroy( &manotazo );
  pthread_mutex_destroy( &poner_mano );
  pthread_mutex_destroy( &quitar_mano );

}

/*
 */
void * manos(void * param) {
  long me = ( long ) param;
  pthread_t los_ojos;
  int estado_hilo = pthread_create( &los_ojos, NULL, ojos , (void *) me );
  if( estado_hilo ){
    fprintf( stderr, "%s" , crear_ojos_error);
    liberar_recursos();
    exit(1);
  }

  while( !fin_del_juego ){
    pthread_mutex_lock( &mtx_jugadores[me] );
    int carta_a_poner;

    if( !fin_del_juego ){

      if( cuenta_cartas[me] ){
        carta_siguiente = ( carta_siguiente + 1 ) % CARTAS_EN_PINTA;
        carta_a_poner = cartas_jugadores[me][cuenta_cartas[me] -1];
        cartas[ cartas_centro ] = carta_a_poner;
        ++cartas_centro;
        cartas_jugadores[me][cuenta_cartas[me] -1] = NO_CARTA;
        --cuenta_cartas[me];

        if( carta_siguiente == carta_a_poner ){
          pthread_mutex_lock( &manotazo );
          cartas_recogidas = 0;
          poner_manos = 1;
          imprimir_juego( ronda_jugada);
          ++ronda_jugada;
          pthread_mutex_lock( &manotazo );
          pthread_mutex_unlock( &manotazo );
          pthread_mutex_unlock( &poner_mano );
        }

      }

    }else{
      break;
    }

    int i;
    for( i = 0; i < num_jugadores; ++i){
      if( ( cuenta_cartas[i] == 0 && cartas_centro == 0 ) || cartas_centro == 52 ){
        fin_del_juego = 1;
        break;
      }
    }

    pthread_mutex_unlock( &mtx_jugadores[SIGUIENTE] ); //me #defined

  }
  pthread_join( los_ojos , NULL );

  return NULL;
}


/*
 */
void * ojos(void * param) {
  long me = (long) param;
  while( !fin_del_juego ){
    if(poner_manos == 1){
                        //sleep(2);
                usleep( random() % 500  );
      pthread_mutex_lock( &poner_mano );
      ++manos_en_centro;
      if( manos_en_centro == num_jugadores ){
        tomar_cartas(me);
        manos_en_centro = 0;
        poner_manos = 0;
        cartas_recogidas = 1;
        int i;
        for(i=0; i < num_jugadores; ++i){
          if( cuenta_cartas[i] == 0  ){
            fin_del_juego = 1;
            break;
          }
        }


      }
      pthread_mutex_unlock( &poner_mano );

      while( !fin_del_juego && cartas_recogidas == 0 && poner_manos == 1); //yuck!!!
      pthread_mutex_lock( &quitar_mano );
      ++quitar_manos;
      if( quitar_manos == num_jugadores ){
        quitar_manos = 0;
        pthread_mutex_unlock( &manotazo );
      }

      pthread_mutex_unlock( &quitar_mano );
    }
  }

  return NULL;
}

/*
 */
void * tomar_cartas(long me) {
  fprintf( stdout , "%s %ld\n" , jugador_pierde , me + 1 );
  int mazo_aux[CARTAS], i;
  for(i=0; i < CARTAS; ++i ) mazo_aux[i] = NO_CARTA;

  for(i=0; i < cartas_centro; ++i){
    mazo_aux[i] = cartas[i];
    cartas[i] = NO_CARTA;
  }

  for(i=0; i < cuenta_cartas[me]; ++i)
    mazo_aux[cartas_centro + i] = cartas_jugadores[me][i];

  cuenta_cartas[me] += cartas_centro;
  cartas_centro = 0;

  for(i=0; i < cuenta_cartas[me]; ++i)
    cartas_jugadores[me][i] = mazo_aux[i];
  return NULL;
}

/*
 */
void forzar_salida() {
  fin_del_juego = 1;
  cartas_recogidas = 1;
  pthread_mutex_unlock( &poner_mano );
}
