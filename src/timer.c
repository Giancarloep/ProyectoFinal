#include "timer.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>

static void dormir_ms(long ms) { Sleep(ms); }

static time_t inicio_cronometro = 0;

void temporizador_iniciar(void) {
    inicio_cronometro = time(NULL);
}

long temporizador_transcurrido_segundos(void) {
    if (inicio_cronometro == 0) {
        return 0;
    }
    return (long)difftime(time(NULL), inicio_cronometro);
}

static void imprimir_reloj(long total_segundos) {
    printf("\r  %02ld:%02ld ", total_segundos / 60L, total_segundos % 60L);
    fflush(stdout);
}

void temporizador_regresivo(long segundos) {
    long restante = segundos > 0 ? segundos : 0;
    imprimir_reloj(restante);
    while (restante > 0) {
        dormir_ms(1000);
        restante--;
        imprimir_reloj(restante);
    }
    printf("\n");
    inicio_cronometro = 0;
}
