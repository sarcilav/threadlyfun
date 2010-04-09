/* @(#)sb_dealer.c
   AUTOR: Sergio Botero Uribe
   Creado para la practica de sistemas operativos (Manotazo).
   EAFIT 2010-1
*/

#include "sb_dealer.h"

/*
 */
int main(int argc, char * argv[])
{
  init();

  if(argc > 1){
    num_jugadores = atoi(argv[1]);
    if( num_jugadores < 2 || num_jugadores > 52 ){
      fprintf(stderr, "%s" , numero_inv );
      exit(1);
    }
  }

  void * jugadores_ptr = realloc( jugadores, ( num_jugadores * sizeof(pthread_t) ));
  if(jugadores_ptr == NULL){
    fprintf(stderr , "%s" , jugadores_error);
    exit(1);
  }
  jugadores = (pthread_t*) jugadores_ptr;

  void * cartas_jugadores_ptr = realloc( cartas_jugadores , num_jugadores * sizeof(int *) );
  if(cartas_jugadores_ptr == NULL){
    fprintf(stderr , "%s" , cartas_error);
    free( jugadores );
    exit(1);
  }
  cartas_jugadores = cartas_jugadores_ptr;
  int i;
  for (i = 0; i < num_jugadores ; ++i){
    cartas_jugadores[i] = malloc( sizeof(int) * CARTAS );
    if(cartas_jugadores[i] == NULL){
      free( jugadores );
      free( cartas_jugadores );
      exit(1);
    }
  }

  void * mtx_jugadores_ptr = realloc( mtx_jugadores, (num_jugadores * sizeof(pthread_mutex_t) ));
  if(mtx_jugadores_ptr == NULL){
    fprintf(stderr , "%s" , mutex_error);
    free( jugadores );
    free( cartas_jugadores );
    exit(1);
  }
  mtx_jugadores = (pthread_mutex_t*) mtx_jugadores_ptr;

  void * cuenta_cartas_ptr = realloc(cuenta_cartas, (num_jugadores * sizeof(int) ));
  if(cuenta_cartas_ptr == NULL){
    fprintf(stderr,"%s",cuenta_error);
    free(jugadores);
    free(cartas_jugadores);
    free(mtx_jugadores);
    exit(1);
  }
  cuenta_cartas = (int *)cuenta_cartas_ptr;

  iniciar_juego( random() % num_jugadores );

  while( !fin_del_juego );
  terminar_juego();
  return 0;
}


/*
 */
void init() {
  freopen( nombre_archivo, "w", stdout);
  jugadores = NULL;
  num_jugadores = 2;
  srand( (unsigned)time(0) );
  fin_del_juego = 0;
  carta_siguiente = NO_CARTA;
}

/*
 */
void c_baraja(int cartas[]) {
  int i , j , k = 0 , l;
  for(l = 0; l < BARAJAS; ++l){
    for(i = 0; i < PINTAS; ++i){
      for(j = 0; j < CARTAS_EN_PINTA; ++j){
        cartas[k] = j;
        ++k;
      }
    }
  }
}

/*
  Método de R.A. Fisher and F. Yates, Explicado en `The Art of Computer programming' by Donald Knuth
*/
void barajar_cartas(int cartas[],int num_cartas) {
  int i,j,aux;
  for( i = num_cartas -1; i > 0; --i){
    j = ( random()) % (i + 1);
    aux = cartas[i];
    cartas[i] = cartas[j];
    cartas[j] = aux;
  }
}


/*
 */
void repartir_cartas() {
  int i,j,k;
  for(i = 0; i < CARTAS ;++i) cuenta_cartas[i] = 0;

  for(j = 0,k = 0; j < CARTAS; ++j){
    for(i = 0; i < num_jugadores; ++i){
      if(k < CARTAS){
        cartas_jugadores[i][j] = cartas[k];
        cartas[k] = NO_CARTA;
        ++cuenta_cartas[i];
      }else{
        cartas_jugadores[i][j] = NO_CARTA;
      }
      ++k;
    }
  }
}

/*
 */
void lock_mutex_ronda(int index) {
  while(index){
    --index;
    pthread_mutex_init( &mtx_jugadores[index] , NULL);
    pthread_mutex_lock( &mtx_jugadores[index] );
  }
}


/*
 */
void iniciar_juego(int jugador) {
  init_recursos_jugadores();
  lock_mutex_ronda( num_jugadores );
  c_baraja( cartas );
  barajar_cartas( cartas , CARTAS  );
  repartir_cartas(  );
  imprimir_juego( 0 );
  poner_jugadores();
  pthread_mutex_unlock( &mtx_jugadores[jugador] );
}


/*
 */
void poner_jugadores() {
  int estado_hilo;
  long i = 0;
  pthread_t *index;
  for(index = jugadores ; index < &jugadores[num_jugadores] ; ++index){
    estado_hilo = pthread_create( index , NULL, manos ,(void*) i++);
    if( estado_hilo ){
      fprintf(stderr , "%s" , crear_error);
      free( jugadores );
      free( mtx_jugadores );
      exit(1);
    }
  }
}


/*
 */
void quitar_jugadores(int index) {
  int i = index;
  while( index )
    pthread_mutex_unlock( &mtx_jugadores[--index] );

  void * estado_join;
  int join_index;
  for(join_index = 0 ; join_index < num_jugadores ; ++join_index){
    pthread_join( jugadores[join_index], &estado_join );
  }
  fprintf(stdout, "\n%s\t%d\n" , fin_hilos,join_index );

  while( i ) pthread_mutex_destroy( &mtx_jugadores[--i] );

}


/*
 */
void liberar_recursos(){
  liberar_recursos_jugadores();
  free(jugadores);//realloc( jugadores,0 ); //cool
  free( cartas_jugadores );
  //free( mtx_jugadores );
  // free( cuenta_cartas );
}


/*
 */
void imprimir_juego(int ronda) {
  imprimir_juego_h( ronda ) ;
  //  imprimir_juego_v( ronda ) ;
  if(ronda == -1) imprimir_resultados();
}

/*
 */
void imprimir_juego_v(int ronda) {

}

/*
 */
void imprimir_juego_h(int ronda) {
  int i,j,karta;
  char rep_carta;
  if(ronda == -1)
    fprintf( stdout, "\n\t--------------------\n\n\n%s\n", ronda_final);
  else if (ronda == 0)
    fprintf( stdout, "%s\n",ronda_inicial);
  else
    fprintf( stdout, "\n\t--------------------\n\n\n%s %d\n", inicio_ronda, ronda);

  for(i = 0; i < num_jugadores; ++i){
    for(j = 0; j < CARTAS; ++j){
      if(j == 0 ) fprintf(stdout,"%s #%3d  (%d)\n", jugador_string, i + 1 , cuenta_cartas[i]);
      karta = cartas_jugadores[i][j];
      if(karta == NO_CARTA){
        break;
      }
      fprintf(stdout,"%c  ",  valor_carta(karta));
    }
    puts("");
  }

  for(j = 0; j < CARTAS; ++j){
    if( j == 0 ) fprintf(stdout,"%s (%d)\n", cartas_mesa , cartas_centro);
    karta = cartas[j];
    if(karta == NO_CARTA){
      break;
    }
    fprintf(stdout,"%c  ", valor_carta(karta));
  }
  puts("");



}

/*
 */
char valor_carta(int karta){
  char rep_carta;
  if(karta == 0){
    rep_carta='A';
  }else if(karta == 9){
    rep_carta= 'X';
  }else if(karta == 10){
    rep_carta = 'J';
  }else if(karta == 11){
    rep_carta = 'Q';
  }else if(karta == 12){
    rep_carta = 'K';
  }else{
    rep_carta = '1' + karta;
  }

  return rep_carta;
}


/*
 */
void terminar_juego() {
  imprimir_juego( -1 );
  quitar_jugadores( num_jugadores );
  liberar_recursos();
  fprintf(stdout, "%s" , fin_juego);
}


/*
 */
void imprimir_resultados() {

  char ganan[52] = "", pierden[52] = "";
  int count_g = 0 , count_p = 0 , max_p = 0 , i;
  for(i = 0; i < num_jugadores; ++i){
    if( cuenta_cartas[i] == 0){
      ++count_g;
    }else if( cuenta_cartas[i] == max_p ){
      ++count_p;
    }else if( max_p < cuenta_cartas[i] ){
      max_p = cuenta_cartas[i];
      count_p = 1;
    }
  }
  for(i = 0; i < num_jugadores; ++i){
    if( cuenta_cartas[i] == 0  ){
      sprintf(ganan,"%s %d",ganan, i + 1 );
    }else if(cuenta_cartas[i] == max_p){
      sprintf(pierden,"%s %d",pierden, i + 1 );
    }
  }


  if( count_g > 1 ){
    fprintf(stdout, "%s %s\n",empate,ganan);
  }else if( count_g == 1 ){
    fprintf(stdout, "%s %s\n",ganador,ganan);
  }

  if( count_p > 1 ){
    fprintf(stdout, "%ses %s\n",pierden_msg,pierden);
  }else if( count_p == 1 ){
    fprintf(stdout, "%s %s\n",pierden_msg,pierden);
  }
  puts("");

}


/*
  cartas[i] ^= cartas[j];
  cartas[j] ^= cartas[i];
  cartas[i] ^= cartas[j];
*/

/*
   { puts("");int i;for(i=0; i<CARTAS ;++i) fprintf(stdout, "%d ",cartas[i]);puts("");}
*/
