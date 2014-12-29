		
#include "stdafx.h"
#include <Windows.h>
#include <strsafe.h>
#include <string>
#include <stdio.h>

 

namespace tools
{
	// Funzione utilizzata per spacchettare il parametro della linea di comando in maniera da riavere un vettore di puntatori alle singole voci.
	char**  CommandLineToArgvA( char* buffer,  int* _argc  )
	{
		PCHAR* argv;
		PCHAR  _argv;
		ULONG   len;
		ULONG   argc;
		CHAR   a;
		ULONG   i, j;
		CHAR CmdLine[2048];

		strcpy( CmdLine, "WrapperDll.exe " );
		strcat( CmdLine, buffer );

		BOOLEAN  in_QM;
		BOOLEAN  in_TEXT;
		BOOLEAN  in_SPACE;

		len = strlen(CmdLine);
		i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

		argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
			i + (len+2)*sizeof(CHAR));

		_argv = (PCHAR)(((PUCHAR)argv)+i);

		argc = 0;
		argv[argc] = _argv;
		in_QM = FALSE;
		in_TEXT = FALSE;
		in_SPACE = TRUE;
		i = 0;
		j = 0;

		while( a = CmdLine[i] ) {
			if(in_QM) {
				if(a == '\"') {
					in_QM = FALSE;
				} else {
					_argv[j] = a;
					j++;
				}
			} else {
				switch(a) {
				case '\"':
					in_QM = TRUE;
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					in_SPACE = FALSE;
					break;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if(in_TEXT) {
						_argv[j] = '\0';
						j++;
					}
					in_TEXT = FALSE;
					in_SPACE = TRUE;
					break;
				default:
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					_argv[j] = a;
					j++;
					in_SPACE = FALSE;
					break;
				}
			}
			i++;
		}
		_argv[j] = '\0';
		argv[argc] = NULL;

		(*_argc) = argc;
		return argv;
	};

	// FUnzione che formatta il messaggio di sistema associato al codice di errore restituito da GetLastError.
	std::string FormattaMessaggioDiErrore( std::string sMsg ) 
	{
		LPCTSTR lpszFunction;
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		std::string ret;

		std::wstring wsMsg( sMsg.begin(), sMsg.end() ); 
		lpszFunction = sMsg.c_str();
		FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						(int) GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &lpMsgBuf,
						0, NULL 
						);
				
						lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 

						StringCchPrintf(     (LPTSTR)lpDisplayBuf, 
											LocalSize(lpDisplayBuf) / sizeof(TCHAR),
											TEXT("%s failed with error %d: %s"), 
											lpszFunction, (int) GetLastError(), lpMsgBuf); 
   
					
		ret = (LPCSTR)lpDisplayBuf;	

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);

		return ret;
	}

}

//Funzioni private
namespace
{
	/**
	 * Converte una stringa nel CodePage predefinito di Windows in una stringa multibyte
	 * e ritorna un puntatore al nuovo spazio associato per memorizzarla.
	 * Questo spazio deve essere liberato dal client
	 */
	WCHAR* stringCPToWChar(const char *orig);

	/**
	 * Converte una stringa dal formato multibyte verso una stringa UTF8 e ritorna un puntatore
	 * al nuovo spazio usato per memorizzarla.
	 * Questo spazio deve essere liberato dal client
	 */
	char* stringWCharToUtf8(WCHAR* orig);

	/**
	 * Converte una stringa dal codepage predefinito di Windows in una stringa UTF-8 e ritorna
	 * un puntatore al nuovo spazio associato per memorizzarla.
	 * Questo spazio deve essere liberato dal client.
	 */
	char *stringCPToUtf8(const char *orig);

	/**
	 * Converte una stringa dal formato UTF8 verso il formato che usa il codepage predefinito
	 * di Windows e ritorna un puntatore al nuovo spazio usato per memorizzarla.
	 * Questo spazio deve essere liberato dal client
	 */
	char *stringUtf8ToCP(const char *orig);

	/**
	 * Converte una stringa nel UTF8 in una stringa wchar e ritorna 
	 * un puntatore al nuovo spazio associato per memorizzarla.
	 * Questo spazio deve poi essere liberato dal client
	 */
	WCHAR* stringUtf8ToWChar(const char *orig);

	/**
	 * Converte una stringa dal formato multibyte verso una stringa codificata secondo
	 * il codepage di windows e ritorna un puntatore al nuovo spazio usato per memorizzarla.
	 * Questo spazio deve essere liberato dal client
	 */
	char *stringWCharToCP(WCHAR* orig);

	WCHAR* stringCPToWChar(const char *orig) {
		if(orig==NULL) {
			return NULL;
		}

		int requiredBufferSize=MultiByteToWideChar(CP_ACP, 0, orig, strlen(orig)+1, NULL, 0);
		WCHAR *spazioConversione=(WCHAR *)malloc(sizeof(WCHAR)*requiredBufferSize);

		if(spazioConversione!=NULL) {
			// Conversione dal codepage a multibyte
			MultiByteToWideChar(CP_ACP, 0, orig, strlen(orig)+1, spazioConversione, requiredBufferSize);
			return spazioConversione;
		} else {
			return NULL;
		}
	}

	char* stringWCharToUtf8(WCHAR* orig) {
		if(orig==NULL) {
			return NULL;
		}

		int requiredBufferSize=WideCharToMultiByte(CP_UTF8, 0, orig, -1, NULL, 0, NULL, NULL);
		char *spazioConversione=(char *)malloc(requiredBufferSize);
		
		if(spazioConversione!=NULL) {
			WideCharToMultiByte(CP_UTF8, 0, orig, -1, spazioConversione, requiredBufferSize, NULL, NULL);
			return spazioConversione;
		} else {
			return NULL;
		}
	}

	char *stringCPToUtf8(const char *orig) {
		if(orig==NULL) {
			return NULL;
		}

		WCHAR *multiByte=stringCPToWChar(orig);
		if(multiByte==NULL) {
			return NULL;
		}

		char *utf8String=stringWCharToUtf8(multiByte);
		free(multiByte);
		if(utf8String==NULL) {
			return NULL;
		}

		return(utf8String);
	}

	WCHAR* stringUtf8ToWChar(const char *orig) {
		if(orig==NULL) {
			return NULL;
		}

		int requiredBufferSize=MultiByteToWideChar(CP_UTF8, 0, orig, strlen(orig)+1, NULL, 0);
		WCHAR *spazioConversione=(WCHAR *)malloc(sizeof(WCHAR)*requiredBufferSize);

		if(spazioConversione==NULL) {
			return NULL;
		}

		MultiByteToWideChar(CP_UTF8, 0, orig, strlen(orig)+1, spazioConversione, requiredBufferSize);
		return spazioConversione;
	}

	char* stringWCharToCP(WCHAR* orig) {
		if(orig==NULL) {
			return NULL;
		}
		
		int requiredBufferSize=WideCharToMultiByte(CP_ACP, 0, orig, -1, NULL, 0, NULL, NULL);
		char *spazioConversione=(char *)malloc(requiredBufferSize);

		if(spazioConversione==NULL) {
			return NULL;
		}
		
		WideCharToMultiByte(CP_ACP, 0, orig, -1, spazioConversione, requiredBufferSize, NULL, NULL);
		return spazioConversione;
	}

	char *stringUtf8ToCP(const char *orig) {
		if(orig==NULL) {
			return NULL;
		}

	#ifdef UNICODE_TRACE        
	  printf("stringUtf8ToCP 1) Stringa originale UTF8: %p [%s]\n", orig, orig);
	#endif
        
		WCHAR *wString=stringUtf8ToWChar(orig);
        
	#ifdef UNICODE_TRACE        
			printf("stringUtf8ToCP 2) Stringa wchar: %p [%S]\n", wString, wString);
	#endif
		if(wString==NULL) {
			return NULL;
		}
        
		char *cpString=stringWCharToCP(wString);
	  free(wString);

	#ifdef UNICODE_TRACE        
	  printf("stringUtf8ToCP 3) Stringa locale: %p [%s]\n", cpString, cpString);
	#endif

		if(cpString==NULL) {
			return NULL;
		}
		return cpString;
	}

}


namespace tools
{
	std::string stringFromCPToUTF8( const char *str )
	{
		char *buffer = stringCPToUtf8(str);
		std::string result ( buffer );
		free(buffer);
		return result;
	}

	std::string stringFromUTF8ToCP( const char *str )
	{
		char *buffer = stringUtf8ToCP(str);
		std::string result ( buffer );
		free(buffer);
		return result;
	}
}