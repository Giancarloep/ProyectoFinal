#include "GeneradorRutinas.h"

#include <algorithm>
#include <map>
#include <set>

#include "Equipos.h"
#include "TablaHipertrofia.h"

namespace {

struct CatalogoEjercicio {
    const char* nombre;
    bool compuesto;
    int nivelEquipo;   // 0 gimnasio, 1 mancuernas, 2 peso corporal
};

using Catalogo = std::map<std::string, std::vector<CatalogoEjercicio>>;

const Catalogo& catalogo() {
    static const Catalogo pozo = {
        {"Cuadriceps",
         {{"Sentadilla", true, 2},
          {"Prensa de piernas", true, 0},
          {"Zancadas", false, 2},
          {"Extension de cuadriceps", false, 0}}},
        {"Femoral",
         {{"Peso muerto rumano", true, 1},
          {"Curl femoral", false, 0},
          {"Hiperextensiones", false, 1},
          {"Puente femoral", false, 2}}},
        {"Gluteos",
         {{"Hip thrust", true, 1},
          {"Patada de gluteo", false, 2},
          {"Puente de gluteos", false, 2}}},
        {"Pecho",
         {{"Press banca", true, 0},
          {"Press inclinado con mancuerna", true, 1},
          {"Fondos en paralelas", true, 2},
          {"Flexiones", false, 2},
          {"Flexiones declinadas", false, 2},
          {"Cruce de cables", false, 0},
          {"Aperturas con mancuerna", false, 1}}},
        {"Espalda",
         {{"Dominadas", true, 1},
          {"Remo con barra", true, 0},
          {"Jalon al pecho", true, 0},
          {"Remo con mancuerna", false, 1},
          {"Remo invertido", true, 2},
          {"Superman espalda", false, 2},
          {"Pullover en polea", false, 0}}},
        {"Hombro",
         {{"Press militar", true, 0},
          {"Press Arnold", true, 1},
          {"Elevaciones laterales", false, 1},
          {"Face pull", false, 1},
          {"Pajaros", false, 1},
          {"Flexiones pike", true, 2}}},
        {"Biceps",
         {{"Curl con barra", false, 0},
          {"Curl martillo", false, 1},
          {"Curl inclinado", false, 1},
          {"Curl en polea", false, 0},
          {"Curl con toalla", false, 2}}},
        {"Triceps",
         {{"Extension de triceps en polea", false, 0},
          {"Press frances", false, 1},
          {"Rompecraneos", false, 1},
          {"Fondos en banco", false, 2},
          {"Flexiones diamante", false, 2}}},
        {"Trapecio",
         {{"Encogimientos con barra", false, 0},
          {"Encogimientos con mancuernas", false, 1}}},
        {"Gemelos",
         {{"Elevacion de gemelos de pie", false, 2},
          {"Elevacion de gemelos sentado", false, 1}}},
        {"Abdomen",
         {{"Crunch", false, 2},
          {"Plancha", false, 2},
          {"Elevacion de piernas colgado", false, 1},
          {"Rueda abdominal", false, 1}}},
    };
    return pozo;
}

struct PlanDia {
    const char* nombre;
    std::vector<std::string> grupos;
};

std::vector<PlanDia> planSemanal(int dias) {
    if (dias >= 6) {
        std::vector<PlanDia> plan = {
            {"Empuje", {"Pecho", "Hombro", "Triceps"}},
            {"Traccion", {"Espalda", "Biceps", "Trapecio"}},
            {"Pierna", {"Cuadriceps", "Femoral", "Gemelos"}},
        };
        while (plan.size() < static_cast<std::size_t>(dias)) {
            PlanDia extra = plan[plan.size() % 3];
            plan.push_back(extra);
        }
        return plan;
    }
    if (dias >= 4) {
        std::vector<PlanDia> plan = {
            {"Torso A", {"Pecho", "Espalda", "Hombro", "Biceps", "Triceps"}},
            {"Pierna A", {"Cuadriceps", "Femoral", "Gemelos", "Abdomen"}},
            {"Torso B", {"Pecho", "Espalda", "Hombro", "Biceps", "Triceps"}},
            {"Pierna B", {"Cuadriceps", "Femoral", "Gemelos", "Abdomen"}},
        };
        if (dias >= 5) {
            plan.push_back({"Hombro y brazos",
                            {"Hombro", "Biceps", "Triceps", "Abdomen"}});
        }
        return plan;
    }
    std::vector<PlanDia> plan;
    for (int i = 0; i < dias; ++i) {
        plan.push_back({"Cuerpo completo",
                        {"Cuadriceps", "Pecho", "Espalda", "Hombro", "Biceps",
                         "Triceps", "Gemelos", "Abdomen"}});
    }
    return plan;
}

bool esPrioritario(const Usuario& usuario, const std::string& musculo) {
    for (const auto& p : usuario.musculosPrioritarios) {
        if (p == musculo || (buscarGrupoTabla(p) &&
                             buscarGrupoTabla(p)->musculo == musculo)) {
            return true;
        }
    }
    return false;
}

int volumenSemanalObjetivo(const GrupoTabla& grupo, const Usuario& usuario,
                           bool prioridad) {
    int objetivo = volumenObjetivo(grupo, usuario.objetivo);
    if (prioridad) {
        objetivo = std::min((objetivo + grupo.mavMax) / 2, grupo.mrvMin - 1);
    }
    return std::max(objetivo, grupo.mvMin);
}

int repeticionesPara(const GrupoTabla& grupo, bool compuesto) {
    const int rango = grupo.repsMax - grupo.repsMin;
    int reps =
        compuesto ? grupo.repsMin + rango / 3 : grupo.repsMax - rango / 3;
    if (reps < 2) reps = 2;
    return reps;
}

std::vector<CatalogoEjercicio> pozoFiltrado(const std::string& musculo,
                                            int nivelUsuario,
                                            const std::string* excluir) {
    std::vector<CatalogoEjercicio> resultado;
    auto it = catalogo().find(musculo);
    if (it == catalogo().end()) return resultado;
    for (const auto& ce : it->second) {
        if (ce.nivelEquipo < nivelUsuario) continue;
        if (excluir && ce.nombre == *excluir) continue;
        resultado.push_back(ce);
    }
    return resultado;
}

} // namespace

Rutina GeneradorRutinas::generar(const Usuario& usuario) {
    Rutina rutina(usuario.nombre);

    int dias = usuario.diasDisponibles;
    if (dias < 1) dias = 1;
    if (dias > 7) dias = 7;

    const InfoEquipo* equipo = buscarEquipo(usuario.equipo);
    const int nivelUsuario = equipo ? equipo->nivel : 0;

    std::vector<PlanDia> plan = planSemanal(dias);

    std::map<std::string, int> sesionesPorGrupo;
    for (const auto& dia : plan) {
        std::set<std::string> unicos(dia.grupos.begin(), dia.grupos.end());
        for (const auto& g : unicos) sesionesPorGrupo[g]++;
    }

    for (const auto& nombre : usuario.musculosPrioritarios) {
        const GrupoTabla* fila = buscarGrupoTabla(nombre);
        if (!fila) continue;
        if (sesionesPorGrupo[fila->musculo] > 0) continue;

        auto objetivo = std::min_element(
            plan.begin(), plan.end(),
            [](const PlanDia& a, const PlanDia& b) {
                return a.grupos.size() < b.grupos.size();
            });
        objetivo->grupos.push_back(fila->musculo);
        sesionesPorGrupo[fila->musculo]++;
    }

    int indiceDia = 0;
    for (const auto& planDia : plan) {
        DiaEntrenamiento dia;
        dia.nombre = "Dia " + std::to_string(indiceDia + 1) + " - " +
                     planDia.nombre;

        for (const auto& nombreGrupo : planDia.grupos) {
            const GrupoTabla* grupo = buscarGrupoTabla(nombreGrupo);
            if (!grupo) continue;

            auto pool =
                pozoFiltrado(grupo->musculo, nivelUsuario, nullptr);
            if (pool.empty()) continue;

            const bool prioridad = esPrioritario(usuario, grupo->musculo);
            const int sesiones =
                std::max(1, sesionesPorGrupo[grupo->musculo]);
            int objetivoSeries =
                volumenSemanalObjetivo(*grupo, usuario, prioridad) / sesiones;
            objetivoSeries = std::clamp(objetivoSeries, 2, 10);

            int seriesRestantes = objetivoSeries;
            for (std::size_t i = 0;
                 i < pool.size() && seriesRestantes >= 2; ++i) {
                const CatalogoEjercicio& ce =
                    pool[(indiceDia + i) % pool.size()];
                int seriesEjercicio = ce.compuesto ? 4 : 3;
                if (seriesRestantes < 3 && !ce.compuesto) {
                    seriesEjercicio = seriesRestantes;
                }
                dia.ejercicios.emplace_back(
                    ce.nombre, grupo->musculo, seriesEjercicio,
                    repeticionesPara(*grupo, ce.compuesto));
                seriesRestantes -= seriesEjercicio;
            }
        }

        rutina.agregarDia(std::move(dia));
        indiceDia++;
    }

    return rutina;
}

std::vector<Ejercicio> GeneradorRutinas::alternativasPara(
    const Usuario& usuario, const std::string& grupoMuscular,
    const std::string& excluirNombre) {
    std::vector<Ejercicio> opciones;

    const GrupoTabla* grupo = buscarGrupoTabla(grupoMuscular);
    if (!grupo) return opciones;

    const InfoEquipo* equipo = buscarEquipo(usuario.equipo);
    const int nivelUsuario = equipo ? equipo->nivel : 0;

    auto pool = pozoFiltrado(grupo->musculo, nivelUsuario, &excluirNombre);
    for (const auto& ce : pool) {
        opciones.emplace_back(ce.nombre, grupo->musculo, ce.compuesto ? 4 : 3,
                              repeticionesPara(*grupo, ce.compuesto));
    }
    return opciones;
}

std::optional<Ejercicio> GeneradorRutinas::crearEjercicio(
    const Usuario& usuario, const std::string& nombre) {
    const InfoEquipo* equipo = buscarEquipo(usuario.equipo);
    const int nivelUsuario = equipo ? equipo->nivel : 0;

    for (const auto& [grupoNombre, lista] : catalogo()) {
        for (const auto& ce : lista) {
            if (ce.nombre != nombre || ce.nivelEquipo < nivelUsuario) continue;
            const GrupoTabla* grupo = buscarGrupoTabla(grupoNombre);
            if (!grupo) continue;
            return Ejercicio(ce.nombre, grupo->musculo,
                             ce.compuesto ? 4 : 3,
                             repeticionesPara(*grupo, ce.compuesto));
        }
    }
    return std::nullopt;
}
