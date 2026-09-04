#ifndef TRACKINGPR_H
#define TRACKINGPR_H

#include <map>
#include <optional>
#include <string>
#include <vector>

struct RegistroPR {
    std::string usuario;
    std::string ejercicio;
    double pesoKg = 0.0;
    int repeticiones = 0;
    std::string fecha;
};

class TrackerPRs {
public:
    static double estimar1RM(double pesoKg, int repeticiones);

    bool registrar(const RegistroPR& registro);

    std::optional<RegistroPR> mejorPR(const std::string& ejercicio) const;

    std::vector<RegistroPR> historial(const std::string& ejercicio) const;

    std::vector<std::string> ejerciciosConPR() const;

    void imprimirResumen() const;

private:
    std::map<std::string, std::vector<RegistroPR>> m_historial;
};

std::string fechaActual();

#endif
