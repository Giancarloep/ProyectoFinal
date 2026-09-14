#ifndef RUTINA_H
#define RUTINA_H

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Ejercicio.h"

struct DiaEntrenamiento {
    std::string nombre;
    std::vector<Ejercicio> ejercicios;
};

class Rutina {
public:
    Rutina() = default;

    explicit Rutina(std::string nombreUsuario)
        : m_nombreUsuario(std::move(nombreUsuario)) {}

    void agregarDia(DiaEntrenamiento dia) { m_dias.push_back(std::move(dia)); }

    bool reemplazarEjercicio(std::size_t dia, std::size_t indice,
                             Ejercicio nuevo) {
        if (dia >= m_dias.size() || indice >= m_dias[dia].ejercicios.size()) {
            return false;
        }
        m_dias[dia].ejercicios[indice] = std::move(nuevo);
        return true;
    }

    bool insertarEjercicio(std::size_t dia, std::size_t indice,
                           Ejercicio ej) {
        if (dia >= m_dias.size()) return false;
        auto& v = m_dias[dia].ejercicios;
        if (indice > v.size()) indice = v.size();
        v.insert(v.begin() + static_cast<std::ptrdiff_t>(indice),
                 std::move(ej));
        return true;
    }

    bool quitarEjercicio(std::size_t dia, std::size_t indice) {
        if (dia >= m_dias.size() || indice >= m_dias[dia].ejercicios.size()) {
            return false;
        }
        m_dias[dia].ejercicios.erase(m_dias[dia].ejercicios.begin() +
                                     static_cast<std::ptrdiff_t>(indice));
        return true;
    }

    const std::string& nombreUsuario() const { return m_nombreUsuario; }
    const std::vector<DiaEntrenamiento>& dias() const { return m_dias; }
    bool vacia() const { return m_dias.empty(); }

    void imprimir() const {
        std::cout << "\n=== Rutina de " << m_nombreUsuario << " ("
                  << m_dias.size() << " dia(s)/semana) ===\n";
        for (std::size_t i = 0; i < m_dias.size(); ++i) {
            std::cout << "\n[" << (i + 1) << "] " << m_dias[i].nombre << "\n";
            for (const auto& e : m_dias[i].ejercicios) {
                std::cout << "   - " << e.nombre
                          << " (" << e.grupoMuscular << ") "
                          << e.series << "x" << e.repeticiones << "\n";
            }
        }
    }

private:
    std::string m_nombreUsuario;
    std::vector<DiaEntrenamiento> m_dias;
};

#endif
