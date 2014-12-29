/*
Gianluca Braccini

L'applicativo è un "demone" che si mette in ascolto su di una porta e attende che sul socket arrivino dei pacchetti contenenti informazioni memorizzate con l'ausilio della libreria PROTOBUF di google. Questo pacchetto conterrà
le indicazioni sulla libreria DLL da aprire o sulla funzione associata a una DLL aperta in precedenza, da eseguire. In questo ultimo caso il pacchetto conterrà i parametri della funzione da eseguire e restituirà ( sempre col medesimo 
sistema ) i parametri passati per riferimento e il parametro di ritorno.

Il WrapperDll è quindi un programma per eseguire delle funzioni presenti all'interno di una DLL o su un altro processo o su di un'altra macchina. Potrà essere chiamato da client scritti sia in Java che in C/C++, più in generale
da un qualsiasi linguaggio di programmazione che usi le socket e sia compatibile con le protobuf.

Librerie utilizzate:

boost	 release 1.49
protobuf release 2.4.1
glog	 release 0.3.2
gflags   release 2.0

tutte le librerie sono state linkate staticamente.


*/

#include "stdafx.h"
#include <boost/asio.hpp> 
#define GLOG_NO_ABBREVIATED_SEVERITIES

#include "wrapper_server.h"
#include "packedmessage.h"
#include "parametri.pb.h"
#include "EseguiDLL.h"
#include <iostream>
#include <iterator>
#include <algorithm>



#include <glog/logging.h>
#include <gflags/gflags.h>
#include <boost/shared_ptr.hpp>
#include <Windows.h>
#include "tools.h"

using namespace parametri;
DEFINE_int32( endpoint, 6820, " port number" );
DEFINE_string( log_directory, ".\\log\\", "log file's directory");
DEFINE_int32 ( log_level,     1         , "mininum level of log file. Set 0 for all info");
DEFINE_bool( multithread_disable, false, "disable multithread connection, only one thread for all connections" );


using namespace std;
namespace asio = boost::asio;


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	PCHAR* argv;
	int argc;
    boost::system::error_code ec;

	argv = tools::CommandLineToArgvA( lpCmdLine, &argc );
    
	google::SetUsageMessage( "[endpoint] [endpoint] [-log_directory=<path>] [-log_level] [-multithread_disable] [-help]\n\rEnginered by Gianluca Braccini release 1.0 2012" );
	google::SetVersionString( "Engineered by Gianluca Braccini release 1.0 2012" );
	google::ParseCommandLineFlags(&argc, &argv, false);
	google::InitGoogleLogging("WrapperDll");
	
	FLAGS_log_dir      = FLAGS_log_directory;
	FLAGS_minloglevel  = FLAGS_log_level; // Livello minimo di log, con zero gestisce anche INFO
	FLAGS_logbuflevel  = -1;
 
	
	try
	{
      boost::asio::io_service io_service;
		  
	  boost::asio::ip::tcp::socket sock(io_service);
	  tcp::endpoint end_point(boost::asio::ip::address::from_string("127.0.0.1"), FLAGS_endpoint);
	  boost::system::error_code err;

	  sock.connect( end_point, err );
	  if(!err)
	  {
		  LOG(ERROR) << "Impossibile avviare il Server sulla porta " << FLAGS_endpoint  <<  " in quanto la porta è già impegnata" << endl;
		  sock.close();
		  google::FlushLogFiles(0);
		  google::ShutdownGoogleLogging();
		  return 0;
	  }
	  sock.close();

        Server server(io_service, FLAGS_endpoint);
		LOG(INFO) << "Server avviato sulla porta " << FLAGS_endpoint << endl;
        io_service.run( ec );
        if( ec )
        {
            LOG(ERROR) << ec << std::endl;
        }

		 
      
		
    }
    catch (std::exception& e)
	{
		 cerr << e.what() << endl;
		 LOG(ERROR) << e.what();
    }
	google::FlushLogFiles(0);
	google::ShutdownGoogleLogging();
    return 0;
}


