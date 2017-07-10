

#ifndef SIMPLEHTTPSERVER_SERVER_H
#define SIMPLEHTTPSERVER_SERVER_H


#include <tiff.h>
#include <boost/asio.hpp>
#include <unordered_map>

using namespace boost::asio::ip;
using connection_handler_t = std::function<std::string(std::unordered_map<std::string,std::string>)>;

class Server {

public:
    Server(uint16 portNumber);
    void registerAction(std::string url,connection_handler_t  handler);
    void listen();
    void stop();

private:
    std::unordered_map<std::string,connection_handler_t> handlerTable;
    std::string doAction(std::string requestUrl);
    std::unordered_map<std::string, std::string> splitParams(std::string params);
    uint16 portNumber;
    tcp::socket *s;
    tcp::acceptor *acceptor;
    bool continueLooping;
};


#endif //SIMPLEHTTPSERVER_SERVER_H
