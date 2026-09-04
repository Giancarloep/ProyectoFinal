#ifndef GENERADORRUTINAS_H
#define GENERADORRUTINAS_H

#include <optional>
#include <string>
#include <vector>

#include "Ejercicio.h"
#include "Rutina.h"
#include "Usuario.h"

class GeneradorRutinas {
public:
    GeneradorRutinas() = delete;

    static Rutina generar(const Usuario& usuario);

    static std::vector<Ejercicio> alternativasPara(
        const Usuario& usuario, const std::string& grupoMuscular,
        const std::string& excluirNombre);

    static std::optional<Ejercicio> crearEjercicio(
        const Usuario& usuario, const std::string& nombre);
};

#endif
