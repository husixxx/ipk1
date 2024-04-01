#include "client.cpp"
#include "main.hpp"
#include "fsm.cpp"

IPK24_Client* globalClientPtr = nullptr;

void signalHandler(int signum) {
    
    if (globalClientPtr != nullptr) {
        // Odeslat zprávu před ukončením
        if(globalClientPtr->sock_protocol == SOCK_STREAM){
            //aaa
            globalClientPtr->sendDataTCP("BYE\r\n");
            
            // globalClientPtr->closeConnectionTCP();
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

    
    close(globalClientPtr->sock_fd);
    
    exit(0);
    // Ukončení programu s daným signálem
}

void segfaultHandler(int signum) {
    std::cerr << "Fatal error occurred." << std::endl;
    exit(1);
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


int main(int argc, char *argv[]){

    

    // Local variables needed for the program
    // user info
    std::string userName; // current name
    std::string secret; // secret
    // arguments values
    int port = 4567;
    int timeout = 250;
    int retryCount = 3;
    std::string ip;
    std::string protocol;
    std::vector<uint8_t> message;
    int option;
    // user input work
    std::vector<std::string> reply;
    std::string joinedInputMsg;


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
                port = std::stoi(optarg);
                break;
            case 'd':
                timeout = std::stoi(optarg);
                break;
            case 'r':
                retryCount = std::stoi(optarg);
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
    
    
    IPK24_Client client(port, ip, protocol == "tcp" ? SOCK_STREAM : SOCK_DGRAM, timeout, retryCount); // construct the client object
    globalClientPtr = &client; // set the global pointer for success end.
    signal(SIGINT, signalHandler);  // capture the Ctrl-c signal for success end

    signal(SIGSEGV, segfaultHandler); // capture the segmentation fault signal
    // Finite State Machine object
    FSM fsm;

    
    if(client.sock_protocol == SOCK_STREAM){ // open connection if TCP
        client.connectTCP();
    }
    


    // set the timeout for select to NULL, instant reply
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    
    // scan first lane, to start
    if(!std::getline(std::cin, client.message)){
        client.sendBye(); // send BYE
        
        // close the socket and exit succesfuly
        return 0;
        
    }

    std::vector<std::string> inputParts = split(client.message);
    std::string command = inputParts[0];
    int activity;
   
    while(42){

        switch ( client.state )
        {   
            case(Client_State::start):
                fsm.stateStart(client, inputParts, command);
                
                if(client.state == Client_State::start){ // Give user a retry if he didnt wrote auth

                    if(!std::getline(std::cin, client.message)){
                        client.sendBye(); // send BYE
                        // close the socket and exit succesfuly
                        return 0;
        
                    }
                    if(client.message.empty()){
                        continue;
                    }
                    inputParts = split(client.message);
                    command = inputParts[0];
                }
                
                break;
            case( Client_State::auth ):

                if( protocol == "tcp" ){
                    fsm.stateAuthTCP(client);
                }else{
                    fsm.stateAuthUDP(client, inputParts);
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
            
            client.sendBye(); // send BYE 
            client.closeConnection(); // close the socket and exit succesfuly
            return 0;
            
        case (Client_State::error):
            //print error
            return 1; 
        }
    }

    // std::string msg = "AUTH xhuska03 AS husix USING 39246c67-2331-4e85-aeec-750a763983a1\r\n";
    return 0;
}
