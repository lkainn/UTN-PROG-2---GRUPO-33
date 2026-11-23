#pragma once

#include <array>
#include <cstdint>

struct RegistroArma
{
    int idArma;
    char nombre[30];
    int dano;
    float velocidadAtaque;
    float alcance;
    int nivelMaximo;
    std::uint8_t eliminado;
};

class ArchivoArmas
{
public:
    using Catalogo = std::array<RegistroArma, 4>;

    // Devuelve los registros en orden de ID, con la pistola en la primera posicion.
    Catalogo leerCatalogo() const;
    bool guardarCatalogo(const Catalogo& catalogo) const;
};
