#include "FiniteDifferencesConsole.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Console::Parameters parameters(L"FiniteDifferencesConsole");
    auto console = TheConsoleSystem.Create<FiniteDifferencesConsole>(parameters);
    TheConsoleSystem.Execute(console);
    TheConsoleSystem.Destroy(console);
    return 0;
}
