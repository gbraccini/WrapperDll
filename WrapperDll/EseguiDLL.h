# ifndef _EseguiDLL
#define _EseguiDLL

#include "parametri.pb.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <boost\shared_ptr.hpp>


using std::vector;
using std::string;

class EseguiDLL
{

public:
	EseguiDLL( int hLib ):hLibreria((HMODULE) hLib){};
	EseguiDLL( HMODULE hLib ):hLibreria(hLib){};
	
	
	parametri::EseguiFunzione Esegui( parametri::EseguiFunzione *Exec );
private:
	struct parametro_asm
	{
		 parametri::Parametro_TipoParametro tipo;
		 void *ptr;
		 double dPara;
	};
	HMODULE hLibreria;
	
};


#endif