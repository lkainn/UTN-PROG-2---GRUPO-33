#include "InventarioArmas.h"
#include "ArchivoPartidaEnCurso.h"

#include <algorithm>
#include <limits>

namespace
{
int sumarPorcentajes(int anterior, int aumento)
{
    return static_cast<int>(std::min(static_cast<long long>(anterior) + aumento,
        static_cast<long long>(std::numeric_limits<int>::max())));
}
}

bool OfertaArma::esValida() const
{
    const int indiceRareza = static_cast<int>(rareza);
    return idArma >= 1 && idArma <= 4 && indiceRareza >= 0 && indiceRareza <= 3 &&
        !(rareza == RarezaMejora::Comun && especial) &&
        !(rareza == RarezaMejora::Legendaria && !especial);
}

int OfertaArma::obtenerPorcentajeDano() const
{
    constexpr std::array<int, 4> PORCENTAJES{{5, 10, 18, 25}};
    return esValida() ? PORCENTAJES[static_cast<std::size_t>(rareza)] : 0;
}

int OfertaArma::obtenerPorcentajeCadencia() const
{
    constexpr std::array<int, 4> PORCENTAJES{{4, 8, 12, 18}};
    if (!esValida() || (especial && rareza != RarezaMejora::Legendaria))
    {
        return 0;
    }
    return PORCENTAJES[static_cast<std::size_t>(rareza)];
}

InventarioArmas::InventarioArmas()
    : InventarioArmas(std::array<bool, 4>{{true, true, true, true}})
{
}

InventarioArmas::InventarioArmas(const std::array<bool, 4>& disponibles)
    : _slots{{1, 0}}, _bonificaciones{}, _disponibles(disponibles)
{
    _disponibles[0] = true;
}

void InventarioArmas::reiniciar()
{
    _slots = {{1, 0}};
    _bonificaciones = {};
}

int InventarioArmas::obtenerIdSlot(std::size_t slot) const
{
    return slot < _slots.size() ? _slots[slot] : 0;
}

bool InventarioArmas::poseeArma(int idArma) const
{
    return idArma >= 1 && idArma <= 4 &&
        std::find(_slots.begin(), _slots.end(), idArma) != _slots.end();
}

bool InventarioArmas::puedeRecibirArma(int idArma) const
{
    if (idArma < 1 || idArma > 4 || !_disponibles[idArma - 1])
    {
        return false;
    }
    if (poseeArma(idArma))
    {
        return true;
    }
    return idArma != 1 && (_slots[1] == 0 || _slots[0] == 1);
}

bool InventarioArmas::reemplazaPistola(int idArma) const
{
    return puedeRecibirArma(idArma) && !poseeArma(idArma) &&
        _slots[1] != 0 && _slots[0] == 1;
}

bool InventarioArmas::puedeMejorarEspecial(int idArma) const
{
    if (!puedeRecibirArma(idArma))
    {
        return false;
    }
    return _bonificaciones[idArma - 1].especiales < (idArma == 4 ? 3 : 4);
}

bool InventarioArmas::puedeAplicarOferta(const OfertaArma& oferta) const
{
    return oferta.esValida() && puedeRecibirArma(oferta.idArma) &&
        (!oferta.especial || puedeMejorarEspecial(oferta.idArma));
}

bool InventarioArmas::aplicarOferta(const OfertaArma& oferta, bool confirmarReemplazo)
{
    if (!puedeAplicarOferta(oferta) ||
        (reemplazaPistola(oferta.idArma) && !confirmarReemplazo))
    {
        return false;
    }
    if (!poseeArma(oferta.idArma))
    {
        if (_slots[1] == 0)
        {
            _slots[1] = oferta.idArma;
        }
        else
        {
            _slots[0] = oferta.idArma;
            _bonificaciones[0] = {};
        }
    }
    BonificacionesArma& bonos = _bonificaciones[oferta.idArma - 1];
    bonos.porcentajeDano = sumarPorcentajes(bonos.porcentajeDano, oferta.obtenerPorcentajeDano());
    bonos.porcentajeCadencia = sumarPorcentajes(bonos.porcentajeCadencia, oferta.obtenerPorcentajeCadencia());
    if (oferta.especial)
    {
        ++bonos.especiales;
    }
    return true;
}

BonificacionesArma InventarioArmas::obtenerBonificaciones(int idArma) const
{
    return idArma >= 1 && idArma <= 4 ? _bonificaciones[idArma - 1] : BonificacionesArma{};
}

const char* InventarioArmas::obtenerNombreRareza(RarezaMejora rareza)
{
    constexpr std::array<const char*, 4> NOMBRES{{"Comun", "Rara", "Epica", "Legendaria"}};
    const int indice = static_cast<int>(rareza);
    return indice >= 0 && indice <= 3 ? NOMBRES[indice] : "Invalida";
}

bool OfertaArma::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    return archivo(idArma, rareza, especial) &&
        ((idArma == 0 && rareza == RarezaMejora::Comun && !especial) || esValida());
}

bool BonificacionesArma::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    return archivo(porcentajeDano, porcentajeCadencia, especiales) &&
        porcentajeDano >= 0 && porcentajeCadencia >= 0 && especiales >= 0 && especiales <= 4;
}

bool InventarioArmas::transferirEstado(ArchivoPartidaEnCurso& archivo)
{
    if (!archivo(_slots, _bonificaciones))
    {
        return false;
    }
    const int primero = _slots[0];
    const int segundo = _slots[1];
    return primero >= 1 && primero <= 4 && _disponibles[primero - 1] &&
        segundo >= 0 && segundo <= 4 && segundo != 1 && primero != segundo &&
        (segundo == 0 ? primero == 1 : _disponibles[segundo - 1]);
}
