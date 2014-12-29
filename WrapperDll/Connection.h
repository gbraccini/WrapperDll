#ifndef _Connection
#define _Connection
// La classe connection gestice una singola connessione di un client, mentre la classe server ha il compito di accettare una connessione e creare la relativa istanza di connection.

#include "stdafx.h"

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/chrono.hpp>
#include <boost/asio/signal_set.hpp>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <time.h>
#include "EseguiDLL.h"
#include "tools.h"

#include "packedmessage.h"
#include "parametri.pb.h"
 
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
class Connection : public boost::enable_shared_from_this<Connection>
{
public:
    typedef boost::shared_ptr<Connection> Pointer;
    typedef boost::shared_ptr<parametri::Richiesta>		 RequestPointer;
    typedef boost::shared_ptr<parametri::EseguiFunzione> ResponsePointer;
	 

    static Pointer create(asio::io_service& io_service ) // Create a new connection
    {
        return Pointer(new Connection(io_service ));
    }

    tcp::socket& get_socket();
    

    void start_reader_asynchronous();
    

	void start_reader_synchronous();
	private:
	bool Esci;
	asio::io_service &io_service;
    tcp::socket m_socket;
    vector<uint8_t> writebuf;
    vector<uint8_t> m_readbuf;
	

    PackedMessage<parametri::Richiesta> m_packed_request;

    Connection(asio::io_service& _io_service );


    void handle_read_header(const boost::system::error_code& error);
    void handle_read_body(const boost::system::error_code& error);
	
	void handle_request();

	void handle_write(const boost::system::error_code & error, size_t bytes);
    void start_read_header();
    
    void start_read_body(unsigned msg_len);


    ResponsePointer prepare_response(RequestPointer req);
};


#endif