//
//  connect.hpp
//

#ifndef connect_h
#define connect_h

#include <ctime>
#include <iostream>
#include <string>
#include <list>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include "connect.h"

#define PORT 7000

using boost::asio::ip::tcp;
using namespace std;

class remove_tcp_connection_key;
class tcp_server;
class tcp_connection;

class tcp_connection
: public boost::enable_shared_from_this<tcp_connection>
{
   friend class tcp_server;
public:
   typedef boost::shared_ptr<tcp_connection> pointer;
   ~tcp_connection();
   tcp::socket& socket();
   
   
   /// Synchornous write a data structure to the socket.
   template <typename T>
   int write(const T& t)
   {
      // Serialize the data first so we know how large it is.
      std::ostringstream archive_stream;
      boost::archive::text_oarchive archive(archive_stream);
      archive << t;
      outbound_data_ = archive_stream.str();
      // Format the header.
      std::ostringstream header_stream;
      header_stream << std::setw(header_length)
      << std::hex << outbound_data_.size();
      if (!header_stream || header_stream.str().size() != header_length)
      {
         // Something went wrong, inform the caller.
         boost::system::error_code error(boost::asio::error::invalid_argument);
         cerr << error.message() << endl;
         return -1;
      }
      outbound_header_ = header_stream.str();
      // Write the serialized data to the socket. We use "gather-write" to send
      // both the header and the data in a single write operation.
      std::vector<boost::asio::const_buffer> buffers;
      buffers.push_back(boost::asio::buffer(outbound_header_));
      buffers.push_back(boost::asio::buffer(outbound_data_));
      boost::system::error_code ec;
      int writeLength=-1;
      try{
         writeLength=boost::asio::write(socket_,buffers,ec);
      }catch(exception e){
         cerr << e.what() << endl;
         cerr << ec.message() << endl;
         remove_connection();
         return -1;
      }
      return writeLength;
   };
   /// Synchronous read a data structure from the socket.
   template <typename T>
   int read(T& t)
   {
      int readLength=-1;
      boost::system::error_code ec;
      try{
         readLength=boost::asio::read(socket_, boost::asio::buffer(inbound_header_), ec);
      }catch(exception e){
         cerr << e.what() << endl;
         cerr << ec.message() << endl;
         remove_connection();
         return readLength;
      }
      // Determine the length of the serialized data.
      std::istringstream is(std::string(inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
      {
         // Header doesn't seem to be valid. Inform the caller.
         boost::system::error_code error(boost::asio::error::invalid_argument);
         cerr << error.message() << endl;
         return -1;
      }
      
      // Start an asynchronous call to receive the data.
      inbound_data_.resize(inbound_data_size);
      readLength=-1;
      try{
         readLength=boost::asio::read(socket_, boost::asio::buffer(inbound_data_));
      }catch(exception e){
         remove_connection();
         return readLength;
      }
      // Extract the data structure from the data just received.
      try
      {
         std::string archive_data(&inbound_data_[0], inbound_data_.size());
         std::istringstream archive_stream(archive_data);
         boost::archive::text_iarchive archive(archive_stream);
         archive >> t;
      }
      catch (std::exception& e)
      {
         // Unable to decode data.
         boost::system::error_code error(boost::asio::error::invalid_argument);
         cerr << error.message() << endl;
         return -1;
      }
      return readLength;
   };
   
private:
   tcp_connection(boost::asio::io_service& io_service,tcp_server& rtcp_server );
   void remove_connection();
   tcp::socket socket_;
   /// The size of a fixed length header.
   enum { header_length = 8 };
   /// Holds an outbound header.
   std::string outbound_header_;
   /// Holds the outbound data.
   std::string outbound_data_;
   /// Holds an inbound header.
   char inbound_header_[header_length];
   /// Holds the inbound data.
   std::vector<char> inbound_data_;
   tcp_server& m_rtcp_server;
};

class tcp_server
{
public:
   tcp_server (boost::asio::io_service& io_service,
      void(*newclienthandle)(tcp_connection::pointer)
   );
   ~tcp_server();
   void start_accept();
   list<tcp_connection::pointer>& GetConnections() {
      return m_lsConnections;
   }
   tcp_connection::pointer connect(string sAddress, string sPort);
   void remove_connection (
      tcp_connection* connection,
      remove_tcp_connection_key
   );
   
private:
   void(*newclienthandle_)(tcp_connection::pointer new_connection);
   boost::asio::io_service& io_service_;
   tcp::acceptor acceptor_;
   list<tcp_connection::pointer> m_lsConnections;
};

class remove_tcp_connection_key {
   friend class tcp_connection;
   friend class tcp_server;
   remove_tcp_connection_key(){};
};

#endif /* connect_h */
