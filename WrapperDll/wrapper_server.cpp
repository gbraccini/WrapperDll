#include "stdafx.h"
#include "wrapper_server.h"


void Server::start_accept()
{
	// Create a new connection to handle a client. 
	Connection::Pointer new_connection = 	Connection::create(acceptor.get_io_service());

	// Asynchronously wait to accept a new client
	acceptor.async_accept(new_connection->get_socket(),
		boost::bind(&Server::handle_accept, this, new_connection,
		asio::placeholders::error));
}


void Server::handle_accept(Connection::Pointer connection,
	const boost::system::error_code& error)
{
	// A new client has connected
	if (!error)
	{
		// Start the connection
		if( FLAGS_multithread_disable )
			connection->start_reader_asynchronous();
		else
		{
			boost::shared_ptr<boost::thread> ptrThread ( new boost::thread(boost::bind(&Connection::start_reader_asynchronous, connection)) ); // run connection in new thread
		}

		// Accept another client
		start_accept();

	}
}

void Server::handle_stop()
{
	// The server is stopped by cancelling all outstanding asynchronous
	// operations. Once all operations have finished the io_service::run() call
	// will exit.
	acceptor.close();
	//acceptor.get_io_service().stop();

}


Server::Server( boost::asio::io_service &io_service, unsigned port)
	:   signals(io_service, SIGINT, SIGTERM), acceptor(io_service, tcp::endpoint(tcp::v4(), port))
{
	acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor.listen();

	signals.async_wait(boost::bind(&Server::handle_stop, this));
	start_accept();

}



Server::~Server()
{
}


