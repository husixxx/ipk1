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

   this->messageID = 0;
    
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

        // bind(this->sock_fd, (struct sockaddr *)&this->server_address, sizeof(this->server_address));
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

void IPK24_Client::closeConnectionUDP(){
    this->sendData("BYE\r\n");
    this->receiveConfirmation(this->messageID - 1);
    close(this->sock_fd);
    printf("Socket closed\n");
    
}


std::string IPK24_Client::recieveDataUDP() {
    // Zde nepotřebujeme inicializovat sockaddr_in pro místní adresu, protože nebudeme volat bind
    sockaddr_in sender_address;
    socklen_t sender_address_len = sizeof(sender_address);
    std::array<char, 1024> buffer{0};

    // Přijetí dat
    int bytes_rx = recvfrom(this->sock_fd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&sender_address, &sender_address_len);
    if (bytes_rx == -1) {
        perror("Error receiving data");
        return "";
    }

    // Získání portu odesílatele
    int sender_port = ntohs(sender_address.sin_port);
    this->server_address = sender_address;
    this->address = (struct sockaddr *)&this->server_address;
    // Odpověď serveru byla přijata, vytvoříme nový socket a nasloucháme na portu odesílatele
    
    // printf("Message Type: %d\n", type);
    // printf("Message ID: %d\n", messageID);
    // printf("Content: %s\n", content.c_str());

    // Nastavíme adresu klienta na nulu a port na port odesílatele
    
    
    // Port odesílatele

    
    
   // printf("Data received from port: %d\n", sender_port);
    this->printUDPmessagge(std::string(buffer.data(), bytes_rx));

    return std::string(buffer.data(), bytes_rx);
}




void IPK24_Client::printUDPmessagge(std::string message){
    
    size_t nullTerminatorPos;
    uint8_t type = static_cast<uint8_t>(message[0]);
    std::string messageContents;
    switch(type){

        case 0x01:
            messageContents = message.substr(5); // Přeskočit prvních 5 bajtů (1 byte message type + 2 bytes MessageID + 1 byte Result + 2 bytes Ref_MessageID)
            nullTerminatorPos = messageContents.find('\0');
            if (nullTerminatorPos != std::string::npos) {
                messageContents = messageContents.substr(0, nullTerminatorPos); // Extrahovat pouze do nulového byte
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }

            std::cout << "Success: " << messageContents << "\n";
            break;

        case 0x04:

            std::string displayName = message.substr(3);
            nullTerminatorPos = displayName.find('\0');
            if (nullTerminatorPos != std::string::npos) {
                displayName = displayName.substr(0, nullTerminatorPos); // Extrahovat pouze do nulového byte
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }
            
            std::string content = message.substr(3 + displayName.size() + 1);
            size_t nullTerminatorPos2 = content.find('\0');
            if (nullTerminatorPos2 != std::string::npos) {
                content = content.substr(0, nullTerminatorPos2); // Extrahovat pouze do nulového byte
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }


            std::cout << displayName << ": " << content << "\n";
            break;



                
    }
    
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

// std::string IPK24_Client::receiveReply(uint16_t expectedMessageID){
//     fd_set readfds;
//     FD_ZERO(&readfds);
//     FD_SET(this->sock_fd, &readfds);

//     struct timeval tv;
//     tv.tv_sec = 0;
//     tv.tv_usec = 250 * 1000; // Timeout for select

//     uint16_t receivedMessageID;
//     uint8_t messageType;
//     bool success = false;

    
//     int activity = select(this->sock_fd + 1, &readfds, NULL, NULL, &tv);
//     if (activity < 0) {
//         // Handle select error
//         perror("select error");
//         break;
//     } else if (activity == 0) {
//         // Timeout, no data received
//         continue;
//     } else {
//         if (FD_ISSET(this->sock_fd, &readfds)) {
//             // We have received data
//             std::string response = recieveDataUDP();
//             uint8_t type = static_cast<uint8_t>(response[0]);
//             std::cout << "Type: " << type << std::endl;
//             if(type == 0x01){
//                 uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
//                 std::vector<uint8_t> message2;
//                 message2.push_back(0x00);
//                 message2.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
//                 message2.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
//                 this->sendDataUDP(message2);
//             }else{
//                 continue;
            
//             }
//         }
//     }
    

//     return "";
// }

bool IPK24_Client::receiveConfirmation(uint16_t expectedMessageID){
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(this->sock_fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 250; // Timeout for select

    for(int i = 0; i < 3 ; i++){
        int activity = select(this->sock_fd + 1, &readfds, NULL, NULL, &tv);
        if (activity < 0) {
            // Handle error
        } else if (activity == 0) {
            continue;
        } else {
            if (FD_ISSET(this->sock_fd, &readfds)) {
                // We have received data
                // Read the confirmation, validate it, and return true if it matches messageId
                std::string response = recieveDataUDP();
                uint8_t type = static_cast<uint8_t>(response[0]);
                if(type == 0x00){

                    
                    uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
                    if(messageID == expectedMessageID){
                        return true;
                    }else{
                        continue;
                    }
                if(type == 0x01){
                    std::cout << "Type KOKOT: " << type << std::endl;
                }
                    
                }else{
                    continue;
                }
            }
        }
    }

    return false;
}
void IPK24_Client::sendDataUDP(std::vector<uint8_t> message){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int bytes_tx = sendto(this->sock_fd, message.data(), message.size(), flags, this->address, address_len);
    this->messageID += 0x0001;
    if(bytes_tx < 0){
        printf("Error sending data\n");
        exit(1);
    }
//    printf("Data sent\n");
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



void IPK24_Client::sendMessage(std::vector<std::string> inputParts){
    if(this->sock_protocol == SOCK_STREAM){

        std::string joinedInputMsg = join(inputParts);
        this->message = "MSG FROM " + this->displayName + " IS " + joinedInputMsg + "\r\n";
        this->sendData(this->message);
    }
    else{
        std::vector<uint8_t> message;
        uint8_t refID = this->messageID;
        message.push_back(0x04); // MSG typ
        message.push_back(static_cast<uint8_t>(this->messageID >> 8)); // Horní bajt
        message.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); // Dolní bajt
        for (char c : this->displayName) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci
        for (char c : join(inputParts)) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci
        this->sendDataUDP(message);
        this->receiveConfirmation(refID); // confirm
    }
}