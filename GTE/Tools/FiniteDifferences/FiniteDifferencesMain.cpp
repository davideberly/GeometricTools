#include "FiniteDifferencesConsole.h"
#include <iostream>

int main()
{
    try
    {
        Console::Parameters parameters(L"FiniteDifferencesConsole");
        auto console = TheConsoleSystem.Create<FiniteDifferencesConsole>(parameters);
        TheConsoleSystem.Execute(console);
        TheConsoleSystem.Destroy(console);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
