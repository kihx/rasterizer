// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KIHX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KIHX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KIHX_EXPORTS
#define KIHX_API __declspec(dllexport)
#else
#define KIHX_API __declspec(dllimport)
#endif

// This class is exported from the kihx.dll
class KIHX_API Ckihx {
public:
	Ckihx(void);
	// TODO: add your methods here.
};

extern KIHX_API int nkihx;

KIHX_API int fnkihx(void);
