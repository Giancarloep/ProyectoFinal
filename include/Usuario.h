#ifndef USUARIO_H
#define USUARIO_H

#include <string>
#include <vector>

struct Usuario {
    std::string nombre;
    int diasDisponibles = 0;
    double pesoCorporal = 0.0;
    double altura = 0.0;
    int edad = 0;
    std::string sexo = "hombre";
    std::string objetivo = "progresar";
    std::string equipo = "gimnasio";
    std::vector<std::string> musculosPrioritarios;
};

#endif
