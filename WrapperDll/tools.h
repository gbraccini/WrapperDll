#ifndef TOOLS_WRAPPERDLL
#define TOOLS_WRAPPERDLL

namespace tools
{
	std::string FormattaMessaggioDiErrore( std::string sMsg );
	
	std::string stringFromCPToUTF8( const char *str );
	
	std::string stringFromUTF8ToCP( const char *str );

	char**  CommandLineToArgvA( char* buffer,  int* _argc );
};
#endif