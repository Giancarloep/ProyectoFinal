#include "Asistente.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

#include "CerebroIA.h"
#include "Equipos.h"
#include "Servicio.h"
#include "TablaHipertrofia.h"

namespace {

std::string minusculas(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(
                            static_cast<unsigned char>(c)));
    return s;
}

void reemplazar(std::string& s, const std::string& de, const std::string& a) {
    std::size_t pos = 0;
    while ((pos = s.find(de, pos)) != std::string::npos) {
        s.replace(pos, de.size(), a);
        pos += a.size();
    }
}

std::string normalizar(const std::string& texto) {
    std::string s = minusculas(texto);
    const std::pair<const char*, const char*> mapa[] = {
        {"a", "a"}, {"e", "e"}, {"i", "i"}, {"o", "o"},
        {"u", "u"}, {"u", "u"}, {"n", "n"},
    };
    for (const auto& [de, a] : mapa) reemplazar(s, de, a);
    return s;
}

bool contiene(const std::string& texto, const std::vector<std::string>& claves) {
    for (const auto& c : claves) {
        if (texto.find(c) != std::string::npos) return true;
    }
    return false;
}

const GrupoTabla* musculoMencionado(const std::string& q) {
    for (const auto& fila : tablaHipertrofia()) {
        if (q.find(minusculas(fila.musculo)) != std::string::npos ||
            q.find(minusculas(fila.referencia)) != std::string::npos) {
            return &fila;
        }
        for (const auto& alias : fila.alias) {
            if (!alias.empty() && q.find(minusculas(alias)) != std::string::npos) {
                return &fila;
            }
        }
    }
    return nullptr;
}

std::string estadoLegible(EstadoVolumen estado) {
    switch (estado) {
        case EstadoVolumen::Insuficiente: return "por debajo del mantenimiento";
        case EstadoVolumen::Mantenimiento: return "en zona de mantenimiento";
        case EstadoVolumen::Optimo: return "en zona optima";
        case EstadoVolumen::Alto: return "alto, cerca del MRV";
        case EstadoVolumen::Excesivo: return "superando el MRV";
    }
    return "";
}

} // namespace

bool Asistente::intentarReglas(const std::string& pregunta,
                               std::string& respuesta) const {
    const std::string q = normalizar(pregunta);
    std::ostringstream r;
    const Servicio& s = m_servicio;

    if (contiene(q, {"hola", "buenas", "hey", "que tal", "saludos"})) {
        r << "Hola! Soy tu entrenador virtual. Puedo ayudarte con:\n"
          << "- Tu rutina y como esta distribuida\n"
          << "- Analisis de volumen semanal por musculo\n"
          << "- La tabla de hipertrofia (MV/MEV/MAV/MRV)\n"
          << "- Tus PRs y 1RM estimado\n"
          << "- Cuantos dias deberias entrenar segun tu objetivo\n"
          << "- Descanso, dieta y suplementacion basica\n"
          << "Preguntame lo que quieras.";
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"gracias", "genial", "perfecto", "excelente"})) {
        respuesta =
            "Para eso estoy. Recuerda que la constancia gana siempre a la "
            "perfeccion!";
        return true;
    }

    if (contiene(q, {"ayuda", "que puedes", "como funciona", "help"})) {
        respuesta =
            "Prueba a preguntarme: \"como esta mi rutina\", \"analiza mi "
            "volumen\", \"cuantos dias debo entrenar\", \"explicame la tabla "
            "de hipertrofia\", \"mis mejores prs\" o menciona un musculo como "
            "\"pecho\" o \"espalda\".";
        return true;
    }

    const GrupoTabla* musculo = musculoMencionado(q);

    if (musculo &&
        contiene(q, {"volumen", "series", "semana", "mev", "mav", "mrv",
                     "tabla", "datos", "info"})) {
        for (const auto& v : analizarVolumenSemanal(s.rutina())) {
            if (v.fila.musculo != musculo->musculo) continue;
            r << musculo->musculo << " (" << musculo->referencia << "):\n"
              << "- MV: " << musculo->mv << " | MEV: " << musculo->mev
              << " | MAV: " << musculo->mav << " | MRV: " << musculo->mrv
              << "\n"
              << "- Frecuencia ideal: " << musculo->frecuencia << "\n"
              << "- Reps: " << musculo->repeticiones << " | RIR: "
              << musculo->rir << "\n";
            if (s.hayRutina()) {
                r << "- Con tu rutina actual haces " << v.seriesSemana
                  << " series/semana, estas " << estadoLegible(v.estado)
                  << ".\n";
                if (v.estado == EstadoVolumen::Insuficiente)
                    r << "  Sugerencia: agrega una serie mas por sesion o un ejercicio extra.\n";
                else if (v.estado == EstadoVolumen::Excesivo)
                    r << "  Sugerencia: baja 2-3 series semanales para recuperar mejor.\n";
            } else {
                r << "- Genera una rutina para evaluar tu volumen actual.\n";
            }
            respuesta = r.str();
            return true;
        }
    }

    if (musculo) {
        r << "Sobre " << musculo->musculo << ": MEV " << musculo->mev
          << ", MAV " << musculo->mav << ", MRV " << musculo->mrv
          << " series/semana. Frecuencia " << musculo->frecuencia
          << ", repeticiones " << musculo->repeticiones << " y RIR "
          << musculo->rir << ".\n";
        if (const auto* obj = buscarObjetivo(s.usuario().objetivo)) {
            r << "Con tu objetivo \"" << obj->etiqueta
              << "\" apuntamos a ~" << volumenObjetivo(*musculo, obj->clave)
              << " series semanales";
            if (s.hayRutina()) {
                for (const auto& v : analizarVolumenSemanal(s.rutina())) {
                    if (v.fila.musculo == musculo->musculo) {
                        r << "; ahora mismo llevas " << v.seriesSemana;
                        break;
                    }
                }
            }
            r << ".\n";
        }
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"rutina", "plan", "entreno", "entrenamiento",
                     "ejercicios"})) {
        if (!s.hayRutina()) {
            respuesta =
                "Todavia no tienes rutina generada. Crea tu perfil en la "
                "pestana Perfil y pulsa \"Generar rutina\": la construyo "
                "usando la tabla de hipertrofia.";
            return true;
        }
        const Rutina& rutina = s.rutina();
        int totalEjercicios = 0;
        int totalSeries = 0;
        for (const auto& d : rutina.dias())
            for (const auto& e : d.ejercicios) {
                totalEjercicios++;
                totalSeries += e.series;
            }
        r << "Tu rutina actual (" << rutina.dias().size()
          << " dias/semana, " << totalEjercicios << " ejercicios, ~"
          << totalSeries << " series totales):\n";
        for (const auto& d : rutina.dias()) {
            r << "- " << d.nombre << " (" << d.ejercicios.size()
              << " ejercicios)\n";
        }
        if (const auto* obj = buscarObjetivo(s.usuario().objetivo)) {
            r << "Esta disenada para \"" << obj->etiqueta << "\".";
        }
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"volumen", "series por semana", "analiza", "analisis"})) {
        if (!s.hayRutina()) {
            respuesta =
                "Primero genera una rutina y luego te analizo el volumen "
                "semanal musculo por musculo.";
            return true;
        }
        auto volumen = analizarVolumenSemanal(s.rutina());
        int optimos = 0;
        std::vector<std::pair<std::string, std::string>> problemas;
        for (const auto& v : volumen) {
            if (v.estado == EstadoVolumen::Optimo) {
                optimos++;
                continue;
            }
            problemas.emplace_back(
                v.fila.musculo,
                std::to_string(v.seriesSemana) + " series (" +
                    estadoLegible(v.estado) + ")");
        }
        r << "De " << volumen.size() << " musculos evaluados, " << optimos
          << " estan en zona optima.\n";
        if (problemas.empty()) {
            r << "Todo el volumen esta dentro de rangos saludables. Buen trabajo!";
        } else {
            r << "Atencion en:\n";
            for (const auto& [nombre, detalle] : problemas) {
                r << "- " << nombre << ": " << detalle << "\n";
            }
        }
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"pr", "record", "records", "marca", "1rm",
                     "peso maximo"})) {
        auto mejores = s.mejoresPRs();
        if (mejores.empty()) {
            respuesta =
                "Aun no registraste ningun PR. Ve a la pestana PRs y anota tus "
                "levantamientos; yo calculo el 1RM estimado (formula de Epley).";
            return true;
        }
        std::sort(mejores.begin(), mejores.end(),
                  [](const RegistroPR& a, const RegistroPR& b) {
                      return TrackerPRs::estimar1RM(a.pesoKg,
                                                    a.repeticiones) >
                             TrackerPRs::estimar1RM(b.pesoKg, b.repeticiones);
                  });
        r << "Tus records registrados:\n";
        for (std::size_t i = 0; i < mejores.size() && i < 5; ++i) {
            r << "- " << mejores[i].ejercicio << ": " << mejores[i].pesoKg
              << "kg x " << mejores[i].repeticiones << " (1RM est. "
              << TrackerPRs::estimar1RM(mejores[i].pesoKg,
                                        mejores[i].repeticiones)
              << "kg)\n";
        }
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"dia", "dias", "frecuencia semanal",
                     "veces por semana"})) {
        const Usuario& u = s.usuario();
        const InfoObjetivo* obj =
            buscarObjetivo(u.objetivo.empty() ? "progresar" : u.objetivo);
        if (!obj) {
            respuesta =
                "Crea tu perfil primero y te recomiendo los dias ideales.";
            return true;
        }
        r << "Para tu objetivo \"" << obj->etiqueta << "\" lo ideal son "
          << obj->diasMin << "-" << obj->diasMax
          << " dias de gimnasio por semana.\n";
        if (s.hayPerfil()) {
            r << "Ahora tienes configurados " << u.diasDisponibles << ".";
            if (u.diasDisponibles < obj->diasMin)
                r << " Te quedas corto: podrias sumar al menos uno.";
            else if (u.diasDisponibles > obj->diasMax)
                r << " Vas pasado: cuidado con la recuperacion.";
            else
                r << " Estas justo en el rango recomendado.";
        }
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"casa", "gimnasio", "crossfit", "equipo", "peso corporal",
                     "mancuernas", "no tengo barra", "sin barra", "maquinas"})) {
        const InfoEquipo* eq = buscarEquipo(s.usuario().equipo);
        r << "Tu configuracion actual es: \""
          << (eq ? eq->etiqueta : "gimnasio")
          << "\". La rutina se adapta automaticamente al equipo que tengas:\n"
          << "- Gimnasio / CrossFit: barras, maquinas y poleas.\n"
          << "- Casa con mancuernas: sin maquinas ni barras largas.\n"
          << "- Solo peso corporal: nada mas que tu cuerpo.\n"
          "Cambialo desde el Perfil y regenera la rutina; tambien puedes "
          "cambiar cualquier ejercicio concreto con el boton de cambiar junto "
          "a cada ejercicio.";
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"descanso", "entre series", "dormir", "sueno"})) {
        respuesta =
            "Descanso recomendado:\n"
            "- Ejercicios compuestos pesados (sentadilla, press, remo): 2-5 min entre series.\n"
            "- Accesorios: 1-2 min.\n"
            "- Dormir 7-9 horas: es donde realmente crece el musculo.\n"
            "- Un musculo necesita 48-72h entre sesiones duras; por eso el split importa.";
        return true;
    }

    if (contiene(q, {"dieta", "proteina", "comida", "calorias", "nutricion"})) {
        respuesta =
            "Bases nutricionales para hipertrofia:\n"
            "- Proteina: 1.6-2.2 g por kg de peso corporal al dia.\n"
            "- Para ganar musculo: superavit ligero de +200-300 kcal.\n"
            "- Para definir: deficit suave manteniendo la proteina alta.\n"
            "- No descuides los carbohidratos: son el combustible del entreno.";
        return true;
    }

    if (contiene(q, {"suplemento", "creatina", "esteroides", "sustancias",
                     "doping"})) {
        respuesta =
            "Sobre suplementacion:\n"
            "- Con evidencia solida: creatina monohidratada (3-5 g/dia), cafeina pre-entreno y proteina en polvo si no "
            "llegas con comida.\n"
            "- Los esteroides anabolicos tienen riesgos serios de salud y solo deben manejarse con supervision "
            "medica.\n"
            "- En la app el modo \"Entrenamiento avanzado\" refleja volumenes tipo zona naranja-roja de la tabla, "
            "pensados para quien tiene recuperacion muy por encima de lo natural.";
        return true;
    }

    if (contiene(q, {"objetivo", "meta", "mantener", "progresar", "estancado",
                     "estancamiento"})) {
        r << "Los objetivos disponibles y su zona de la tabla:\n";
        for (const auto& o : objetivosEntrenamiento()) {
            r << "- " << o.etiqueta << " (" << o.zona << ", " << o.diasMin
              << "-" << o.diasMax << " dias): " << o.descripcion << "\n";
        }
        r << "Cambialo cuando quieras desde el perfil y regenera la rutina.";
        respuesta = r.str();
        return true;
    }

    if (contiene(q, {"tabla", "hipertrofia", "mev", "mav", "mrv", "mv"})) {
        respuesta =
            "La tabla de hipertrofia guia cuantas series semanales necesita "
            "cada musculo:\n"
            "- MV (volumen de mantenimiento): lo minimo para no perder.\n"
            "- MEV (minimo efectivo): lo minimo para seguir creciendo - zona verde.\n"
            "- MAV (maximo adaptable): donde mas progreso se adapta - zona naranja.\n"
            "- MRV (maximo recuperable): el techo; pasarse lleva a sobreentrenar - zona roja.\n"
            "Menciona un musculo (pecho, espalda, biceps...) y te doy sus numeros exactos.";
        return true;
    }

    return false;
}

std::string Asistente::responder(const std::string& pregunta) const {
    std::string ia = m_cerebro.conversar(pregunta);
    if (!ia.empty()) return ia;

    return "Ollama no esta disponible. Instala Ollama y ejecuta 'ollama run llama3.2' "
           "para usar el asistente IA.";
}