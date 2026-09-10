#ifndef SERVICIO_H
#define SERVICIO_H

#include <string>
#include <vector>
#include <optional>

#include "RepositorioMemoria.h"
#include "Rutina.h"
#include "TablaHipertrofia.h"
#include "TrackingPR.h"
#include "Usuario.h"
#include "Ejercicio.h"
#include "FirebaseClient.h"

struct ResultadoPR {
    RegistroPR registro;
    bool nuevoPR = false;
    double rm1Estimado = 0.0;
    double ratio = 0.0;
    std::string tier;
    bool tierSubio = false;
};

struct TierEjercicio {
    std::string ejercicio;
    double rm1Estimado = 0.0;
    double ratio = 0.0;
    std::string tier;
    std::string color;
};

struct EstadoTiers {
    bool conPeso = false;
    double pesoCorporal = 0.0;
    std::string tierActual;
    std::string colorActual;
    std::string mejorEjercicio;
    double mejorRatio = 0.0;
    std::string siguienteTier;
    double siguienteUmbral = 0.0;
    std::vector<TierEjercicio> porEjercicio;
};

class Servicio {
public:
    static const char* tipoSplit(int dias);

    bool hayPerfil() const { return m_hayPerfil; }
    const Usuario& usuario() const { return m_usuario; }
    Usuario crearPerfil(std::string nombre, int dias,
                        std::vector<std::string> musculosPrioritarios = {},
                        std::string objetivo = "progresar",
                        std::string equipo = "gimnasio",
                        double pesoCorporal = 0.0,
                        double altura = 0.0,
                        int edad = 0,
                        std::string sexo = "hombre");

    std::vector<Ejercicio> alternativasDe(int dia, int indice) const;
    bool cambiarEjercicio(int dia, int indice, const std::string& nuevo);

    bool hayRutina() const { return !m_rutina.vacia(); }
    const Rutina& rutina() const { return m_rutina; }
    const Rutina& generarRutina();
    std::vector<std::string> ejerciciosDeRutina() const;

    ResultadoPR registrarPR(const std::string& ejercicio, double pesoKg,
                            int repeticiones);
    std::vector<RegistroPR> mejoresPRs() const;
    EstadoTiers estadoTiers() const;

    void iniciarSesionEntrenamiento();
    bool sesionActiva() const { return m_sesionActiva; }
    long segundosSesion() const;

    std::vector<VolumenMusculo> volumenSemanal() const { return analizarVolumenSemanal(m_rutina); }

private:
    Usuario m_usuario;
    bool m_hayPerfil = false;
    Rutina m_rutina;
    TrackerPRs m_tracker;
    RepositorioMemoria m_repositorio;
    bool m_sesionActiva = false;
};

#endif
