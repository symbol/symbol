#pragma once

#if defined(WIN32) || defined(WIN64)
#ifdef DLL_EXPORTS
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif
#else
#define PLUGIN_API __attribute__ ((visibility ("default")))
#endif
