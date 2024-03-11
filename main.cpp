#include "client.cpp"


IPK24_Client* globalClientPtr = nullptr;

void signalHandler(int signum) {
    
    if (globalClientPtr != nullptr) {
        // Odeslat zprávu před ukončením
        std::vector<uint8_t> message;
        message.push_back(0xFF); // Předpokládáme, že 0xFF je kód pro ukončení
        message.push_back(static_cast<uint8_t>(globalClientPtr->messageID >> 8));
        message.push_back(static_cast<uint8_t>(globalClientPtr->messageID & 0xFF));
        globalClientPtr->sendDataUDP(message);
        globalClientPtr->receiveConfirmation(globalClientPtr->messageID - 1);
    }

    std::cout << "SIGINT caught, shutting down." << std::endl;
    exit(signum); // Ukončení programu s daným signálem
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

    // Předpokládáme, že vstup má správný formát a dostatečný počet částí
    if (inputParts.size() < 4) {
        return parsedInput; // Vrací prázdný vektor, pokud nejsou všechny části přítomny
    }

    std::string userName = inputParts[1];
    std::string secret = inputParts[2];
    std::string displayName = inputParts[3]; 
    // MessageContent není součástí inputParts v tvém příkladu

    // Regex pro validaci
    std::regex userAndChannelRegex("^[A-Za-z0-9\\-]{1,20}$"); // Pro Username a ChannelID
    std::regex secretRegex("^[A-Za-z0-9\\-]{1,128}$"); // Pro Secret
    std::regex displayNameRegex("^[\x21-\x7E]{1,20}$"); // Pro DisplayName

    // Kontrola Username a ChannelID
    if (!std::regex_match(userName, userAndChannelRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }

    // Kontrola Secret
    if (!std::regex_match(secret, secretRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }

    // Kontrola DisplayName
    if (!std::regex_match(displayName, displayNameRegex)) {
        exit(10);
        return parsedInput; // Chybný vstup
    }

    // Všechny kontroly prošly, přidáme validní hodnoty do parsedInput
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
            client.state = Client_State::error;
            return;
        }
        std::string userName = parsedParts[0];
        client.displayName = parsedParts[2]; 
        std::string secret = parsedParts[1];

        // client.message = "1 " + userName + " " + client.displayName + " " + secret + "\r\n";

        if(client.sock_protocol == SOCK_STREAM){

            client.connectTCP();
            client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
            client.sendData(client.message);
            client.receiveData();
            client.state = Client_State::auth;
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
        // client.state = Client_State::auth;


    }else{

        printf("Error\n");
        client.state = Client_State::error; // HANDLE THIS, REPEAT UNTIL AUTH IS SUCCESSFUL

    }

    // Function code...
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
            exit(23); // EOF , END OF FILITO BRO

        }
        std::vector<std::string> inputParts = split(client.message);
        if(inputParts.size() != 4){
            std::cerr << "Invalid input" << std::endl;
            client.state = Client_State::error;
            return;
        }
        std::string command = inputParts[0];
        std::string userName = inputParts[1];
        client.displayName = inputParts[3]; 
        std::string secret = inputParts[2];


        if( command != "AUTH" || userName.empty() || client.displayName.empty() || secret.empty() ){
            std::cerr << "Invalid input bro" << std::endl;
            client.state = Client_State::error;
            return;
        }


        
        client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
        client.sendData(client.message);
                    
        }
    }

void caseAuthUDP(IPK24_Client& client){
    uint16_t authMsgID = 0x00;
    client.receiveConfirmation(client.messageID - 1);
    std::string response = client.recieveDataUDP();

        // uint8_t type_conf = static_cast<uint8_t>(confirm[0]);
        // uint16_t messageIDconfirm = static_cast<uint16_t>(confirm[1]) << 8 | static_cast<uint16_t>(confirm[2]);

        // if(messageIDconfirm != client.messageId){
        //     std::cerr << "Invalid message ID" << std::endl;
        //     client.state = Client_State::error;
        //     return;
        // } 

        uint8_t type = static_cast<uint8_t>(response[0]);
        uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
        if(type == 0x01){
            uint8_t result = static_cast<uint8_t>(response[3]);
            if(result == 0x01){
                std::cout << "Success" << std::endl;
                std::vector<uint8_t> message;
                message.push_back(0x00); // confirm
                message.push_back(authMsgID); // UPRAVOVAT maybe ? CHANGE in reply he sents ID msg
                message.push_back(0x00); // Horní bajt CHANGE AS WELL
                client.sendDataUDP(message);
                
                client.state = Client_State::open;
                return;
            }
        }else{
            authMsgID += 0x01; // increment for retry!
        }

}   





int main(int argc, char *argv[]){

    // Ctrl+C handler

    
    std::string userName;
    std::string secret;
    uint16_t port;
    uint16_t timeout;
    uint8_t retrans = 3;
    std::string ip;
    std::string protocol;
    std::vector<uint8_t> message;
    int option;
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

    fd_set readfds;
    struct timeval tv;
    
    
    std::getline(std::cin, client.message);
    std::vector<std::string> inputParts = split(client.message);
    std::vector<std::string> reply;
    std::string command = inputParts[0];
    std::string joinedInputMsg;
    int activity;
    int i = 0;
    while(true){


        
        
        
        // print the state
        //std::cout << "State: " << static_cast<int>(client.state) << std::endl;
        switch ( client.state )

        {   
            
            case(Client_State::start):

                stateStart(client, inputParts, command);
                
                break;
                 
            case( Client_State::auth ):

                if( protocol == "tcp" ){
                    caseAuth(client);
                }else{
                    caseAuthUDP(client);
                }
                
               
                
                break;
                

            case( Client_State::open ):



               

                    
                    FD_ZERO(&readfds);
                    FD_SET(client.sock_fd, &readfds);
                    FD_SET(STDIN_FILENO, &readfds);

                    tv.tv_sec = 0;
                    tv.tv_usec = 0;
                    //flags

                    activity = select(std::max(client.sock_fd, STDIN_FILENO) + 1, &readfds, NULL, NULL, &tv);

                    
                    // first check if there is something to read from the server
                    if(FD_ISSET(client.sock_fd, &readfds)){
                        // read data and validate
                        if(protocol == "tcp"){
                            client.receiveData();
                            reply = split(client.reply);

                            if((reply[0] == "AUTH" || reply[0] == "JOIN"))
                            {
                                client.sendData("ERROR FROM " + client.displayName + " IS " + join(inputParts) + "\r\n");
                                client.state = Client_State::error;
                                break;
                            }

                            if(reply[0] == "ERR"){
                                client.sendData("BYE\r\n");
                                client.state = Client_State::end;
                                break;
                            }
                            
                            if(reply[0] == "BYE"){
                                client.state = Client_State::end;
                                break;
                            }
                        }else{
                            
                            client.handleUDPincome();
                            //std::cout << "s" << std::endl;
                            client.state = Client_State::open;
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
                        inputParts = split(client.message);
                        command = inputParts[0];
                        if(command == "/join")
                        {
                            client.sendJoin(inputParts);
                            client.state = Client_State::open;
                            
                            break;
                        }
                        else if (command == "/rename"){
                            client.displayName = inputParts[1];
                            break;
                        }
                        else if(command == "/help"){
                            std::cout << "Commands: /join, /rename, /auth, /help" << std::endl;
                            break;
                        }
                        else if(command == "/auth"){
                            std::cerr << "You are already authenticated !" << std::endl;
                            break;
                        }
                        else if(command[0] != '/'){
                            // not command, must be message
                            client.sendMessage(inputParts);
                            
                            break;

                        }else{
                            std::cerr << "Invalid command" << std::endl;
                            break;
                        }
                        
                    }
                
                break;
            
        case( Client_State::end ):
            //print success
            std::cout << "Success ENDDD" << std::endl;
            
            // message.push_back(0xFF); // BYE typ
            // message.push_back(static_cast<uint8_t>(client.messageID >> 8)); // Horní bajt
            // message.push_back(static_cast<uint8_t>(client.messageID & 0xFF)); // Dolní bajt
            // client.sendDataUDP(message);
            // client.receiveConfirmation(client.messageID - 1);

            // exit(0);
        case (Client_State::error):
            //print error
            printf("Error\n");
            client.sendData("BYE\r\n");
            client.state = Client_State::end;
            break;
        }


    }

    // std::string msg = "AUTH xhuska03 AS husix USING 39246c67-2331-4e85-aeec-750a763983a1\r\n";

    return 0;
}
