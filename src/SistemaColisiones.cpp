#include "SistemaColisiones.h"

#include <cmath>

namespace {
bool esValida(const FormaColision& forma) {
    return std::isfinite(forma.x) && std::isfinite(forma.y) &&
           std::isfinite(forma.ancho) && std::isfinite(forma.alto) &&
           forma.ancho > 0.0f && forma.alto > 0.0f;
}
}

bool SistemaColisiones::hayInterseccion(const FormaColision& primera,
                                        const FormaColision& segunda) {
    if (!esValida(primera) || !esValida(segunda)) {
        return false;
    }

    return primera.x < segunda.x + segunda.ancho &&
           segunda.x < primera.x + primera.ancho &&
           primera.y < segunda.y + segunda.alto &&
           segunda.y < primera.y + primera.alto;
}

bool SistemaColisiones::estaDentroDe(const FormaColision& forma,
                                     const FormaColision& limites) {
    if (!esValida(forma) || !esValida(limites)) {
        return false;
    }

    return forma.x >= limites.x && forma.y >= limites.y &&
           forma.x + forma.ancho <= limites.x + limites.ancho &&
           forma.y + forma.alto <= limites.y + limites.alto;
}
