#pragma once

class ArchivoPartidaEnCurso;

#include <array>
#include <cstddef>

enum class RarezaMejora
{
    Comun,
    Rara,
    Epica,
    Legendaria
};

struct OfertaArma
{
    int idArma = 0;
    RarezaMejora rareza = RarezaMejora::Comun;
    bool especial = false;
    int obtenerPorcentajeDano() const;
    int obtenerPorcentajeCadencia() const;
    bool esValida() const;
    bool transferirEstado(ArchivoPartidaEnCurso& archivo);
};

struct BonificacionesArma
{
    int porcentajeDano = 0;
    int porcentajeCadencia = 0;
    int especiales = 0;
    bool transferirEstado(ArchivoPartidaEnCurso& archivo);
};

class InventarioArmas
{
public:
    bool transferirEstado(ArchivoPartidaEnCurso& archivo);
    InventarioArmas();
    explicit InventarioArmas(const std::array<bool, 4>& disponibles);
    void reiniciar();
    int obtenerIdSlot(std::size_t slot) const;
    bool poseeArma(int idArma) const;
    bool puedeRecibirArma(int idArma) const;
    bool reemplazaPistola(int idArma) const;
    bool puedeMejorarEspecial(int idArma) const;
    bool puedeAplicarOferta(const OfertaArma& oferta) const;
    bool aplicarOferta(const OfertaArma& oferta, bool confirmarReemplazo = false);
    BonificacionesArma obtenerBonificaciones(int idArma) const;
    static const char* obtenerNombreRareza(RarezaMejora rareza);

private:
    std::array<int, 2> _slots;
    std::array<BonificacionesArma, 4> _bonificaciones;
    std::array<bool, 4> _disponibles;
};
