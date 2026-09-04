#include <string>
#include <vector>

struct DefinicionTier {
    const char* nombre;
    double umbralRatio;   // ratio minimo: 1RM estimado / peso corporal
    const char* color;    // clave CSS para el frontend
};

// Tiers estilo Saiyajin: cada umbral es el ratio de fuerza sobre el
// peso corporal del usuario. 0.8 = levanta 20% menos que su peso.
const std::vector<DefinicionTier>& tablaTiers();

int indiceTier(double ratio);

const DefinicionTier& tierDe(double ratio);