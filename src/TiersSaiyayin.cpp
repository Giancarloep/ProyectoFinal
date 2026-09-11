#include "TiersSaiyayin.h"

const std::vector<DefinicionTier>& tablaTiers() {
    static const std::vector<DefinicionTier> tiers = {
        {"Saiyajin base",    0.0,  "base"},
        {"Kaioken x2",       1.0,  "kaioken2"},
        {"Kaioken x10",      1.4,  "kaioken10"},
        {"Kaioken x20",      1.6,  "kaioken20"},
        {"Super Saiyajin",   1.8,  "supersaiyajin"},
        {"Super Saiyajin 2", 2.0,  "supersaiyajin2"},
        {"Super Saiyajin 3", 3.0,  "supersaiyajin3"},
    };
    return tiers;
}

int indiceTier(double ratio) {
    const auto& tiers = tablaTiers();
    int indice = 0;
    for (std::size_t i = 0; i < tiers.size(); ++i) {
        if (ratio >= tiers[i].umbralRatio) {
            indice = static_cast<int>(i);
        }
    }
    return indice;
}

const DefinicionTier& tierDe(double ratio) {
    return tablaTiers()[static_cast<std::size_t>(indiceTier(ratio))];
}