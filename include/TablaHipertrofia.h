#ifndef TABLAHIPERTROFIA_H
#define TABLAHIPERTROFIA_H

#include <string>
#include <vector>

#include "Rutina.h"

struct GrupoTabla {
    std::string musculo;
    std::string referencia;
    std::vector<std::string> alias;

    std::string mv, mev, mav, mrv;
    std::string frecuencia;
    std::string repeticiones;
    std::string rir;

    int mvMin = 0;
    int mevMin = 0, mevMax = 0;
    int mavMin = 0, mavMax = 0;
    int mrvMin = 0;
    int repsMin = 6, repsMax = 20;

    int mevPromedio() const { return (mevMin + mevMax) / 2; }
    int mavPromedio() const { return (mavMin + mavMax) / 2; }
};

enum class EstadoVolumen {
    Insuficiente,
    Mantenimiento,
    Optimo,
    Alto,
    Excesivo,
};

struct VolumenMusculo {
    GrupoTabla fila;
    int seriesSemana = 0;
    EstadoVolumen estado = EstadoVolumen::Optimo;
};

const std::vector<GrupoTabla>& tablaHipertrofia();

const GrupoTabla* buscarGrupoTabla(const std::string& musculoOAlias);

std::vector<VolumenMusculo> analizarVolumenSemanal(const Rutina& rutina);

const char* nombreEstado(EstadoVolumen estado);

struct InfoObjetivo {
    std::string clave;
    std::string etiqueta;
    std::string descripcion;
    std::string zona;   // verde | naranja | roja
    int diasMin = 2;
    int diasMax = 7;
};

const std::vector<InfoObjetivo>& objetivosEntrenamiento();
const InfoObjetivo* buscarObjetivo(const std::string& clave);
int volumenObjetivo(const GrupoTabla& grupo, const std::string& objetivo);

#endif