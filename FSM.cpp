#include "FSM.hpp"

void fsm::caseStartTCP(std::vector<std::string> inputParts, std::string command){
    if(command == "/auth"){
        
        if(inputParts.size() != 4){
            std::cerr << "Invalid input" << std::endl;
            client.state = Client_State::error;
            return;
        }
        
        std::string userName = inputParts[1];
        client.displayName = inputParts[3]; 
        std::string secret = inputParts[2];
 
        if( userName.empty() || client.displayName.empty() || secret.empty() ){
            std::cerr << "Invalid input bro" << std::endl;
            client.state = Client_State::error;
            return;
        }
        // client.message = "1 " + userName + " " + client.displayName + " " + secret + "\r\n";

        if(client.sock_protocol == SOCK_STREAM){
            client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
            client.sendData(client.message);
            client.receiveData();
            client.state = Client_State::auth;
            return;
        }
    }
}


void fsm::caseStartUDP(std::vector<std::string> inputParts, std::string command){
    
    std::string userName = inputParts[1];
        client.displayName = inputParts[3]; 
        std::string secret = inputParts[2];
 
        if( userName.empty() || client.displayName.empty() || secret.empty() ){
            std::cerr << "Invalid input bro" << std::endl;
            client.state = Client_State::error;
            return;
        }
    std::vector<uint8_t> message;

        // Přidání typu zprávy
        message.push_back(0x02); // AUTH typ

        // Přidání MessageID - pro příklad použijeme 0x0001, ve skutečné implementaci by to mělo být unikátní
        
         // Předpokládáme, že messageId je 16-bitové číslo
        message.push_back(static_cast<uint8_t>(client.messageID >> 8)); // Horní bajt
        message.push_back(static_cast<uint8_t>(client.messageID & 0xFF)); // Dolní bajt
        


        // Přidání Username
        for (char c : userName) {
           message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci

        // Přidání DisplayName
        for (char c : client.displayName) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci

        // Přidání Secret
        for (char c : secret) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci
        client.sendDataUDP(message);
        client.state = Client_State::auth;

}