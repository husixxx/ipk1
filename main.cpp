#include "client.cpp"
#include "main.hpp"
IPK24_Client* globalClientPtr = nullptr;

void signalHandler(int signum) {
    
    if (globalClientPtr != nullptr) {
        // Odeslat zprávu před ukončením
        if(globalClientPtr->sock_protocol == SOCK_STREAM){
            //aaa
            globalClientPtr->sendDataTCP("BYE\r\n");
            // globalClientPtr->receiveData();
        }else{
                
            std::vector<uint8_t> message;
            message.push_back(0xFF); // Předpokládáme, že 0xFF je kód pro ukončení
            message.push_back(static_cast<uint8_t>(globalClientPtr->messageID >> 8));
            message.push_back(static_cast<uint8_t>(globalClientPtr->messageID & 0xFF));
            globalClientPtr->sendDataUDP(message);
            globalClientPtr->receiveConfirmation(globalClientPtr->messageID - 1, message);
        }
    }

    
    //close(globalClientPtr->sock_fd);
    exit(0); // Ukončení programu s daným signálem
}



void print_help() {
    std::cout << "Usage: program_name [options]\n"
              << "Options:\n"
              << " -t <tcp|udp>                Transport protocol used for connection\n"
              << " -s <IP address|hostname>    Server IP or hostname\n"
              << " -p <port>                   Server port\n"
              << " -d <timeout>                UDP confirmation timeout\n"
              << " -r <retransmissions>        Maximum number of UDP retransmissions\n"
              << " -h                          Prints this help output and exits\n";
}




std::vector<std::string> parseInput(std::vector<std::string> inputParts) {
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




void stateStart(IPK24_Client& client, std::vector<std::string> inputParts, std::string command){
	
	
	if(command == "/auth"){
        
        std::vector<std::string> parsedParts = parseInput(inputParts);
        if(parsedParts.empty()){
            std::cerr << "Invalid input" << std::endl;
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

        std::cerr << "ERR: Invalid command ! nothing happenned." << std::endl;
        client.state = Client_State::start; // HANDLE THIS, REPEAT UNTIL AUTH IS SUCCESSFUL
        return;
    }

    
}

void caseAuth(IPK24_Client& client){
    std::vector<std::string> reply = split(client.reply);

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
            std::cerr << "Invalid input" << std::endl;
            
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


void caseAuthUDP(IPK24_Client& client , std::vector<std::string> inputParts){
        uint16_t authMsgID = 0x00;
    
        std::string response = client.recieveDataUDP();

        uint8_t type = static_cast<uint8_t>(response[0]);
        uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
        if(type == 0x01){
            uint8_t result = static_cast<uint8_t>(response[3]);
            if(result == 0x01){
                
                std::vector<uint8_t> message;
                message.push_back(0x00); // confirm
                message.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
                message.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Horní bajt CHANGE AS WELL
                client.sendDataUDP(message);
                
                client.state = Client_State::open;
                return;
            }else{

                std::vector<uint8_t> messageConfirm;
                messageConfirm.push_back(0x00); // confirm
                messageConfirm.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
                messageConfirm.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Horní bajt CHANGE AS WELL
                client.sendDataUDP(messageConfirm);

                std::vector<uint8_t> message;
                if(!std::getline(std::cin, client.message)){
                    exit(23); // EOF , END OF FILITO BRO
                }
                std::vector<std::string> parsedParts = parseInput(split(client.message));
                if(parsedParts.empty()){
                    std::cerr << "Invalid input" << std::endl;
                    return;
                }
                client.sendAuth(parsedParts);// ADD MESSAGE 
                client.state = Client_State::auth;

            }
        }else if(type == 0xfe){
            std::vector<uint8_t> message;
            message.push_back(0x00); // confirm
            message.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
            message.push_back(static_cast<uint8_t>(messageID & 0xFF));
            client.sendDataUDP(message);
            client.state = Client_State::end;
            // Horní bajt CHANGE AS W

        }


}   





int main(int argc, char *argv[]){

    

    // Local variables needed for the program
    std::string userName;
    std::string secret;
    uint16_t port;
    uint16_t timeout;
    uint8_t retrans = 3;
    std::string ip;
    std::string protocol;
    std::vector<uint8_t> message;
    int option;


    // Parse the input arguments
    while((option = getopt(argc , argv, "t:s:p:d:r:h")) != -1){
        switch(option){

            case 't':
                protocol = optarg;
                if (protocol != "tcp" && protocol != "udp") {
                    std::cerr << "Invalid protocol. Use 'tcp' or 'udp'.\n";
                    return 1;
                }
                break;
            case 's':
                ip = optarg;
                break;
            case 'p':
                port = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'd':
                timeout = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'r':
                retrans = static_cast<uint8_t>(std::stoi(optarg));
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                // getopt automatically prints an error message for unrecognized option characters or missing option arguments
                return 1;
            default:
                print_help();
                return 1;
        }
    }
    
    
    

    IPK24_Client client(port, ip, protocol == "tcp" ? SOCK_STREAM : SOCK_DGRAM);
    // signal(SIGINT, signalHandler(client));
    globalClientPtr = &client;
    signal(SIGINT, signalHandler);
    // client.connectTCP();
    if(client.sock_protocol == SOCK_STREAM){
        client.connectTCP();
    }
    fd_set readfds;
    struct timeval tv;
    

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    
    if(!std::getline(std::cin, client.message)){
        if(client.sock_protocol == SOCK_STREAM){
            // TCP only sends BYE and exit
            client.sendData("BYE\r\n");
            
        }else{ // UDP protocol must send BYE, receive confirmation and then exit.

            std::vector<uint8_t> message;
            message.push_back(0xFF); // Předpokládáme, že 0xFF je kód pro ukončení
            message.push_back(static_cast<uint8_t>(client.messageID >> 8));
            message.push_back(static_cast<uint8_t>(client.messageID & 0xFF));
            client.sendDataUDP(message);
            client.receiveConfirmation(client.messageID - 1, message);
        
        }
        
        // close the socket and exit succesfuly
        exit(0);
        
        
    }
    std::vector<std::string> inputParts = split(client.message);
    std::vector<std::string> reply;
    std::string command = inputParts[0];
    std::string joinedInputMsg;
    int activity;
    int i = 0;
    while(true){

        switch ( client.state )
        {   
            case(Client_State::start):

                stateStart(client, inputParts, command);
                
                break;
            case( Client_State::auth ):

                if( protocol == "tcp" ){
                    caseAuth(client);
                }else{
                    caseAuthUDP(client, inputParts);
                }

                break;
            case( Client_State::open ):
                FD_ZERO(&readfds);
                FD_SET(client.sock_fd, &readfds);
                FD_SET(STDIN_FILENO, &readfds);
            
                activity = select(std::max(client.sock_fd, STDIN_FILENO) + 1, &readfds, NULL, NULL, &tv); // select between socket and stdin

                // first check if there is something to read from the server
                if(FD_ISSET(client.sock_fd, &readfds)){
                    // read data and validate
                    if(protocol == "tcp"){
                        client.handleTCPincome(inputParts);
                        break;
                    }else{
                        
                        client.handleUDPincome();
                        break;
                        
                    }
                    
                }
                // then check if there is something to read from the user
                if(FD_ISSET(STDIN_FILENO, &readfds) && client.replyReceived == true){
                    // read, split and validate user input
                    if( !std::getline(std::cin, client.message)){
                        // std::cerr << "EOF" << std::endl;
                        client.state = Client_State::end;
                        break;
                    }
                    client.handleUserInput();
                    
                }
            
            break;
            
        case( Client_State::end ):
            
            if(client.sock_protocol == SOCK_STREAM){ // TCP only sends BYE and exit
                client.sendData("BYE\r\n");
                
            }else{ // UDP protocol must send BYE, receive confirmation and then exit.

                std::vector<uint8_t> message;
                message.push_back(0xFF); // Předpokládáme, že 0xFF je kód pro ukončení
                message.push_back(static_cast<uint8_t>(client.messageID >> 8));
                message.push_back(static_cast<uint8_t>(client.messageID & 0xFF));
                client.sendDataUDP(message);
                client.receiveConfirmation(client.messageID - 1, message);
            
            }
            
            client.closeConnection(); // close the socket and exit succesfuly
            exit(0);
            
        case (Client_State::error):
            //print error
            exit(1);
            
        }


    }

    // std::string msg = "AUTH xhuska03 AS husix USING 39246c67-2331-4e85-aeec-750a763983a1\r\n";

    return 0;
}
