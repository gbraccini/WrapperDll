//============================================================================
// Name        : ClientWrapDll.cpp
// Author      : Gianluca Braccini
// Version     :
// Description : Client C++ per il test del wrapperDLL
//============================================================================

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <string>
#include "packedmessage.h"
#include "parametri.pb.h"
#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <gflags/gflags.h>

using namespace parametri;
DEFINE_int32( endpoint, 6820, " port number" );
DEFINE_string( log_directory, ".\\log\\", "log file's directory");
DEFINE_int32 ( log_level,     1         , "mininum level of log file. Set 0 for all info");
DEFINE_bool( multithread_disable, false, "disable multithread connection, only one thread for all connections" );
using namespace std;
namespace asio = boost::asio;
using asio::ip::tcp;
using boost::uint8_t;

boost::asio::io_service io_service;
boost::asio::ip::tcp::resolver resolver(io_service);
boost::asio::ip::tcp::socket sock(io_service);
boost::array<char, 4096> buffer;
typedef boost::shared_ptr<parametri::Richiesta>   RequestPointer;
typedef boost::shared_ptr<parametri::EseguiFunzione>   ResponsePointer;


void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
	 PackedMessage<parametri::Richiesta> m_packed_request;


	  if (!ec)
	  {
		 std::cout << "qualcosa è arrivato" << endl;
		 
      
	    std::cout << std::string(buffer.data(), bytes_transferred) << std::endl;

	    sock.async_read_some(boost::asio::buffer(buffer), read_handler);
	  }

}
typedef boost::shared_ptr<parametri::EseguiFunzione> ResponsePointer;

ResponsePointer send_and_wait( RequestPointer request )
{

	PackedMessage<parametri::Richiesta> req_msg(request);
	ResponsePointer ret;
	vector<uint8_t> writebuf, m_readbuf;


	req_msg.pack(writebuf);
    boost::asio::write(sock, boost::asio::buffer(writebuf));

	// Leggo la testata, per ricavare il numero di byte da leggere successivamente.
	m_readbuf.resize(HEADER_SIZE);
	boost::asio::read( sock, boost::asio::buffer( m_readbuf ) );

	// Preparo il buffer dove caricare lo stream portobuf
	PackedMessage<parametri::EseguiFunzione> m_packed_request(boost::shared_ptr<parametri::EseguiFunzione>(new parametri::EseguiFunzione()));
	unsigned int msg_len = m_packed_request.decode_header( m_readbuf );
	m_readbuf.resize(HEADER_SIZE + msg_len);
    asio::mutable_buffers_1 buf = asio::buffer(&m_readbuf[HEADER_SIZE], msg_len);
    boost::asio::read(sock, buf );

	
    //sock.async_read_some(boost::asio::buffer(buffer), read_handler);
	 if (m_packed_request.unpack(m_readbuf))
	 {
           ret = m_packed_request.get_msg(); 
		   request->Clear();
	 }
	
     return ret;

}

void connect_handler(const boost::system::error_code &ec)
{
  if (!ec)
  {

	vector<uint8_t> writebuf;
	vector<uint8_t> m_readbuf;
	RequestPointer request(new parametri::Richiesta );
	ResponsePointer ret;
	
	parametri::Parametro type, *p;
	int handle;

	request->set_tipo( parametri::Richiesta_TipoRichiesta_LOADLIBRARY);

	request->mutable_richiesta_loadlibrary()->set_nomelibreria(("test.dll"));
	request->mutable_richiesta_loadlibrary()->set_path((""));

	ret = send_and_wait( request );

	cout << ret->nome_funzione();
	handle = ret->handle();
	

	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "test_double" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_DOUBLE );
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_DOUBLE  );
	p->set_numero_double( 5.5 );
	

	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_DOUBLE  );
	p->set_numero_double( 6.25 );
	ret = send_and_wait( request );  
	cout << ret->mutable_ritorno()->numero_double();


	
	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_create_default_data_connection" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_INTERO );
 
	
	ret = send_and_wait( request );
parametri::Parametro data_connection ( *ret->mutable_ritorno());

	
	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_DCN_sql_into" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero( data_connection.intero() );
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );
	p->set_stringa( "select parola from users" );
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO_POINTER );
	p->set_intero( 0 );

	ret = send_and_wait( request );
	
	parametri::Parametro stringa ( *ret->mutable_ritorno());
	cout << stringa.stringa() << endl;

	
	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_DCN_sql_retrieve" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_INTERO );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero( data_connection.intero() );
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );
	p->set_stringa( "select * from users" );


	ret = send_and_wait( request );
	
	parametri::Parametro rowset ( *ret->mutable_ritorno());
	int iRowset( rowset.intero() );

	bool eof;

	//API_RS_get_col_name
	
	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_get_col_name" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero(iRowset);
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO );
	p->set_intero(1);


	ret = send_and_wait( request );
	
	 
	cout << "colonna: " << ret->mutable_ritorno()->stringa() << endl;


	 // Conto le colonne della select

    request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
    request->mutable_esegui_funzione()->set_handle(handle );
    request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_col_count" );
    request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_INTERO );

    p = request->mutable_esegui_funzione()->add_parametri();
    p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
    p->set_intero(iRowset);

    ret = send_and_wait( request );

    int numero_colonne = ret->mutable_ritorno()->intero();


  

    for( int i = 1; i <= numero_colonne; i++)
    {
        request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
        request->mutable_esegui_funzione()->set_handle( handle );
        request->mutable_esegui_funzione()->set_nome_funzione( "API_RS_get_col_name" );
        request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );

        p = request->mutable_esegui_funzione()->add_parametri();
        p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
        p->set_intero(iRowset);
        p = request->mutable_esegui_funzione()->add_parametri();
        p->set_tipo(parametri::Parametro_TipoParametro_INTERO );
        p->set_intero(i);


        ret = send_and_wait( request );
      

    }









	//API_RS_eof


	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_eof" );
	request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_BYTE );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero( iRowset );
	
	ret = send_and_wait( request );
	
	
	eof =  ret->mutable_ritorno()->intero();

	while( !eof )
	{
		//API_RS_get_value_1
	
		  for( int i = 1; i <= numero_colonne; i++)
        {
            request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
            request->mutable_esegui_funzione()->set_handle( handle );
            request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_get_value_1" );
            request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_CONST_CHAR_POINTER );

            p = request->mutable_esegui_funzione()->add_parametri();
            p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
            p->set_intero(iRowset);
            p = request->mutable_esegui_funzione()->add_parametri();
            p->set_tipo(parametri::Parametro_TipoParametro_INTERO );
            p->set_intero( i );

            ret = send_and_wait( request );

		cout << ret->mutable_ritorno()->stringa() << endl;
          
            //API_RS_move_next
        }


		//API_RS_move_next
		
		request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
		request->mutable_esegui_funzione()->set_handle( handle );
		request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_move_next" );
		request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_BYTE );
					
		p = request->mutable_esegui_funzione()->add_parametri();
		p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
		p->set_intero( iRowset );
	
		ret = send_and_wait( request );

		//API_RS_eof

		request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
		request->mutable_esegui_funzione()->set_handle( handle );
		request->mutable_esegui_funzione()->set_nome_funzione(  "API_RS_eof" );
		request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_BYTE );
					
		p = request->mutable_esegui_funzione()->add_parametri();
		p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
		p->set_intero( iRowset );
	
		ret = send_and_wait( request );
	
	
		eof =  ret->mutable_ritorno()->intero();

	}

	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_DCN_destroy" );
	//request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_INTERO );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero(iRowset );
	
	ret = send_and_wait( request );
	
	//API_destroy_connection
	request->set_tipo( parametri::Richiesta_TipoRichiesta_FUNCTION );
	request->mutable_esegui_funzione()->set_handle( handle );
	request->mutable_esegui_funzione()->set_nome_funzione(  "API_destroy_connection" );
	//request->mutable_esegui_funzione()->mutable_ritorno()->set_tipo(  parametri::Parametro_TipoParametro_INTERO );
					
	p = request->mutable_esegui_funzione()->add_parametri();
	p->set_tipo(parametri::Parametro_TipoParametro_INTERO  );
	p->set_intero(data_connection.intero() );	

	ret = send_and_wait( request );
		


	request->set_tipo( parametri::Richiesta_TipoRichiesta_CLOSELIBRARY);

	request->mutable_richiesta_close_library()->set_handle( handle );
 

	ret = send_and_wait( request );
  }
}

void resolve_handler(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it)
{
  if (!ec)
  {
    sock.async_connect(*it, connect_handler);
  }
}

int main()
{
  boost::asio::ip::tcp::resolver::query query("127.0.0.1", "6820");
  resolver.async_resolve(query, resolve_handler);
  io_service.run();
}
