#include "pch.h"
#include "Checkers.h"

extern "C" __declspec(dllexport) const char* GetGameName() {
    return "Checkers";
}

extern "C" __declspec(dllexport) IGame * CreateGame() {
    return new Checkers();
}