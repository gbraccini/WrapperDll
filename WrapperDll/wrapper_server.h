#ifndef _WRAPPED_SERVER
#define _WRAPPED_SERVER

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "Connection.h"

// Database server. The constructor starts it listening on the given
// port with the given io_service.
//
class Server  
{
public:
    //Server(boost::shared_ptr< boost::asio::io_service > io_service, unsigned port);
	Server( boost::asio::io_service &io_service, unsigned port);
    ~Server();
	
void run();
private:

    Server();
    void start_accept();
	void handle_accept(Connection::Pointer connection,  const boost::system::error_code& error);
	void handle_stop();
	
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::signal_set signals;
	
};

#endif /* DB_SERVER_H */

