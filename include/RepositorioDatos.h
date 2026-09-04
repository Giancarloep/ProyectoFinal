#ifndef REPOSITORIODATOS_H
#define REPOSITORIODATOS_H

#include <string>
#include <vector>

#include "Rutina.h"
#include "TrackingPR.h"
#include "Usuario.h"

class RepositorioDatos {
public:
    virtual ~RepositorioDatos() = default;

    virtual bool guardarUsuario(const Usuario& usuario) = 0;
    virtual std::vector<Usuario> cargarUsuarios() = 0;

    virtual bool guardarPR(const RegistroPR& registro) = 0;
    virtual std::vector<RegistroPR> cargarPRs(const std::string& usuario) = 0;

    virtual bool guardarRutina(const Rutina& rutina) = 0;
    virtual std::vector<Rutina> cargarRutinas(const std::string& usuario) = 0;
};

#endif
