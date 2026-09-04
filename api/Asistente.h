#ifndef ASISTENTE_H
#define ASISTENTE_H

#include <string>

class Servicio;
class CerebroIA;

class Asistente {
public:
    Asistente(const Servicio& servicio, const CerebroIA& cerebro)
        : m_servicio(servicio), m_cerebro(cerebro) {}

    std::string responder(const std::string& pregunta) const;

private:
    bool intentarReglas(const std::string& pregunta,
                        std::string& respuesta) const;

    const Servicio& m_servicio;
    const CerebroIA& m_cerebro;
};

#endif
