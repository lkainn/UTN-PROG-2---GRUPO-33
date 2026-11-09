#include "Juego.h"

#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        Juego juego;
        juego.ejecutar();
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}

