#ifndef REPOSITORIOMEMORIA_H
#define REPOSITORIOMEMORIA_H

#include <map>
#include <string>
#include <vector>

#include "RepositorioDatos.h"

class RepositorioMemoria final : public RepositorioDatos {
public:
    bool guardarUsuario(const Usuario& usuario) override;
    std::vector<Usuario> cargarUsuarios() override;

    bool guardarPR(const RegistroPR& registro) override;
    std::vector<RegistroPR> cargarPRs(const std::string& usuario) override;

    bool guardarRutina(const Rutina& rutina) override;
    std::vector<Rutina> cargarRutinas(const std::string& usuario) override;

private:
    std::vector<Usuario> m_usuarios;
    std::multimap<std::string, RegistroPR> m_prs;
    std::vector<Rutina> m_rutinas;
};

#endif
