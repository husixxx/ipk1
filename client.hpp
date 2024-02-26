#ifndef clientHPP
#define clientHPP
// include basic libraries
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>


enum class Client_State{
    start,
    auth,
    open,
    error,
    end
};

enum class Messages {
    AUTH,
    JOIN,
    ERR,
    BYE,
    MSG,
    REPLY,
};






class IPK24_Client{

    public:
        IPK24_Client(int port, std::string ip, int sock_protocol);
        void sendData(std::string message);
        void recieveData();
        void closeConnection();
        
    
    private:


        Client_State state;
        int sock_fd;
        int sock_protocol;
        int port;
        struct sockaddr *address;
        struct hostent* host;
        struct sockaddr_in server_address{};
        std::string ip;
    
};


#endif // clientHPP