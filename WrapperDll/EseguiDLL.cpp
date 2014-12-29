#include "stdafx.h"
#include <iostream>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "EseguiDLL.h"
#include "tools.h"

using namespace std;


parametri::EseguiFunzione EseguiDLL::Esegui( parametri::EseguiFunzione *Exec )
{
	vector<parametro_asm> parametri_ingresso, parametri_uscita;
	typedef int (__stdcall *FUNCTION)(void);
	parametri::EseguiFunzione ritorno;
	parametro_asm par;
	parametri::Parametro      *parametro;
	DWORD dwDWord_ritorno;
	bool bDouble(false);
	double dPara;
	
	// Prende l'indirizzo della funzione da eseguire
	FUNCTION  proc = (FUNCTION) GetProcAddress(this->hLibreria, (tools::stringFromUTF8ToCP(Exec->nome_funzione().c_str()).c_str() ) );
	if(!proc)
	{
		ritorno.set_nome_funzione( "Errore!" );
		ritorno.mutable_ritorno()->set_intero( GetLastError() );
		return ritorno;
	}

	// Devo caricare lo stack in senso inverso rispetto all'ordine di chiamata
	int size = Exec->parametri().size();
	LOG(INFO) << "Parametri n°:" << size << endl;
	for (int i = size-1; i >= 0  ; --i )
	{
		switch(  Exec->parametri(i).tipo() )
		{
		
		case parametri::Parametro_TipoParametro_CONST_CHAR_POINTER:
			{
				par.tipo = parametri::Parametro_TipoParametro_CONST_CHAR_POINTER;
				std::string str( tools::stringFromUTF8ToCP(Exec->parametri(i).stringa().c_str()) );
			 	par.ptr  = (void *) new char[str.size() + 1];
				parametri_uscita.push_back( par ); // Non è un parametro di uscita ma inserendolo nella lista poi verrà disallocato.
				strcpy( (char *) par.ptr,str.c_str() );
				parametri_ingresso.push_back( par );
				
				LOG(INFO) << str << endl;
			}
			break;
		case parametri::Parametro_TipoParametro_CHAR_POINTER:
			{	
				par.tipo = parametri::Parametro_TipoParametro_CHAR_POINTER;
				std::string str( tools::stringFromUTF8ToCP(Exec->parametri(i).stringa().c_str()) );
				par.ptr  = (void *) new char[str.size() + 1];
				parametri_uscita.push_back( par );
				strcpy( (char *) par.ptr,str.c_str() );
				parametri_ingresso.push_back( par );
			
				LOG(INFO) << str << endl;
			}
			break;
		case parametri::Parametro_TipoParametro_INTERO:
			{
				par.tipo = parametri::Parametro_TipoParametro_INTERO_POINTER;
				par.ptr  = (void *) Exec->parametri(i).intero();
				parametri_ingresso.push_back( par );
				LOG(INFO) << par.ptr << endl;
			}
			break;
		case parametri::Parametro_TipoParametro_DOUBLE:
			{
				par.tipo = parametri::Parametro_TipoParametro_DOUBLE;
				par.dPara = (double) Exec->parametri(i).numero_double();
				parametri_ingresso.push_back( par );	
				LOG(INFO) << par.dPara << endl;
		    }
			break;
		case parametri::Parametro_TipoParametro_INTERO_POINTER:
			{
				par.tipo = parametri::Parametro_TipoParametro_INTERO_POINTER;
				par.ptr  = (void *) new int;
				*( (int*)(par.ptr)) = Exec->parametri(i).intero();
				parametri_ingresso.push_back( par );
				parametri_uscita.push_back( par );
				
				LOG(INFO) << (DWORD) par.ptr << endl;
			}
			break;
		}

	
		
	}

	

	// Devo preparare un oggetto di tipo Parametro per restituire i valori passati come puntatori.
	vector<parametro_asm>::iterator Iter;
	for( Iter = parametri_ingresso.begin(); Iter != parametri_ingresso.end(); Iter++ )
	{
	
		if( Iter->tipo == parametri::Parametro_TipoParametro_DOUBLE )
		{
			double dPara;

			dPara = Iter->dPara;
			_asm
			{
				 FLD dPara                  // Carico il numero floating point
				 SUB ESP,8					// faccio spazio nello stack per il double
				 FSTP QWORD PTR [ESP]  
			}
		}
		else
		{
			DWORD dwDWord;

			dwDWord = (DWORD) Iter->ptr;
			_asm
			{
				mov eax, dwDWord
				push eax
			}
		}

	}
	dwDWord_ritorno = 0;
	// Viene richiamata la funzione e letto il parametro di ritorno
	if( Exec->has_ritorno() )
	{
		if( Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_DOUBLE )
		{   // Se il parametro di ritorno è un floating point allore...
			_asm
			{
				call proc
				fstp dPara
			}
		}
		else
		{
			_asm
			{
				call proc
				mov dwDWord_ritorno, eax
			}
		}
	}
	else
	{
		_asm
			{
				call proc
			}
	}

	

	ritorno.set_nome_funzione( Exec->nome_funzione() );

	if( parametri_uscita.rbegin()!= parametri_uscita.rend() )
		LOG(INFO) << "Parametri di uscita:" << endl;

	// Devo preparare un oggetto di tipo Parametro per restituire i valori passati come puntatori.
	vector<parametro_asm>::reverse_iterator rIter;
	for( rIter = parametri_uscita.rbegin(); rIter != parametri_uscita.rend(); rIter++ )
	{
		if(  rIter->tipo == parametri::Parametro_TipoParametro_CHAR_POINTER ||  rIter->tipo == parametri::Parametro_TipoParametro_INTERO_POINTER ||  rIter->tipo == parametri::Parametro_TipoParametro_BYTE_POINTER )
		{
			parametro = ritorno.add_parametri();
			parametro->set_tipo( rIter->tipo );
		}
		switch( rIter->tipo )
		{
		case parametri::Parametro_TipoParametro_CHAR_POINTER:
			{
				parametro->set_stringa (tools::stringFromCPToUTF8( (const char *) rIter->ptr ) );
				LOG(INFO) << (const char *) rIter->ptr << endl;
			}
			break;
		case parametri::Parametro_TipoParametro_INTERO_POINTER:
		case parametri::Parametro_TipoParametro_BYTE_POINTER:
			parametro->set_intero( *((int *) rIter->ptr) );
			LOG(INFO) << *((int *) rIter->ptr ) << endl;
			break;
		}
		 
		delete rIter->ptr; // rilascio la memoria allocata prima della chiamata.
	}
	
	ritorno.set_handle(0); // Non c'è stato errore	

	if( Exec->has_ritorno() )
	{
		 
		ritorno.mutable_ritorno()->set_tipo( Exec->ritorno().tipo() );
		if( Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_INTERO  || Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_BYTE )
		{
			ritorno.mutable_ritorno()->set_intero( dwDWord_ritorno );
			LOG(INFO) << "parametro di ritorno: " << dwDWord_ritorno << endl;
		}
		else if( Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_DOUBLE )
		{
			ritorno.mutable_ritorno()->set_numero_double( dPara );
			LOG(INFO) << "parametro di ritorno: " << dPara << endl;
		}
		else if( Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_CONST_CHAR_POINTER || Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_CHAR_POINTER )
		{
			ritorno.mutable_ritorno()->set_stringa( tools::stringFromCPToUTF8( (const char *) dwDWord_ritorno ) );
			LOG(INFO) << "parametro di ritorno: " << (const char *) dwDWord_ritorno << endl;
		}
		else if( Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_INTERO_POINTER || Exec->ritorno().tipo() == parametri::Parametro_TipoParametro_BYTE_POINTER )
		{
			ritorno.mutable_ritorno()->set_intero( *((int*)dwDWord_ritorno) );
			LOG(INFO) << "parametro di ritorno: " <<  *((int*)dwDWord_ritorno)  << endl;
		}
	}
	parametri_uscita.clear();
	parametri_ingresso.clear();
	// google::FlushLogFiles(0); Inutile dato che nel main ho settato FLAGS_logbuflevel a -1 che dovrebbe disabilitare i log...
	return ritorno;
}   

