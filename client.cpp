#include "client.hpp"




IPK24_Client::IPK24_Client(int port, std::string ip, int sock_protocol){

    if(port < 0 || port > 65535){
        printf("Invalid port\n");
        exit(1);
    }
    this->port = port;
    this->ip = ip;
    this->sock_protocol = sock_protocol;
    std::array<char, 1024> buffer{0};
    this->state = Client_State::start;

    
    // create a new socket
    this->sock_fd = socket(AF_INET, this->sock_protocol , 0);

    if(this->sock_fd == -1){
        printf("Error creating socket\n");
        exit(1);
    }

    printf("Socket created\n");

    

        
        // instance of server
        this->host = gethostbyname(this->ip.c_str());

        if(this->host == NULL){
            printf("Error getting host\n");
            exit(1);
        }
        printf("Host found\n");

        this->server_address = {0};
        this->server_address.sin_family = AF_INET;
        this->server_address.sin_port = htons(this->port);
        memcpy(&this->server_address.sin_addr.s_addr, this->host->h_addr, this->host->h_length);

        if(this->sock_protocol == 2){
            printf("INFO: Server socket: %s : %d \n",
                inet_ntoa(server_address.sin_addr),
                ntohs(server_address.sin_port));

        }
        this->address = (struct sockaddr *)&this->server_address;
        
    
    printf("Client created\n");
    
}


void IPK24_Client::connectTCP(){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int connection = connect(this->sock_fd, this->address, address_len);
    if(connection < 0){
        printf("Error connecting to server\n");
        exit(1);
    }
    printf("Connected to server\n");
}

void IPK24_Client::closeConnection(){
    close(this->sock_fd);
    printf("Socket closed\n");
}



std::string IPK24_Client::recieveDataUDP(){
    sockaddr_in sender_address = {0};
    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(0);
    socklen_t address_len2 = sizeof(sender_address);

    socklen_t address_len = sizeof(this->server_address);
    int flags = 0;
    printf("AWOJ");
    std::array<char, 1024> buffer{0};
    int bytes_rx = recvfrom(this->sock_fd, buffer.data(), 1024, flags,(struct sockaddr*)&sender_address, &address_len2);
    if(bytes_rx == -1){
        printf("Error receiving data\n");
        exit(1);
    }
    sockaddr_in *sender_addr_in = (sockaddr_in*)&sender_address; // Přetypování pro práci s IPv4
    int sender_port = ntohs(sender_addr_in->sin_port); // Získání portu odesílatele
    
    printf("Data received from port: %d\n", sender_port);
    printf("Data: %s\n", buffer.data());
    return buffer.data();
}



// std::string getMessage(std::string message){
//     std::vector<std::string> kar = split(message);
//     std::string Message = kar[0];

//     switch(Message){
//         case "AUTH":
//             return Messages::AUTH;
//         case "/join":
//             // std::string msgToSend = "JOIN " + kar[1] + " AS " client. + "\r\n";
//             return msgToSend;
//         case "ERR":
//             return Messages::ERR;
//         case "BYE":
//             return Messages::BYE;
//         case "MSG":
//             return Messages::MSG;
//         case "REPLY":
//             return Messages::REPLY;
//     }
    
// }


void IPK24_Client::sendDataUDP(std::vector<uint8_t> message){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int bytes_tx = sendto(this->sock_fd, message.data(), message.size(), flags, this->address, address_len);
    if(bytes_tx == -1){
        printf("Error sending data\n");
        exit(1);
    }
    printf("Data sent\n");
}

void IPK24_Client::sendDataTCP(std::string message){
    int flags = 0;
    std::array<char, 1024> buffer{0};
    int bytes_tx = write(this->sock_fd, message.c_str(), message.size());
    
   
    if(bytes_tx < 0){
        printf("Error sending data\n");
        exit(1);
    }
    // printf("Data sent : %s \n", message.c_str());
}

void IPK24_Client::closeConnectionTCP(){
    close(this->sock_fd);
    printf("Socket closed\n");
    
}
std::vector<std::string> split(const std::string& str){
    std::string part;
    std::stringstream spart(str);

    std::vector<std::string> parts;

    while(std::getline(spart, part, ' ')){
        parts.push_back(part);
    }

    return parts;
}

std::string join(const std::vector<std::string>& parts) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1)
            result += " ";
    }
    return result;
}

std::string IPK24_Client::recieveDataTCP(){
    int flags = 0;
    std::array<char, 1024> buffer{0};

    int bytes_rx = read(this->sock_fd, buffer.data(), 200);
    if(bytes_rx < 0){
        printf("Error receiving data\n");
        exit(1);
    }

    // std::cout << "Data received: " << buffer.data() << std::endl;
    return buffer.data();
}


void IPK24_Client::printMessage(std::string message){
    std::vector<std::string> pieces = split(message);
    std::string msg = pieces[0];
    std::string possibleReply = pieces[1];
    std::string usrName = pieces[2];
    // print it out to the console the cpp style not printf
    if(msg == "REPLY"){
        pieces.erase(pieces.begin(), pieces.begin() + 3);
    }else{
        pieces.erase(pieces.begin(), pieces.begin() + 4);
    }
    

    if(msg == "MSG"){
        std::cout << usrName + ": " << join(pieces) << std::endl;
    }else if (msg == "ERR"){
        std::cout << "ERROR FROM" + usrName + ": "<< join(pieces) << std::endl;
    }else if (msg == "REPLY"){
        if(possibleReply == "OK"){
            std::cout << "Succes: " << join(pieces) << std::endl;
        }else{
            std::cout << "Failure: " << join(pieces) << std::endl;
        }
    }
    
    
}


