#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void temporizador_iniciar(void);

long temporizador_transcurrido_segundos(void);

void temporizador_regresivo(long segundos);

#ifdef __cplusplus
}
#endif

#endif
