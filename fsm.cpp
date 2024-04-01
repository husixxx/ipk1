#include "client.hpp"
#include "fsm.hpp"





/**
 * @brief Handles the start state of the FSM.
 * 
 * This function is responsible for processing the input command and transitioning to the appropriate state.
 * If the command is "/auth", it parses the input parts, checks for validity, and sends an authentication message to the client.
 * If the client is using TCP, it establishes a TCP connection before sending the authentication message.
 * If the command is not "/auth", it displays an error message and sets the client state back to "start".
 * 
 * @param client The IPK24_Client object representing the client.
 * @param inputParts The vector of input parts.
 * @param command The input command.
 */
void FSM::stateStart(IPK24_Client& client, std::vector<std::string> inputParts, std::string command){
	
	
	if(command == "/auth"){
        
        std::vector<std::string> parsedParts = parseInput(inputParts);
        if(parsedParts.empty()){
            std::cerr << "ERR: Invalid input. Authorize again" << std::endl;
            return;
        }
        std::string userName = parsedParts[0];
        client.displayName = parsedParts[2]; 
        std::string secret = parsedParts[1];

        

        if(client.sock_protocol == SOCK_STREAM){

            //client.connectTCP();
            client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
            client.sendData(client.message);
            client.receiveData();
            client.state = Client_State::auth;
            return;
        }


        // ADD MESSAGE


        client.sendAuth(parsedParts);
        client.state = Client_State::auth;
        


    }else{

        std::cerr << "ERR: You must authorize first. Try again." << std::endl;
        client.state = Client_State::start; // HANDLE THIS, REPEAT UNTIL AUTH IS SUCCESSFUL
        return;
    }

    
}


/**
 * @brief Parses the input parts and validates them.
 * 
 * This function parses the input parts and validates them using regular expressions.
 * If the input is valid, it returns a vector containing the parsed input parts.
 * If the input is invalid, it returns an empty vector.
 * 
 * @param inputParts The vector of input parts.
 * @return std::vector<std::string> The vector containing the parsed input parts.
 */
std::vector<std::string> FSM::parseInput(std::vector<std::string> inputParts) {
    std::vector<std::string> parsedInput;

    
    if (inputParts.size() < 4) {
        return parsedInput; // return empty vector, if input is invalid
    }

    std::string userName = inputParts[1];
    std::string secret = inputParts[2];
    std::string displayName = inputParts[3]; 
    

    // regex for validation
    std::regex userAndChannelRegex("^[A-Za-z0-9\\-]{1,20}$"); // Pro Username a ChannelID
    std::regex secretRegex("^[A-Za-z0-9\\-]{1,128}$"); // Pro Secret
    std::regex displayNameRegex("^[\x21-\x7E]{1,20}$"); // Pro DisplayName

    // username
    if (!std::regex_match(userName, userAndChannelRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }

    // secret
    if (!std::regex_match(secret, secretRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }

    // displayName
    if (!std::regex_match(displayName, displayNameRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }
 
    //return parsed input
    parsedInput.push_back(userName);
    parsedInput.push_back(secret);
    parsedInput.push_back(displayName);
    // Přidání MessageContent by vyžadovalo další parametr/input

    return parsedInput;
}

void FSM::stateAuthTCP(IPK24_Client& client){
    std::vector<std::string> reply = split(client.reply);
    for (auto &c : reply[0]) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    for (auto &c : reply[1]) {
        c = std::toupper(static_cast<unsigned char>(c));
    }

    if( reply[0] == "REPLY" && reply[1] == "OK"){
        // cout the reply 0 and reply 1
        
        client.state = Client_State::open;
        client.reply = "";
        return;
    }
    else if(reply[0] == "ERR"){
        client.sendData("BYE\r\n");
        client.state = Client_State::error;
        client.reply = "";
        return;
    }
    if( reply[0] == "REPLY" && reply[1] == "NOK"){

        if(!std::getline(std::cin, client.message)){
            client.state = Client_State::end; // EOF , END OF FILITO BRO

        }
        std::vector<std::string> inputParts = split(client.message);
        if(inputParts.size() != 4){
            std::cerr << "ERR: Invalid input !" << std::endl;
            
            return;
        }
        std::string command = inputParts[0];
        std::string userName = inputParts[1];
        client.displayName = inputParts[3]; 
        std::string secret = inputParts[2];


        if(userName.empty() || client.displayName.empty() || secret.empty() ){
            std::cerr << "Invalid input bro" << std::endl;
            return;
        }


        
        client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
        client.sendData(client.message);
        client.receiveData();
                    
        }
    }



void FSM::stateAuthUDP(IPK24_Client& client , std::vector<std::string> inputParts){
    

    // get the current message
    std::string response = client.recieveDataUDP();
    //extreact type and ref messageID
    uint8_t type = static_cast<uint8_t>(response[0]);
    uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
    // type is REPLY thats what we want
    if(type == 0x01){
        uint8_t result = static_cast<uint8_t>(response[3]);
        if(result == 0x01){ // REPLY OK
            
            std::vector<uint8_t> message;
            message.push_back(0x00); // confirm
            message.push_back(static_cast<uint8_t>(messageID >> 8)); 
            message.push_back(static_cast<uint8_t>(messageID & 0xFF)); 
            client.sendDataUDP(message);
            
            client.state = Client_State::open;
            return;
        }else{

            std::vector<uint8_t> messageConfirm;
            messageConfirm.push_back(0x00); // confirm
            messageConfirm.push_back(static_cast<uint8_t>(messageID >> 8));
            messageConfirm.push_back(static_cast<uint8_t>(messageID & 0xFF)); 
            client.sendDataUDP(messageConfirm);

            std::vector<uint8_t> message;
            if(!std::getline(std::cin, client.message)){
                client.state = Client_State::end; // eof, successful end
            }
            std::vector<std::string> parsedParts = parseInput(split(client.message)); // split the input and check if it is valid

            if(parsedParts.empty()){
                std::cerr << "ERR: Invalid input" << std::endl;
                return;
            }

            client.sendAuth(parsedParts);// send the auth message
            client.state = Client_State::auth;

        }
    }else if(type == 0xfe){
        std::vector<uint8_t> message;
        message.push_back(0x00); // confirm
        message.push_back(static_cast<uint8_t>(messageID >> 8)); 
        message.push_back(static_cast<uint8_t>(messageID & 0xFF));
        client.sendDataUDP(message); 
        client.state = Client_State::end; // success end
        

    }


}   
