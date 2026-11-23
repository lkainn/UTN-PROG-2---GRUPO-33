#pragma once

#include "TiposCompartidos.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <locale>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// La misma secuencia de campos se usa al guardar y al cargar. La carga
// trabaja sobre una copia del estado y solo la aplica al validar el archivo.
class ArchivoPartidaEnCurso
{
public:
    ArchivoPartidaEnCurso(FILE* archivo, bool lectura)
        : _archivo(archivo), _lectura(lectura), _control(2166136261u)
    {
    }

    bool leyendo() const
    {
        return _lectura;
    }

    template <typename... Campos>
    bool operator()(Campos&... campos)
    {
        return (transferir(campos) && ...);
    }

    bool bytes(void* datos, std::size_t tamanio)
    {
        const std::size_t procesados = _lectura ? fread(datos, 1, tamanio, _archivo) :
            fwrite(datos, 1, tamanio, _archivo);
        if (procesados != tamanio)
        {
            return false;
        }
        const auto* octetos = static_cast<const unsigned char*>(datos);
        for (std::size_t i = 0; i < tamanio; ++i)
        {
            _control = (_control ^ octetos[i]) * 16777619u;
        }
        return true;
    }

    bool finalizar()
    {
        if (!_lectura)
        {
            return fwrite(&_control, sizeof(_control), 1, _archivo) == 1;
        }
        std::uint32_t esperado = 0;
        return fread(&esperado, sizeof(esperado), 1, _archivo) == 1 &&
            esperado == _control && fgetc(_archivo) == EOF && !ferror(_archivo);
    }

    template <typename T, typename Fabrica>
    bool lista(std::vector<T>& elementos, std::uint32_t maximo, Fabrica fabricar)
    {
        std::uint32_t cantidad = 0;
        if (!transferirCantidad(elementos.size(), maximo, cantidad))
        {
            return false;
        }
        if (_lectura)
        {
            elementos.clear();
            elementos.reserve(cantidad);
            for (std::uint32_t i = 0; i < cantidad; ++i)
            {
                elementos.push_back(fabricar());
            }
        }
        for (T& elemento : elementos)
        {
            if (!transferir(elemento))
            {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool lista(std::vector<T>& elementos, std::uint32_t maximo)
    {
        return lista(elementos, maximo, [] { return T{}; });
    }

    template <typename T>
    bool zonas(std::map<std::pair<int, int>, T>& elementos, std::uint32_t maximo)
    {
        std::uint32_t cantidad = 0;
        if (!transferirCantidad(elementos.size(), maximo, cantidad))
        {
            return false;
        }
        if (_lectura)
        {
            elementos.clear();
            for (std::uint32_t i = 0; i < cantidad; ++i)
            {
                int x = 0;
                int y = 0;
                T zona{};
                if (!(*this)(x, y, zona) || !elementos.emplace(std::make_pair(x, y), std::move(zona)).second)
                {
                    return false;
                }
            }
            return true;
        }
        for (auto& entrada : elementos)
        {
            int x = entrada.first.first;
            int y = entrada.first.second;
            if (!(*this)(x, y, entrada.second))
            {
                return false;
            }
        }
        return true;
    }

private:
    FILE* _archivo;
    bool _lectura;
    std::uint32_t _control;

    bool transferirCantidad(std::size_t tamanio, std::uint32_t maximo, std::uint32_t& cantidad)
    {
        if (!_lectura && tamanio > maximo)
        {
            return false;
        }
        cantidad = _lectura ? 0 : static_cast<std::uint32_t>(tamanio);
        return transferir(cantidad) && cantidad <= maximo;
    }

    template <typename T>
    bool transferir(T& valor)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            std::uint8_t byte = valor ? 1 : 0;
            if (!transferir(byte) || byte > 1)
            {
                return false;
            }
            valor = byte != 0;
            return true;
        }
        else if constexpr (std::is_enum_v<T>)
        {
            std::underlying_type_t<T> numero = static_cast<std::underlying_type_t<T>>(valor);
            if (!transferir(numero))
            {
                return false;
            }
            valor = static_cast<T>(numero);
            return true;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return bytes(&valor, sizeof(valor)) && std::isfinite(valor);
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return bytes(&valor, sizeof(valor));
        }
        else
        {
            return valor.transferirEstado(*this);
        }
    }

    template <typename T, std::size_t N>
    bool transferir(std::array<T, N>& valores)
    {
        for (T& valor : valores)
        {
            if (!transferir(valor))
            {
                return false;
            }
        }
        return true;
    }

    bool transferir(Posicion& posicion)
    {
        return (*this)(posicion.x, posicion.y);
    }

    bool transferir(std::mt19937& generador)
    {
        std::stringstream texto;
        texto.imbue(std::locale::classic());
        if (!_lectura)
        {
            texto << generador;
        }
        std::string estado = texto.str();
        std::uint32_t cantidad = 0;
        if (!transferirCantidad(estado.size(), 8192, cantidad))
        {
            return false;
        }
        estado.resize(cantidad);
        if (!bytes(estado.data(), estado.size()))
        {
            return false;
        }
        if (_lectura)
        {
            texto.str(estado);
            return static_cast<bool>(texto >> generador) && texto.peek() == EOF;
        }
        return true;
    }
};
