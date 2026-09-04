#ifndef EJERCICIO_H
#define EJERCICIO_H

#include <string>
#include <utility>

struct Ejercicio {
    std::string nombre;
    std::string grupoMuscular;
    int series = 0;
    int repeticiones = 0;

    Ejercicio() = default;

    Ejercicio(std::string nombre_, std::string grupo_, int series_, int reps_)
        : nombre(std::move(nombre_)),
          grupoMuscular(std::move(grupo_)),
          series(series_),
          repeticiones(reps_) {}
};

#endif