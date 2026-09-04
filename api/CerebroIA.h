#ifndef CEREBROIA_H
#define CEREBROIA_H

#include <chrono>
#include <string>

class Servicio;

class CerebroIA {
public:
    explicit CerebroIA(const Servicio& servicio);

    bool disponible() const;
    const std::string& modeloActual() const {
        return m_modeloEfectivo.empty() ? m_modelo : m_modeloEfectivo;
    }
    std::string conversar(const std::string& pregunta) const;

private:
    const Servicio& m_servicio;
    std::string m_urlBase;
    std::string m_modelo;
    mutable std::string m_modeloEfectivo;
    mutable std::chrono::steady_clock::time_point m_cacheHasta{};
    mutable bool m_estadoCache = false;
};

#endif
