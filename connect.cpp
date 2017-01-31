//
//  connect.cpp
//

#include <algorithm>
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <sstream>
#include <vector>
#include <list>
#include <boost/exception/diagnostic_information.hpp>
#include <cctype>
#include "connect.h"

using boost::asio::ip::tcp;
using namespace std;

tcp_connection::tcp_connection(boost::asio::io_service& io_service,tcp_server& rtcp_server)
: socket_(io_service),m_rtcp_server(rtcp_server)
{
}

tcp_connection::~tcp_connection(){
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close();
}

tcp_connection::pointer tcp_server::connect(string sAddress, string sPort){
    tcp_connection::pointer new_connection;
    try{
        new_connection=tcp_connection::pointer(new tcp_connection(io_service_,*this));
        tcp::resolver resolver(new_connection->socket_.get_io_service());
        tcp::resolver::query query(sAddress, sPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        boost::asio::connect(new_connection->socket_, endpoint_iterator);
        m_lsConnections.push_back(new_connection);
    }
    catch(boost::system::system_error& e)
    {
        remove_connection(&*new_connection, remove_tcp_connection_key());
        cerr<<"Error: " << e.what()<<endl;
        cerr<<"Info: "  << boost::diagnostic_information(e) <<endl;
        new_connection.reset();
    }catch(exception e){
        remove_connection(&*new_connection, remove_tcp_connection_key());
        cerr<<"Error: " << e.what()<<endl;
        new_connection.reset();
    }
    return new_connection;
}

tcp::socket& tcp_connection::socket()
{
    return socket_;
}

void tcp_connection::remove_connection(){
    m_rtcp_server.remove_connection(this, remove_tcp_connection_key());
}

tcp_server::tcp_server(boost::asio::io_service& io_service,void(*newclienthandle)(tcp_connection::pointer))
: io_service_(io_service),acceptor_(io_service, tcp::endpoint(tcp::v4(), PORT)),newclienthandle_(newclienthandle)
{
    cout << "listening on "<<PORT<<"!"<<endl;
}

void tcp_server::start_accept()
{
    while(true){
        tcp_connection::pointer new_connection =
        tcp_connection::pointer(new tcp_connection(acceptor_.get_io_service(),*this));
        acceptor_.accept(new_connection->socket());
        m_lsConnections.push_back(new_connection);
        newclienthandle_(new_connection);
    }
}

void tcp_server::remove_connection(tcp_connection* p, remove_tcp_connection_key){
        remove(m_lsConnections.begin(), m_lsConnections.end(), p->shared_from_this());
}

tcp_server::~tcp_server(){
    m_lsConnections.clear();
}
