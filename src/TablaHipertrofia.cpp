#include "TablaHipertrofia.h"

#include <algorithm>

const std::vector<GrupoTabla>& tablaHipertrofia() {
    static const std::vector<GrupoTabla> tabla = {
        {"Cuadriceps", "Quads", {"Cuadriceps", "Pierna"},
         "6", "8-12", "12-18", "20+", "1.5-3x Semana", "6-20", "1-3",
         6, 8, 12, 12, 18, 20, 6, 20},
        {"Femoral", "Hams", {"Femoral"},
         "4", "6-10", "10-16", "20+", "2-3x Semana", "6-20", "1-3",
         4, 6, 10, 10, 16, 20, 6, 20},
        {"Gluteos", "Glutes", {"Gluteos"},
         "0", "0-4", "4-12", "16+", "2-3x Semana", "6-20", "1-3",
         0, 0, 4, 4, 12, 16, 6, 20},
        {"Pecho", "Chest", {"Pecho"},
         "8", "10-12", "12-20", "22+", "2-4x Semana", "4-20", "1-3",
         8, 10, 12, 12, 20, 22, 4, 20},
        {"Espalda", "Back", {"Espalda"},
         "8", "10-14", "14-22", "25+", "2-4x Semana", "6-20", "1-3",
         8, 10, 14, 14, 22, 25, 6, 20},
        {"Hombro", "Delts", {"Hombro"},
         "0-6", "6-8", "16-22", "26+", "2-6x Semana", "8-20", "0-2",
         0, 6, 8, 16, 22, 26, 8, 20},
        {"Biceps", "Biceps", {"Biceps"},
         "0-6", "8-14", "14-20", "26+", "2-6x Semana", "8-15", "0-2",
         0, 8, 14, 14, 20, 26, 8, 15},
        {"Triceps", "Triceps", {"Triceps"},
         "0-4", "6-10", "10-14", "18+", "2-4x Semana", "8-20", "0-2",
         0, 6, 10, 10, 14, 18, 8, 20},
        {"Trapecio", "Traps", {"Trapecio"},
         "0", "1-12", "12-20", "26+", "2-6x Semana", "8-20", "0-2",
         0, 1, 12, 12, 20, 26, 8, 20},
        {"Gemelos", "Calves", {"Gemelos"},
         "0-6", "8-12", "12-16", "20+", "2-4x Semana", "8-20", "0-2",
         0, 8, 12, 12, 16, 20, 8, 20},
        {"Abdomen", "Abs", {"Abdomen", "Core"},
         "0", "1-15", "16-20", "25+", "2-6x Semana", "8-20", "0-2",
         0, 1, 15, 16, 20, 25, 8, 20},
    };
    return tabla;
}

const GrupoTabla* buscarGrupoTabla(const std::string& musculoOAlias) {
    for (const auto& fila : tablaHipertrofia()) {
        if (fila.musculo == musculoOAlias) return &fila;
        for (const auto& a : fila.alias) {
            if (a == musculoOAlias) return &fila;
        }
    }
    return nullptr;
}

const char* nombreEstado(EstadoVolumen estado) {
    switch (estado) {
        case EstadoVolumen::Insuficiente: return "insuficiente";
        case EstadoVolumen::Mantenimiento: return "mantenimiento";
        case EstadoVolumen::Optimo: return "optimo";
        case EstadoVolumen::Alto: return "alto";
        case EstadoVolumen::Excesivo: return "excesivo";
    }
    return "";
}

const std::vector<InfoObjetivo>& objetivosEntrenamiento() {
    static const std::vector<InfoObjetivo> lista = {
        {"mantener",
         "Mantener mi físico",
         "Zona verde (MV-MEV). Suficiente estímulo para no perder lo ganado, sin vivir en el gimnasio.",
         "verde", 2, 3},
        {"progresar",
         "Mejorar con el tiempo",
         "Zona verde-naranja (MEV hacia MAV). Progreso constante y sostenible para naturales.",
         "verde", 4, 5},
        {"estancado",
         "Romper el estancamiento",
         "Zona naranja (MAV). Más volumen semanal para empujar nuevas adaptaciones.",
         "naranja", 5, 6},
        {"avanzado",
         "Entrenamiento avanzado (con sustancias)",
         "Zona naranja-roja (MAV alto). Volumen muy alto que exige máxima recuperación.",
         "roja", 5, 7},
    };
    return lista;
}

const InfoObjetivo* buscarObjetivo(const std::string& clave) {
    for (const auto& o : objetivosEntrenamiento()) {
        if (o.clave == clave) return &o;
    }
    return nullptr;
}

int volumenObjetivo(const GrupoTabla& grupo, const std::string& objetivo) {
    if (objetivo == "mantener") return (grupo.mvMin + grupo.mevMax) / 2;
    if (objetivo == "estancado") return grupo.mavPromedio();
    if (objetivo == "avanzado") return grupo.mavMax;
    return (grupo.mevPromedio() + grupo.mavPromedio()) / 2;
}

static EstadoVolumen clasificar(const GrupoTabla& g, int series) {
    if (series < g.mvMin) return EstadoVolumen::Insuficiente;
    if (series < g.mevMin) return EstadoVolumen::Mantenimiento;
    if (series <= g.mavMax) return EstadoVolumen::Optimo;
    if (series < g.mrvMin) return EstadoVolumen::Alto;
    return EstadoVolumen::Excesivo;
}

std::vector<VolumenMusculo> analizarVolumenSemanal(const Rutina& rutina) {
    std::vector<VolumenMusculo> resultado;
    resultado.reserve(tablaHipertrofia().size());

    for (const auto& fila : tablaHipertrofia()) {
        VolumenMusculo entrada;
        entrada.fila = fila;

        for (const auto& dia : rutina.dias()) {
            for (const auto& ejercicio : dia.ejercicios) {
                bool coincide = ejercicio.grupoMuscular == fila.musculo ||
                                ejercicio.grupoMuscular == fila.referencia;
                if (!coincide) {
                    for (const auto& a : fila.alias) {
                        coincide = coincide || ejercicio.grupoMuscular == a;
                    }
                }
                if (coincide) entrada.seriesSemana += ejercicio.series;
            }
        }

        entrada.estado = clasificar(fila, entrada.seriesSemana);
        resultado.push_back(entrada);
    }
    return resultado;
}
