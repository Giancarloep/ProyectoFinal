#include "CerebroIA.h"

#include <cstdlib>
#include <sstream>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "Servicio.h"
#include "TablaHipertrofia.h"
#include "Equipos.h"

using json = nlohmann::json;

namespace {
// La idea original para la ia, preguntas simples que retornan cosas simples, preguntas cortas (aun esta en uso pero la idea es usar la otra)
std::string entorno(const char* clave, const std::string& porDefecto) {
    const char* v = std::getenv(clave);
    return (v && *v) ? std::string(v) : porDefecto;
}

std::string construirContexto(const Servicio& s) {
    std::ostringstream o;
    o << "Eres el entrenador virtual de la app Personal Trainer, un gestor "
         "de entrenamiento. Respondes en espanol, breve y practico "
         "(maximo 150 palabras), con tono motivador pero realista.\n";

    if (s.hayPerfil()) {
        const Usuario& u = s.usuario();
        o << "\nPERFIL DEL USUARIO:\n";
        o << "- Nombre: " << u.nombre << "\n";
        o << "- Dias por semana: " << u.diasDisponibles << "\n";
        if (const auto* obj = buscarObjetivo(u.objetivo)) {
            o << "- Objetivo: " << obj->etiqueta << " (zona " << obj->zona
              << ", recomendado " << obj->diasMin << "-" << obj->diasMax
              << " dias)\n";
        }
        o << "- Equipo disponible: "
          << (buscarEquipo(u.equipo) ? buscarEquipo(u.equipo)->etiqueta
                                     : u.equipo)
          << "\n";
        if (!u.musculosPrioritarios.empty()) {
            o << "- Musculos prioritarios:";
            for (const auto& m : u.musculosPrioritarios)
                o << " " << m;
            o << "\n";
        }
    } else {
        o << "\nEl usuario aun no creo su perfil.\n";
    }

    if (s.hayRutina()) {
        const Rutina& r = s.rutina();
        o << "\nRUTINA ACTUAL:\n";
        for (const auto& d : r.dias()) {
            o << "- " << d.nombre << ": ";
            bool primero = true;
            for (const auto& e : d.ejercicios) {
                if (!primero) o << ", ";
                o << e.nombre << " " << e.series << "x" << e.repeticiones;
                primero = false;
            }
            o << "\n";
        }
        o << "\nVOLUMEN SEMANAL POR MUSCULO:\n";
        for (const auto& v : analizarVolumenSemanal(r)) {
            o << "- " << v.fila.musculo << ": " << v.seriesSemana
              << " series (" << nombreEstado(v.estado) << "; MEV "
              << v.fila.mev << ", MAV " << v.fila.mav << ")\n";
        }
    }

    auto mejores = s.mejoresPRs();
    if (!mejores.empty()) {
        o << "\nRECORDS PERSONALES:\n";
        int n = 0;
        for (const auto& pr : mejores) {
            if (++n > 6) break;
            o << "- " << pr.ejercicio << ": " << pr.pesoKg << "kg x "
              << pr.repeticiones << " (" << pr.fecha << ")\n";
        }
    }

    o << "\nTABLA DE HIPERTROFIA (series/semana):\n";
    for (const auto& g : tablaHipertrofia()) {
        o << "- " << g.musculo << ": MV " << g.mv << ", MEV " << g.mev
          << ", MAV " << g.mav << ", MRV " << g.mrv << ", reps "
          << g.repeticiones << ", RIR " << g.rir << "\n";
    }

    o << "\nUsa estos datos reales cuando sean relevantes. Si preguntan por "
         "cambiar la configuracion, indicalo en la pestana Perfil; para "
         "cambiar un ejercicio concreto, boton de cambiar junto a cada "
         "ejercicio en la pestana Rutina. No inventes datos que no esten "
         "aqui.\n";
    return o.str();
}

} // namespace

CerebroIA::CerebroIA(const Servicio& servicio)
    : m_servicio(servicio),
      m_urlBase(entorno("IRONTRACK_IA_URL", "http://127.0.0.1:11434")),
      m_modelo(entorno("IRONTRACK_IA_MODELO", "llama3.2")) {}

bool CerebroIA::disponible() const {
    const auto ahora = std::chrono::steady_clock::now();
    if (ahora < m_cacheHasta) return m_estadoCache;

    httplib::Client cli(m_urlBase);
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(3, 0);
    auto res = cli.Get("/api/tags");
    m_estadoCache = false;
    m_modeloEfectivo.clear();
    if (res && res->status == 200) {
        try {
            json etiquetas = json::parse(res->body);
            const auto& modelos = etiquetas["models"];
            if (modelos.is_array() && !modelos.empty()) {
                std::string preferido;
                for (const auto& mod : modelos) {
                    std::string nombre =
                        mod.value("name", std::string());
                    if (nombre.find(m_modelo) != std::string::npos) {
                        preferido = nombre;
                        break;
                    }
                    if (preferido.empty()) preferido = nombre;
                }
                m_modeloEfectivo = preferido;
                m_estadoCache = !preferido.empty();
            }
        } catch (...) {
        }
    }
    m_cacheHasta = ahora + std::chrono::seconds(15);
    return m_estadoCache;
}

std::string CerebroIA::conversar(const std::string& pregunta) const {
    if (!disponible()) return "";

    json cuerpo = {
        {"model", m_modelo},
        {"stream", false},
        {"messages",
         json::array({
             json{{"role", "system"},
                  {"content", construirContexto(m_servicio)}},
             json{{"role", "user"}, {"content", pregunta}},
         })},
    };

    httplib::Client cli(m_urlBase);
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(300, 0);
    auto res = cli.Post("/api/chat", cuerpo.dump(), "application/json");
    if (!res || res->status != 200) return "";

    try {
        return json::parse(res->body)["message"]["content"]
            .get<std::string>();
    } catch (...) {
        return "";
    }
}
