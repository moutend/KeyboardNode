#include <cpplogger/cpplogger.h>
#include <windows.h>

Logger::Logger *Log{nullptr};
HANDLE requestEvent{nullptr};
