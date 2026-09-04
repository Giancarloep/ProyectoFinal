#ifndef EQUIPOS_H
#define EQUIPOS_H

#include <string>
#include <vector>

struct InfoEquipo {
    std::string clave;
    std::string etiqueta;
    int nivel = 0;   // 0 gimnasio completo, 1 mancuernas, 2 solo peso corporal
};

inline const std::vector<InfoEquipo>& equiposDisponibles() {
    static const std::vector<InfoEquipo> lista = {
        {"gimnasio", "Gimnasio / CrossFit", 0},
        {"mancuernas", "Casa con mancuernas", 1},
        {"peso_corporal", "Solo peso corporal", 2},
    };
    return lista;
}

inline const InfoEquipo* buscarEquipo(const std::string& clave) {
    for (const auto& e : equiposDisponibles()) {
        if (e.clave == clave) return &e;
    }
    return nullptr;
}

#endif
