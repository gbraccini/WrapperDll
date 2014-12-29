
// La classe connection gestice una singola connessione di un client, mentre la classe server ha il compito di accettare una connessione e creare la relativa istanza di connection.

#include "stdafx.h"
#include "Connection.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace std;
namespace asio = boost::asio;
using asio::ip::tcp;
using boost::uint8_t;



DECLARE_bool( multithread_disable );

// connection - handles a connection with a single client.
// Create only through the Connection::create factory.
//
tcp::socket& Connection::get_socket()
{
	return m_socket;
}

void Connection::start_reader_asynchronous()
{
	start_read_header();
	if(!FLAGS_multithread_disable)
	{
		while( !Esci )	 
		{
			io_service.poll();
			boost::this_thread::sleep(boost::posix_time::milliseconds(15));
		}

	}
	return;
}

void Connection::start_reader_synchronous()
{
	unsigned msg_len;

	while(!Esci)
	{
		try
		{
			m_readbuf.resize(HEADER_SIZE);
			if( asio::read(m_socket, asio::buffer(m_readbuf, HEADER_SIZE), asio::transfer_at_least(HEADER_SIZE)) != HEADER_SIZE )
				throw; 

			msg_len = m_packed_request.decode_header(m_readbuf);
			m_readbuf.resize(HEADER_SIZE + msg_len);

			if( asio::read(m_socket, asio::buffer(&m_readbuf[HEADER_SIZE], msg_len), asio::transfer_at_least(msg_len)) != msg_len )
				throw; 

			handle_request();
		}
		catch(...)
		{
			// Initiate graceful connection closure.
			//boost::system::error_code ignored_ec;
			//m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			Esci = true;
			
		}
	}

}

Connection::Connection(asio::io_service& _io_service )
	: Esci(false), m_socket(_io_service), io_service( _io_service ),
	m_packed_request(boost::shared_ptr<parametri::Richiesta>(new parametri::Richiesta()))
{

}



void Connection::handle_read_header(const boost::system::error_code& error)
{
	if (!error) 
	{
		unsigned msg_len = m_packed_request.decode_header(m_readbuf);

		start_read_body(msg_len);
	}
	else
	{
		LOG(INFO) << "Errore " << error.value() << " " << error.message();

		Esci = true;
		

	}
}

void Connection::handle_read_body(const boost::system::error_code& error)
{
	if (!error) 
	{
		handle_request();
		start_read_header();
	}
	else
	{
		LOG(INFO) << "Errore " << error.value() << " " << error.message();

		Esci = true;
		
	}

}


// Called when enough data was read into m_readbuf for a complete request
// message. 
// Parse the request, execute it and send back a response.
//
void Connection::handle_request()
{

	if (m_packed_request.unpack(m_readbuf))
	{
		RequestPointer req   = m_packed_request.get_msg();
		ResponsePointer resp = 	prepare_response(req);

		PackedMessage<parametri::EseguiFunzione> resp_msg(resp);

		resp_msg.pack(writebuf);

		try 
		{ 
			//if(	FLAGS_multithread_disable  )
			async_write(this->get_socket(), asio::buffer(writebuf, writebuf.size()), boost::bind( 
				&Connection::handle_write, 
				shared_from_this(), 
				boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred)); 
			//else
			//	 asio::write( this->get_socket(), asio::buffer(writebuf, writebuf.size()) ); 
		}
		catch (exception& e)
		{ 
			LOG(ERROR)  << e.what() << endl; 
			// Initiate graceful connection closure.
			//boost::system::error_code ignored_ec;
			//m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			Esci = true;
			
		} 
	}
	else
	{
		LOG(ERROR) << "Attenzione impossibile acquisire il pacchetto dati dal canale socket, probabile errore di compatibilità protobuf";
		// Initiate graceful connection closure.
		//boost::system::error_code ignored_ec;
		//m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		Esci = true;
		
	}

	return; 
} 



void Connection::handle_write(const boost::system::error_code & error, size_t bytes)
{ 
	if( error )
	{
		// Initiate graceful connection closure.
		//boost::system::error_code ignored_ec;
		//m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		Esci = true;
		
	}
	return; 
} 


void Connection::start_read_header()
{
	m_readbuf.resize(HEADER_SIZE);
	asio::async_read(this->get_socket(), asio::buffer(m_readbuf),
		boost::bind(&Connection::handle_read_header, shared_from_this(),
		asio::placeholders::error));
}

void Connection::start_read_body(unsigned msg_len)
{
	// m_readbuf already contains the header in its first HEADER_SIZE
	// bytes. Expand it to fit in the body as well, and start async
	// read into the body.
	//
	m_readbuf.resize(HEADER_SIZE + msg_len);
	asio::mutable_buffers_1 buf = asio::buffer(&m_readbuf[HEADER_SIZE], msg_len);
	asio::async_read(this->get_socket(), buf,
		boost::bind(&Connection::handle_read_body,  shared_from_this(),
		asio::placeholders::error));
}



Connection::ResponsePointer Connection::prepare_response(RequestPointer req)
{  
	string value, library;
	int handle;
	ResponsePointer resp(new parametri::EseguiFunzione );


	switch (req->tipo())
	{
	case parametri::Richiesta_TipoRichiesta_LOADLIBRARY: 
		{ 
			library = req->mutable_richiesta_loadlibrary()->path();
			library += req->mutable_richiesta_loadlibrary()->nomelibreria();
			std::wstring wLibrary( library.begin(), library.end() ); 

			LOG(INFO) << "LoadLibrary: " << library;

			handle = (int) LoadLibrary(library.c_str());
			if( handle != 0 )
			{
				resp->set_handle( handle  );
				resp->set_nome_funzione( library );
			}
			else
			{
				LOG(ERROR) << "LoadLibrary: " << tools::FormattaMessaggioDiErrore(library);
				resp->set_handle( (int) GetLastError() );
				resp->set_nome_funzione( "Errore" );
			}
			break; 
		}
	case parametri::Richiesta_TipoRichiesta_CLOSELIBRARY:
		{
			HMODULE hHandle = 0;

			if( req->mutable_richiesta_close_library()->handle() == -1 )
			{
				LOG(INFO) << "Arrivato segnale di arresto del server. " << hHandle;
				raise(SIGINT);
				resp->set_handle( 0 );
				resp->set_nome_funzione( "" );
			}
			else
			{
				hHandle = (HMODULE) req->mutable_richiesta_close_library()->handle(); 
				LOG(INFO) << "FreeLibrary: " << hHandle;

				if( !FreeLibrary( hHandle ))
				{
					LOG(ERROR) << "FreeLibrary: " << tools::FormattaMessaggioDiErrore("FreeLibrary");
					resp->set_handle( (int) GetLastError() );
					resp->set_nome_funzione( "Errore" );
				}
				else
				{
					resp->set_handle( 0 );
					resp->set_nome_funzione( "" );
				}
			}			
			break;
		}
	case parametri::Richiesta_TipoRichiesta_FUNCTION:
		{	
			char nome_funzione[512];
			try
			{

				handle = req->mutable_esegui_funzione()->handle();

				strcpy( nome_funzione, req->mutable_esegui_funzione()->nome_funzione().c_str() );


				LOG(INFO) << "Esegui: " << nome_funzione;

				EseguiDLL Ex(handle);
				resp->CopyFrom( Ex.Esegui( req->mutable_esegui_funzione()) );
			}
			catch( exception &e )
			{
				LOG(ERROR) << "Errore mentre veniva eseguita: " << nome_funzione << " errore: " << e.what();
				resp->set_handle( (int) GetLastError() );
				resp->set_nome_funzione( "Errore" );
			}

			break;
		}
	default:
		LOG(ERROR) << "Richiesta errata";
		break;
	}

	return resp;

}

