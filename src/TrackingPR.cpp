#include "TrackingPR.h"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>

std::string fechaActual() {
    std::time_t t = std::time(nullptr);
    std::tm tmLocal{};
#ifdef _WIN32
    localtime_s(&tmLocal, &t);
#else
    localtime_r(&t, &tmLocal);
#endif
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tmLocal);
    return buffer;
}
//Formula de Epley
double TrackerPRs::estimar1RM(double pesoKg, int repeticiones) {
    if (pesoKg <= 0.0 || repeticiones <= 0) {
        return 0.0;
    }
    return pesoKg * (1.0 + static_cast<double>(repeticiones) / 30.0);
}

bool TrackerPRs::registrar(const RegistroPR& registro) {
    std::vector<RegistroPR>& historial = m_historial[registro.ejercicio];

    bool esNuevoPR = true;
    const double estimacionNueva =
        estimar1RM(registro.pesoKg, registro.repeticiones);
    for (const auto& r : historial) {
        if (estimar1RM(r.pesoKg, r.repeticiones) >= estimacionNueva) {
            esNuevoPR = false;
            break;
        }
    }

    historial.push_back(registro);
    return esNuevoPR;
}

std::optional<RegistroPR> TrackerPRs::mejorPR(const std::string& ejercicio) const {
    auto it = m_historial.find(ejercicio);
    if (it == m_historial.end()) {
        return std::nullopt;
    }
    const std::vector<RegistroPR>& historial = it->second;
    const RegistroPR* mejor = &historial.front();
    for (const auto& r : historial) {
        if (estimar1RM(r.pesoKg, r.repeticiones) >
            estimar1RM(mejor->pesoKg, mejor->repeticiones)) {
            mejor = &r;
        }
    }
    return *mejor;
}

std::vector<RegistroPR> TrackerPRs::historial(const std::string& ejercicio) const {
    auto it = m_historial.find(ejercicio);
    if (it == m_historial.end()) {
        return {};
    }
    return it->second;
}

std::vector<std::string> TrackerPRs::ejerciciosConPR() const {
    std::vector<std::string> nombres;
    nombres.reserve(m_historial.size());
    for (const auto& [ejercicio, registros] : m_historial) {
        nombres.push_back(ejercicio);
    }
    return nombres;
}

void TrackerPRs::imprimirResumen() const {
    if (m_historial.empty()) {
        std::cout << "\nAun no hay PRs registrados.\n";
        return;
    }

    std::cout << "\n================ RESUMEN DE PRs ================\n";
    std::cout << std::left << std::setw(32) << "Ejercicio"
              << std::right << std::setw(10) << "Peso"
              << std::setw(6) << "Reps"
              << std::setw(9) << "1RM est"
              << "  Fecha\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& [ejercicio, registros] : m_historial) {
        auto mejor = mejorPR(ejercicio);
        if (!mejor) continue;
        std::cout << std::left << std::setw(32)
                  << (ejercicio.size() > 31 ? ejercicio.substr(0, 31) : ejercicio)
                  << std::right << std::setw(9) << mejor->pesoKg << "kg"
                  << std::setw(6) << mejor->repeticiones
                  << std::setw(9) << std::fixed << std::setprecision(1)
                  << estimar1RM(mejor->pesoKg, mejor->repeticiones)
                  << "  " << mejor->fecha << "\n";
    }
}
