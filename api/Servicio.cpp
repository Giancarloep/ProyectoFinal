#include "Servicio.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Equipos.h"
#include "GeneradorRutinas.h"
#include "TablaHipertrofia.h"
#include "TiersSaiyayin.h"
#include "TrackingPR.h"
#include "timer.h"

const char* Servicio::tipoSplit(int dias) {
    if (dias <= 3) return "Cuerpo completo";
    if (dias <= 5) return "Torso/Pierna";
    return "Empuje/Traccion/Pierna";
}

Usuario Servicio::crearPerfil(std::string nombre, int dias,
                              std::vector<std::string> musculosPrioritarios,
                              std::string objetivo, std::string equipo,
                              double pesoCorporal, double altura, int edad,
                              std::string sexo) {
    if (nombre.empty()) nombre = "(sin nombre)";
    if (dias < 1) dias = 1;
    if (dias > 7) dias = 7;
    if (pesoCorporal < 0) pesoCorporal = 0;
    if (pesoCorporal > 400) pesoCorporal = 400;
    if (altura < 0) altura = 0;
    if (altura > 300) altura = 300;
    if (edad < 0) edad = 0;
    if (edad > 120) edad = 120;
    if (sexo != "hombre" && sexo != "mujer") sexo = "hombre";
    if (!buscarObjetivo(objetivo)) objetivo = "progresar";
    if (!buscarEquipo(equipo)) equipo = "gimnasio";

    std::vector<std::string> validos;
    for (const auto& m : musculosPrioritarios) {
        if (buscarGrupoTabla(m)) validos.push_back(m);
    }

    m_usuario.nombre = std::move(nombre);
    m_usuario.diasDisponibles = dias;
    m_usuario.pesoCorporal = pesoCorporal;
    m_usuario.altura = altura;
    m_usuario.edad = edad;
    m_usuario.sexo = std::move(sexo);
    m_usuario.objetivo = std::move(objetivo);
    m_usuario.equipo = std::move(equipo);
    m_usuario.musculosPrioritarios = std::move(validos);
    m_hayPerfil = true;

    m_repositorio.guardarUsuario(m_usuario);
    return m_usuario;
}

std::vector<Ejercicio> Servicio::alternativasDe(int dia, int indice) const {
    if (!m_rutina.vacia() && dia >= 0 &&
        dia < static_cast<int>(m_rutina.dias().size()) && indice >= 0 &&
        indice < static_cast<int>(m_rutina.dias()[dia].ejercicios.size())) {
        const Ejercicio& actual = m_rutina.dias()[dia].ejercicios[indice];
        return GeneradorRutinas::alternativasPara(m_usuario, actual.grupoMuscular, actual.nombre);
    }
    return {};
}
bool Servicio::cambiarEjercicio(int dia, int indice,
                                const std::string& nuevo) {
    if (dia < 0 || indice < 0 ||
        dia >= static_cast<int>(m_rutina.dias().size()) ||
        indice >= static_cast<int>(m_rutina.dias()[static_cast<std::size_t>(dia)].ejercicios.size())) {
        return false;
    }
    const Ejercicio anterior =
        m_rutina.dias()[static_cast<std::size_t>(dia)]
            .ejercicios[static_cast<std::size_t>(indice)];
    auto reemplazo = GeneradorRutinas::crearEjercicio(m_usuario, nuevo);
    if (!reemplazo) return false;
    if (!m_rutina.reemplazarEjercicio(static_cast<std::size_t>(dia),
                                      static_cast<std::size_t>(indice),
                                      *reemplazo)) {
        return false;
    }
    m_historialCambios.push(
        {static_cast<std::size_t>(dia), static_cast<std::size_t>(indice),
         anterior, false});
    m_repositorio.guardarRutina(m_rutina);
    return true;
}

bool Servicio::eliminarEjercicio(int dia, int indice) {
    if (dia < 0 || indice < 0 ||
        dia >= static_cast<int>(m_rutina.dias().size()) ||
        indice >= static_cast<int>(m_rutina.dias()[static_cast<std::size_t>(dia)].ejercicios.size())) {
        return false;
    }
    const Ejercicio anterior =
        m_rutina.dias()[static_cast<std::size_t>(dia)]
            .ejercicios[static_cast<std::size_t>(indice)];
    if (!m_rutina.quitarEjercicio(static_cast<std::size_t>(dia),
                                  static_cast<std::size_t>(indice))) {
        return false;
    }
    m_historialCambios.push(
        {static_cast<std::size_t>(dia), static_cast<std::size_t>(indice),
         anterior, true});
    m_repositorio.guardarRutina(m_rutina);
    return true;
}

bool Servicio::deshacerCambio() {
    if (m_historialCambios.vacia()) return false;
    const CambioEjercicio cambio = m_historialCambios.cima();
    m_historialCambios.pop();
    bool ok = cambio.eraEliminacion
                  ? m_rutina.insertarEjercicio(cambio.dia, cambio.indice,
                                               cambio.anterior)
                  : m_rutina.reemplazarEjercicio(cambio.dia, cambio.indice,
                                                 cambio.anterior);
    if (!ok) return false;
    m_repositorio.guardarRutina(m_rutina);
    return true;
}

const Rutina& Servicio::generarRutina() {
    m_rutina = GeneradorRutinas::generar(m_usuario);
    m_historialCambios.limpiar();
    m_repositorio.guardarRutina(m_rutina);
    return m_rutina;
}

std::vector<std::string> Servicio::ejerciciosDeRutina() const {
    std::vector<std::string> nombres;
    for (const auto& dia : m_rutina.dias()) {
        for (const auto& e : dia.ejercicios) {
            bool existe = false;
            for (const auto& n : nombres) existe = existe || n == e.nombre;
            if (!existe) nombres.push_back(e.nombre);
        }
    }
    return nombres;
}

ResultadoPR Servicio::registrarPR(const std::string& ejercicio, double pesoKg,
                                  int repeticiones) {
    RegistroPR registro;
    registro.usuario = m_usuario.nombre.empty() ? "(sin perfil)" : m_usuario.nombre;
    registro.ejercicio = ejercicio;
    registro.pesoKg = pesoKg;
    registro.repeticiones = repeticiones;
    registro.fecha = fechaActual();

    const double rm1 = TrackerPRs::estimar1RM(pesoKg, repeticiones);
    const bool conPeso = m_usuario.pesoCorporal > 0;
    int indiceAnterior = -1;
    if (conPeso) {
        if (const auto previo = m_tracker.mejorPR(ejercicio)) {
            const double rm1Previo = TrackerPRs::estimar1RM(
                previo->pesoKg, previo->repeticiones);
            indiceAnterior = indiceTier(rm1Previo / m_usuario.pesoCorporal);
        }
    }

    ResultadoPR resultado;
    resultado.registro = registro;
    resultado.nuevoPR = m_tracker.registrar(registro);
    resultado.rm1Estimado = rm1;
    if (conPeso) {
        resultado.ratio = rm1 / m_usuario.pesoCorporal;
        const DefinicionTier& tier = tierDe(resultado.ratio);
        resultado.tier = tier.nombre;
        resultado.tierSubio = resultado.nuevoPR &&
            indiceTier(resultado.ratio) > indiceAnterior;
    }
    m_repositorio.guardarPR(registro);
    return resultado;
}

std::vector<RegistroPR> Servicio::mejoresPRs() const {
    std::vector<RegistroPR> mejores;
    for (const auto& ejercicio : m_tracker.ejerciciosConPR()) {
        auto mejor = m_tracker.mejorPR(ejercicio);
        if (mejor) mejores.push_back(*mejor);
    }
    return mejores;
}

EstadoTiers Servicio::estadoTiers() const {
    EstadoTiers estado;
    estado.pesoCorporal = m_usuario.pesoCorporal;
    estado.conPeso = m_usuario.pesoCorporal > 0;
    if (!estado.conPeso) return estado;

    for (const auto& r : mejoresPRs()) {
        const double rm1 = TrackerPRs::estimar1RM(r.pesoKg, r.repeticiones);
        const double ratio = rm1 / m_usuario.pesoCorporal;
        const DefinicionTier& tier = tierDe(ratio);

        estado.porEjercicio.push_back(
            {r.ejercicio, rm1, ratio, tier.nombre, tier.color});

        if (ratio > estado.mejorRatio) {
            estado.mejorRatio = ratio;
            estado.mejorEjercicio = r.ejercicio;
            estado.tierActual = tier.nombre;
            estado.colorActual = tier.color;

            const auto& tabla = tablaTiers();
            const int indice = indiceTier(ratio);
            if (indice + 1 < static_cast<int>(tabla.size())) {
                estado.siguienteTier = tabla[indice + 1].nombre;
                estado.siguienteUmbral = tabla[indice + 1].umbralRatio;
            }
        }
    }
    return estado;
}

void Servicio::iniciarSesionEntrenamiento() {
    temporizador_iniciar();
    m_sesionActiva = true;
}

long Servicio::segundosSesion() const {
    return temporizador_transcurrido_segundos();
}

SesionEntrenamiento Servicio::terminarSesionEntrenamiento() {
    SesionEntrenamiento sesion{fechaActual(), segundosSesion()};
    m_sesionActiva = false;
    m_colaSesiones.encolar(sesion);
    while (m_colaSesiones.tamano() > 10) {
        m_colaSesiones.desencolar();
    }
    return sesion;
}

std::vector<SesionEntrenamiento> Servicio::sesiones() const {
    return m_colaSesiones.aVector();
}