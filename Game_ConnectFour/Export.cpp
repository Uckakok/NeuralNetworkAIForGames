#include "pch.h"
#include "ConnectFour.h"

extern "C" __declspec(dllexport) const char* GetGameName() {
    return "Connect Four";
}

extern "C" __declspec(dllexport) IGame * CreateGame() {
    return new ConnectFour();
}