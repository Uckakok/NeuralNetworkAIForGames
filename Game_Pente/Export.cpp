#include "pch.h"
#include "Pente.h"

extern "C" __declspec(dllexport) const char* GetGameName() {
    return "Pente";
}

extern "C" __declspec(dllexport) IGame * CreateGame() {
    return new Pente();
}