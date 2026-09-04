#include "RepositorioMemoria.h"

bool RepositorioMemoria::guardarUsuario(const Usuario& usuario) {
    for (auto& existente : m_usuarios) {
        if (existente.nombre == usuario.nombre) {
            existente = usuario;
            return true;
        }
    }
    m_usuarios.push_back(usuario);
    return true;
}

std::vector<Usuario> RepositorioMemoria::cargarUsuarios() {
    return m_usuarios;
}

bool RepositorioMemoria::guardarPR(const RegistroPR& registro) {
    m_prs.emplace(registro.usuario, registro);
    return true;
}

std::vector<RegistroPR> RepositorioMemoria::cargarPRs(const std::string& usuario) {
    std::vector<RegistroPR> resultado;
    auto rango = m_prs.equal_range(usuario);
    for (auto it = rango.first; it != rango.second; ++it) {
        resultado.push_back(it->second);
    }
    return resultado;
}

bool RepositorioMemoria::guardarRutina(const Rutina& rutina) {
    for (auto& existente : m_rutinas) {
        if (existente.nombreUsuario() == rutina.nombreUsuario()) {
            existente = rutina;
            return true;
        }
    }
    m_rutinas.push_back(rutina);
    return true;
}

std::vector<Rutina> RepositorioMemoria::cargarRutinas(const std::string& usuario) {
    std::vector<Rutina> resultado;
    for (const auto& r : m_rutinas) {
        if (r.nombreUsuario() == usuario) {
            resultado.push_back(r);
        }
    }
    return resultado;
}
