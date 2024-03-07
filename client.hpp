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
#include <sstream>
#include <csignal>



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
        
        void sendDataUDP(std::vector<uint8_t> message);
        void connectTCP();
        std::string recieveDataUDP();
        void closeConnection();

        void sendDataTCP(std::string message);
        void sendData(std::string message){
            sendDataTCP(message);
            
        }
        void printUDPmessagge(std::string message);
        std::string receiveReply(uint16_t expectedMessageID);
        bool receiveConfirmation(uint16_t expectedMessageID);
        std::string recieveDataTCP();
        void printMessage(std::string message);
        void closeConnectionTCP();
        std::string reply; 
        void receiveData()
        {

            this->reply = recieveDataTCP();
            printMessage(this->reply);
            

            
        }
        std::string message;
        Client_State state;
        std::string displayName;
        int sock_fd;
        int sock_protocol;
        struct sockaddr *dyn_address;
        uint16_t messageID;
        
        
    
    private:


        
        int port;
        struct sockaddr *address;
        struct hostent* host;
        struct sockaddr_in server_address{};
        std::string ip;
    
};



#endif // clientHPP
