#ifndef APIREST_H
#define APIREST_H

#include <httplib.h>

class Servicio;

void configurarApi(httplib::Server& servidor, Servicio& servicio);

#endif
