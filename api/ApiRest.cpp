#include "ApiRest.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <vector>

#include "Asistente.h"
#include "CerebroIA.h"
#include "Equipos.h"
#include "Servicio.h"
#include "TiersSaiyayin.h"

using json = nlohmann::json;

namespace {

json ejercicioToJson(const Ejercicio& e) {
    json base = json{
        {"nombre", e.nombre},
        {"grupoMuscular", e.grupoMuscular},
        {"series", e.series},
        {"repeticiones", e.repeticiones},
    };
    if (const GrupoTabla* g = buscarGrupoTabla(e.grupoMuscular)) {
        base["rir"] = g->rir;
    }
    return base;
}

json diaToJson(const DiaEntrenamiento& d) {
    json ejercicios = json::array();
    for (const auto& e : d.ejercicios) ejercicios.push_back(ejercicioToJson(e));
    return json{{"nombre", d.nombre}, {"ejercicios", ejercicios}};
}

json rutinaToJson(const Rutina& r) {
    json dias = json::array();
    for (const auto& d : r.dias()) dias.push_back(diaToJson(d));
    return json{{"usuario", r.nombreUsuario()}, {"dias", dias}};
}

json usuarioToJson(const Usuario& u) {
    json prioritarios = json::array();
    for (const auto& m : u.musculosPrioritarios) prioritarios.push_back(m);

    json objetivoInfo = json{{"clave", u.objetivo}};
    if (const InfoObjetivo* info = buscarObjetivo(u.objetivo)) {
        objetivoInfo = json{
            {"clave", info->clave},
            {"etiqueta", info->etiqueta},
            {"zona", info->zona},
            {"diasRecomendados", json::array({info->diasMin, info->diasMax})},
            {"fueraDeRango", u.diasDisponibles < info->diasMin ||
                                 u.diasDisponibles > info->diasMax},
        };
    }

    return json{
        {"nombre", u.nombre},
        {"diasDisponibles", u.diasDisponibles},
        {"pesoCorporal", u.pesoCorporal},
        {"altura", u.altura},
        {"edad", u.edad},
        {"sexo", u.sexo},
        {"tipoSplit", Servicio::tipoSplit(u.diasDisponibles)},
        {"musculosPrioritarios", prioritarios},
        {"objetivo", objetivoInfo},
        {"equipo", buscarEquipo(u.equipo)
                       ? json{{"clave", u.equipo},
                              {"etiqueta", buscarEquipo(u.equipo)->etiqueta}}
                       : json{{"clave", u.equipo}, {"etiqueta", u.equipo}}},
    };
}

json grupoTablaToJson(const GrupoTabla& g) {
    return json{
        {"musculo", g.musculo},
        {"referencia", g.referencia},
        {"mv", g.mv},
        {"mev", g.mev},
        {"mav", g.mav},
        {"mrv", g.mrv},
        {"frecuencia", g.frecuencia},
        {"repeticiones", g.repeticiones},
        {"rir", g.rir},
    };
}

void responderJson(httplib::Response& res, const json& cuerpo, int estado = 200) {
    res.status = estado;
    res.set_content(cuerpo.dump(), "application/json; charset=utf-8");
}

void responderError(httplib::Response& res, int estado, const char* mensaje) {
    responderJson(res, json{{"error", mensaje}}, estado);
}

} // namespace

void configurarApi(httplib::Server& servidor, Servicio& servicio) {

    static CerebroIA cerebro(servicio);
    static Asistente asistente(servicio, cerebro);

    servidor.Get("/api/asistente/estado",
                 [&cerebro](const httplib::Request&, httplib::Response& res) {
                     responderJson(res, json{{"modeloLocal", cerebro.disponible()},
                                             {"modelo", cerebro.modeloActual()}});
                 });

    servidor.Get("/api/equipos",
                 [](const httplib::Request&, httplib::Response& res) {
                     json lista = json::array();
                     for (const auto& e : equiposDisponibles()) {
                         lista.push_back(json{{"clave", e.clave},
                                              {"etiqueta", e.etiqueta}});
                     }
                     responderJson(res, json{{"equipos", lista}});
                 });

    servidor.Get("/api/rutina/alternativas",
                 [&servicio](const httplib::Request& req,
                             httplib::Response& res) {
                     if (!servicio.hayRutina()) {
                         return responderError(res, 404, "sin rutina");
                     }
                     try {
                         int dia = std::stoi(req.get_param_value("dia"));
                         int indice = std::stoi(req.get_param_value("indice"));
                         auto opciones = servicio.alternativasDe(dia, indice);
                         json lista = json::array();
                         for (const auto& e : opciones) {
                             lista.push_back(ejercicioToJson(e));
                         }
                         responderJson(res, json{{"alternativas", lista}});
                     } catch (const std::exception&) {
                         responderError(res, 400, "parametros invalidos");
                     }
                 });

    servidor.Post("/api/rutina/cambiar",
                  [&servicio](const httplib::Request& req,
                              httplib::Response& res) {
                      try {
                          json cuerpo = json::parse(req.body);
                          int dia = cuerpo.at("dia").get<int>();
                          int indice = cuerpo.at("indice").get<int>();
                          std::string nuevo =
                              cuerpo.at("nuevo").get<std::string>();
                          if (!servicio.cambiarEjercicio(dia, indice, nuevo)) {
                              return responderError(
                                  res, 400,
                                  "no se pudo cambiar el ejercicio");
                          }
                          responderJson(res, rutinaToJson(servicio.rutina()));
                      } catch (const json::exception&) {
                          responderError(res, 400, "json invalido");
                      }
                  });

    servidor.Get("/api/objetivos",
                 [](const httplib::Request&, httplib::Response& res) {
                     json lista = json::array();
                     for (const auto& o : objetivosEntrenamiento()) {
                         lista.push_back(json{
                             {"clave", o.clave},
                             {"etiqueta", o.etiqueta},
                             {"descripcion", o.descripcion},
                             {"zona", o.zona},
                             {"diasRecomendados",
                              json::array({o.diasMin, o.diasMax})},
                         });
                     }
                     responderJson(res, json{{"objetivos", lista}});
                 });

    servidor.Post("/api/asistente",
                  [&asistente](const httplib::Request& req,
                               httplib::Response& res) {
                      try {
                          json cuerpo = json::parse(req.body);
                          std::string pregunta =
                              cuerpo.at("pregunta").get<std::string>();
                          responderJson(
                              res,
                              json{{"respuesta", asistente.responder(pregunta)}});
                      } catch (const json::exception&) {
                          responderError(res, 400, "json invalido");
                      }
                  });

    servidor.Get("/api/perfil",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     if (!servicio.hayPerfil()) {
                         return responderError(res, 404, "sin perfil");
                     }
                     responderJson(res, usuarioToJson(servicio.usuario()));
                 });

    servidor.Post("/api/perfil",
                  [&servicio](const httplib::Request& req, httplib::Response& res) {
                      try {
                          json cuerpo = json::parse(req.body);
                          std::string nombre =
                              cuerpo.at("nombre").get<std::string>();
                          int dias =
                              cuerpo.at("diasDisponibles").get<int>();
                          if (nombre.empty()) {
                              return responderError(res, 400, "nombre vacio");
                          }
                          std::vector<std::string> prioritarios;
                          if (cuerpo.contains("musculosPrioritarios") &&
                              cuerpo["musculosPrioritarios"].is_array()) {
                              for (const auto& m :
                                   cuerpo["musculosPrioritarios"]) {
                                  prioritarios.push_back(
                                      m.get<std::string>());
                              }
                          }
                          std::string objetivo = "progresar";
                          if (cuerpo.contains("objetivo") &&
                              cuerpo["objetivo"].is_string()) {
                              objetivo =
                                  cuerpo["objetivo"].get<std::string>();
                          }
                          std::string equipo = "gimnasio";
                          if (cuerpo.contains("equipo") &&
                              cuerpo["equipo"].is_string()) {
                              equipo = cuerpo["equipo"].get<std::string>();
                          }
                          double pesoCorporal = 0.0;
                          if (cuerpo.contains("pesoCorporal") &&
                              cuerpo["pesoCorporal"].is_number()) {
                              pesoCorporal =
                                  cuerpo["pesoCorporal"].get<double>();
                          }
                          double altura = 0.0;
                          if (cuerpo.contains("altura") &&
                              cuerpo["altura"].is_number()) {
                              altura =
                                  cuerpo["altura"].get<double>();
                          }
                          int edad = 0;
                          if (cuerpo.contains("edad") &&
                              cuerpo["edad"].is_number()) {
                              edad =
                                  cuerpo["edad"].get<int>();
                          }
                          std::string sexo = "hombre";
                          if (cuerpo.contains("sexo") &&
                              cuerpo["sexo"].is_string()) {
                              sexo =
                                  cuerpo["sexo"].get<std::string>();
                          }
                          Usuario usuario = servicio.crearPerfil(
                              nombre, dias, prioritarios, objetivo, equipo,
                              pesoCorporal, altura, edad, sexo);
                          responderJson(res, usuarioToJson(usuario));
                      } catch (const json::exception&) {
                          responderError(res, 400, "json invalido");
                      }
                  });

    servidor.Get("/api/rutina",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     if (!servicio.hayRutina()) {
                         return responderError(res, 404, "sin rutina");
                     }
                     responderJson(res, rutinaToJson(servicio.rutina()));
                 });

    servidor.Post("/api/rutina/generar",
                  [&servicio](const httplib::Request&, httplib::Response& res) {
                      if (!servicio.hayPerfil()) {
                          return responderError(res, 400,
                                                "crea un perfil primero");
                      }
                      responderJson(res, rutinaToJson(servicio.generarRutina()));
                  });

    servidor.Get("/api/ejercicios",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     json lista = json::array();
                     for (const auto& n : servicio.ejerciciosDeRutina()) {
                         lista.push_back(n);
                     }
                     responderJson(res, json{{"ejercicios", lista}});
                 });

    servidor.Get("/api/prs",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     json prs = json::array();
                     for (const auto& r : servicio.mejoresPRs()) {
                         prs.push_back(json{
                             {"ejercicio", r.ejercicio},
                             {"pesoKg", r.pesoKg},
                             {"repeticiones", r.repeticiones},
                             {"fecha", r.fecha},
                             {"rm1Estimado", TrackerPRs::estimar1RM(
                                                 r.pesoKg, r.repeticiones)},
                         });
                     }
                     responderJson(res, json{{"prs", prs}});
                 });

    servidor.Post("/api/prs",
                  [&servicio](const httplib::Request& req,
                              httplib::Response& res) {
                      try {
                          json cuerpo = json::parse(req.body);
                          std::string ejercicio =
                              cuerpo.at("ejercicio").get<std::string>();
                          double pesoKg =
                              cuerpo.at("pesoKg").get<double>();
                          int repeticiones =
                              cuerpo.at("repeticiones").get<int>();
                          if (ejercicio.empty() || pesoKg <= 0 ||
                              repeticiones < 1 || repeticiones > 100) {
                              return responderError(res, 400,
                                                    "datos fuera de rango");
                          }
                          ResultadoPR resultado =
                              servicio.registrarPR(ejercicio, pesoKg,
                                                   repeticiones);
                          responderJson(res, json{
                                                 {"nuevoPR", resultado.nuevoPR},
                                                 {"rm1Estimado",
                                                  resultado.rm1Estimado},
                                                 {"ratio", resultado.ratio},
                                                 {"tier", resultado.tier},
                                                 {"tierSubio",
                                                  resultado.tierSubio},
                                             });
                      } catch (const json::exception&) {
                          responderError(res, 400, "json invalido");
                      }
                  });

    servidor.Get("/api/tiers",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     json tabla = json::array();
                     for (const auto& t : tablaTiers()) {
                         tabla.push_back(json{
                             {"nombre", t.nombre},
                             {"umbral", t.umbralRatio},
                             {"color", t.color},
                         });
                     }
                     json cuerpo = json{{"tiers", tabla},
                                        {"conPeso", false}};
                     if (servicio.hayPerfil()) {
                         const EstadoTiers e = servicio.estadoTiers();
                         cuerpo["conPeso"] = e.conPeso;
                         cuerpo["pesoCorporal"] = e.pesoCorporal;
                         cuerpo["tierActual"] = e.tierActual;
                         cuerpo["colorActual"] = e.colorActual;
                         cuerpo["mejorEjercicio"] = e.mejorEjercicio;
                         cuerpo["mejorRatio"] = e.mejorRatio;
                         cuerpo["siguienteTier"] = e.siguienteTier;
                         cuerpo["siguienteUmbral"] = e.siguienteUmbral;
                         json porEjercicio = json::array();
                         for (const auto& t : e.porEjercicio) {
                             porEjercicio.push_back(json{
                                 {"ejercicio", t.ejercicio},
                                 {"rm1Estimado", t.rm1Estimado},
                                 {"ratio", t.ratio},
                                 {"tier", t.tier},
                                 {"color", t.color},
                             });
                         }
                         cuerpo["porEjercicio"] = porEjercicio;
                     }
                     responderJson(res, cuerpo);
                 });

    servidor.Get("/api/tabla",
                 [](const httplib::Request&, httplib::Response& res) {
                     json filas = json::array();
                     for (const auto& g : tablaHipertrofia()) {
                         filas.push_back(grupoTablaToJson(g));
                     }
                     responderJson(res, json{{"tabla", filas}});
                 });

    servidor.Get("/api/volumen",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     if (!servicio.hayRutina()) {
                         return responderError(res, 404, "sin rutina");
                     }
                     json volumen = json::array();
                     for (const auto& v : servicio.volumenSemanal()) {
                         volumen.push_back(json{
                             {"musculo", v.fila.musculo},
                             {"referencia", v.fila.referencia},
                             {"seriesSemana", v.seriesSemana},
                             {"mev", v.fila.mev},
                             {"mav", v.fila.mav},
                             {"estado", nombreEstado(v.estado)},
                         });
                     }
                     responderJson(res, json{{"volumen", volumen}});
                 });

    servidor.Post("/api/sesion/iniciar",
                  [&servicio](const httplib::Request&, httplib::Response& res) {
                      servicio.iniciarSesionEntrenamiento();
                      responderJson(res, json{{"activa", true}, {"segundos", 0}});
                  });

    servidor.Get("/api/sesion",
                 [&servicio](const httplib::Request&, httplib::Response& res) {
                     responderJson(res, json{
                                            {"activa", servicio.sesionActiva()},
                                            {"segundos",
                                             servicio.segundosSesion()},
                                        });
                 });
}
