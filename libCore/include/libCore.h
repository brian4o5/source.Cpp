#pragma once
#pragma warning( disable : 4251 ) // mainly because we use STL base containers/classes in classes
#ifdef  LIBCORE_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DLLEXPORT __declspec(dllexport)  
#else
#ifndef DLLEXPORT
/*Enabled as "import" in the Client side for using already created dll file*/
#define DLLEXPORT __declspec(dllimport)  
#endif
#endif
#define _WINSOCKAPI_ // disable winsock so we can use winsock2
#define NOMINMAX

#include "..\targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
static HINSTANCE DLL_HINSTANCE;
