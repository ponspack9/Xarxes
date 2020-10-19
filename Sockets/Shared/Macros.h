#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")


//------------------------------------------------


// Macros to define the type of exercise
#define USE_UDP
//#define USE_TCP

// Uncomment only one of these two
#define PING_PONG_EXERCISE
//#define MESSAGE_APP

#define MSG_TO_SEND 5
#define BUFFLEN 512