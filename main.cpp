#include "client.cpp"


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

void stateStart(IPK24_Client& client, std::vector<std::string> inputParts, std::string command){
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
        client.message = "AUTH " + userName + " AS " + client.displayName + " USING " + secret + "\r\n";
        client.sendData(client.message);
        client.receiveData();
        client.state = Client_State::auth;
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

                    std::getline(std::cin, client.message);
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


int main(int argc, char *argv[]){

    
    std::string userName;
    std::string secret;
    uint16_t port;
    uint16_t timeout;
    uint8_t retrans = 3;
    std::string ip;
    std::string protocol;
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
    
    client.connectTCP();

    fd_set readfds;
    struct timeval tv;
    
    
    std::getline(std::cin, client.message);
    std::vector<std::string> inputParts = split(client.message);
    std::vector<std::string> reply;
    std::string command = inputParts[0];
    std::string joinedInputMsg;
    int activity;
    while(true){
        // print the state
        switch ( client.state )
        {
            case(Client_State::start):

                stateStart(client, inputParts, command);
                
                break;
                 
            case( Client_State::auth ):
                
               
                caseAuth(client);
                    break;
                

            case( Client_State::open ):
                    FD_ZERO(&readfds);
                    FD_SET(client.sock_fd, &readfds);
                    FD_SET(STDIN_FILENO, &readfds);
                    tv.tv_sec = 0;
                    tv.tv_usec = 0;

                    activity = select(std::max(client.sock_fd, STDIN_FILENO) + 1, &readfds, NULL, NULL, &tv);

                    if(activity < 0){
                        printf("Error in select\n");
                        exit(1);
                    }

                    if(FD_ISSET(client.sock_fd, &readfds)){
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
                        
                        

                    
                    }
                    if(FD_ISSET(STDIN_FILENO, &readfds)){
                        std::getline(std::cin, client.message);
                        inputParts = split(client.message);
                        command = inputParts[0];
                        if(command == "/join")
                        {
                            client.message = "JOIN " + inputParts[1] + " AS " + client.displayName + "\r\n";
                            client.sendData(client.message);
                            client.receiveData();
                            break;
                        }else if (command == "/rename"){
                            client.displayName = inputParts[1];
                            break;
                        }else if(command == "/help"){
                            std::cout << "Commands: /join, /rename, /auth, /help" << std::endl;
                            break;
                        }else if(command == "/auth"){
                            std::cerr << "You are already authenticated !" << std::endl;
                            break;
                        }else if(command[0] != '/'){
                            
                            joinedInputMsg = join(inputParts);
                            client.message = "MSG FROM " + client.displayName + " IS " + joinedInputMsg + "\r\n";
                            client.sendData(client.message);
                            
                            break;

                        }
                        
                    }
                    
                
                
                // command = kar[0];
                // if(command == "/join"){
                //     client.message = "JOIN " + kar[1] + " AS " + client.displayName + "\r\n";
                //     client.sendData(client.message);
                //     client.receiveData();
                //     break;
                // }
                break;
            
        case( Client_State::end ):
            //print success
            printf("Success\n");
            client.closeConnectionTCP();
            exit(0);
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
