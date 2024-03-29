#include "client.hpp"




/**
 * @brief Constructs an instance of the IPK24_Client class.
 * 
 * @param port The port number to connect to.
 * @param ip The IP address of the server to connect to.
 * @param sock_protocol The socket protocol to use.
 * 
 * This constructor initializes the IPK24_Client object with the specified port, IP address, and socket protocol.
 * It also creates a new socket and sets the initial state of the client.
 * If there is an error creating the socket or getting the host, an error message is printed and the program exits.
 * 
 * @see Client_State
 */
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
    this->replyReceived = true;
    this->messageID = 0;
    
    // create a new socket
    this->sock_fd = socket(AF_INET, this->sock_protocol , 0);
    

    if(this->sock_fd == -1){
        printf("Error creating socket\n");
        exit(1);
    }

    

        this->host = gethostbyname(this->ip.c_str());

        if(this->host == NULL){
            printf("Error getting host\n");
            exit(1);
        }
       // printf("Host found\n");

        this->server_address = {0};
        this->server_address.sin_family = AF_INET;
        this->server_address.sin_port = htons(this->port);
        memcpy(&this->server_address.sin_addr.s_addr, this->host->h_addr, this->host->h_length);

        // bind(this->sock_fd, (struct sockaddr *)&this->server_address, sizeof(this->server_address));
        // if(this->sock_protocol == 2){
        //    // printf("INFO: Server socket: %s : %d \n",
        //         inet_ntoa(server_address.sin_addr),
        //         ntohs(server_address.sin_port));

        // }
        this->address = (struct sockaddr *)&this->server_address;
        
    
   // printf("Client created\n");
    
}


/**
 * @brief Connects the client to the server using TCP protocol.
 * 
 * This function establishes a TCP connection between the client and the server.
 * It uses the socket file descriptor and server address stored in the IPK24_Client object.
 * 
 * 
 * 
 */
void IPK24_Client::connectTCP(){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int connection = connect(this->sock_fd, this->address, address_len);
    if(connection < 0){
        printf("Error connecting to server\n");
        exit(1);
    }
   // printf("Connected to server\n");
}


/**
 * @brief Connects the client to the server using TCP protocol.
 * 
 * This function establishes a TCP connection between the client and the server.
 * It uses the socket file descriptor and server address stored in the IPK24_Client object.
 * 
 */
void IPK24_Client::closeConnection(){
    close(this->sock_fd);
    //printf("Socket closed\n");
    
}


/**
*
*    @brief Recieves data from the server using UDP protocol.
*
*   This function receives data from the server using the UDP protocol.
*    It reads the data and stores them in buffer.
*   It then prints the received message by the wanted format.
*   Finally, it returns the read data.
*   @return buffer with data
*/
std::string IPK24_Client::recieveDataUDP() {
    
    sockaddr_in sender_address;
    socklen_t sender_address_len = sizeof(sender_address);
    std::array<char, 1024> buffer{0};

    // receive data
    int bytes_rx = recvfrom(this->sock_fd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&sender_address, &sender_address_len);
    if (bytes_rx == -1) {
        perror("Error receiving data");
        return "";
    }

    // get sender info actuall port
    int sender_port = ntohs(sender_address.sin_port);
    this->server_address = sender_address;
    this->address = (struct sockaddr *)&this->server_address;
    
    // print received message by the wanted format
    this->printUDPmessagge(std::string(buffer.data(), bytes_rx));

    // return read data
    return std::string(buffer.data(), bytes_rx);
}


/**
 * @brief Prints the received UDP message by the wanted format.
 *  
 * 
 * Prints the message in valid format to the standard output.
 * 
 * @param message The message to print.
 * 
 */
void IPK24_Client::printUDPmessagge(std::string message){
    
    size_t nullTerminatorPos;
    uint8_t type = static_cast<uint8_t>(message[0]);
    uint8_t reply = static_cast<uint8_t>(message[3]);
    std::string displayName;
    std::string content;
    std::string messageContents;
    size_t nullTerminatorPos2;
    switch(type){

        case 0x01:
            messageContents = message.substr(6); // skip 5 bytes (1 byte message type + 2 bytes MessageID + 1 byte Result + 2 bytes Ref_MessageID)
            nullTerminatorPos = messageContents.find('\0');
            if (nullTerminatorPos != std::string::npos) {
                messageContents = messageContents.substr(0, nullTerminatorPos); // extract only to the null byte
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }

            if(reply == 0x00){
                std::cerr << "Failure: " << messageContents << "\n";
            }else if(reply == 0x01){

            std::cerr << "Success: " << messageContents << "\n"; //    ZMENAAAAAAAAAAAAAAAAA !
            }

            break;

        case 0x04:
            
            displayName = message.substr(3);
            nullTerminatorPos = displayName.find('\0');
            if (nullTerminatorPos != std::string::npos) {
                displayName = displayName.substr(0, nullTerminatorPos); // null byte only
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }
            
            content = message.substr(3 + displayName.size() + 1);
            nullTerminatorPos2 = content.find('\0');
            if (nullTerminatorPos2 != std::string::npos) {
                content = content.substr(0, nullTerminatorPos2); 
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }


            std::cout << displayName << ": " << content << "\n";
            break;

        case 0xFE:
            
            displayName = message.substr(3);
            nullTerminatorPos = displayName.find('\0');
            if (nullTerminatorPos != std::string::npos) {
                displayName = displayName.substr(0, nullTerminatorPos); // null byte only
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;
            }
            
            content = message.substr(3 + displayName.size() + 1);
            nullTerminatorPos2 = content.find('\0');
            if (nullTerminatorPos2 != std::string::npos) {
                content = content.substr(0, nullTerminatorPos2); 
            } else {
                std::cerr << "ERR: MessageContents not properly terminated with null byte.\n";
                return;     
            }


            std::cerr << "ERR FROM "<< displayName << ": " << content << "\n";
            break;


        case 0x00:

            break;
        default:
        // print out the type of the message

            
            std::cerr << "ERR: Unknown message type" << type << "\n";
            break;
    }
    
}



/**
 * @brief Implements the logic of the UDP protocol package loss, retransmission, and confirmation.
 * 
 * This function sends the message to the server and waits for the confirmation.
 * If the confirmation is not received within the timeout, the message is retransmitted.
 * This process is repeated three times.
 * @param message The message to send.
 * @return True if the confirmation is received, false otherwise.
 * @param expectedMessageID The expected message ID.
*/
bool IPK24_Client::receiveConfirmation(uint16_t expectedMessageID, std::vector<uint8_t> message){
    
    const int timeoutMs = 250;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0; // Timeout for select

    for(int i = 0; i < 3 ; i++){

        // WHILE CURR TIME < 250MS,
        auto startTime = std::chrono::high_resolution_clock::now();
        while(true){
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(this->sock_fd, &readfds);


            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            int remainingTime = timeoutMs - elapsedTime;
            if (remainingTime <= 0) {
                // Pokud jsme překročili 250 ms, přerušíme vnitřní smyčku a pokusíme se o další retransmisi
                break;
            }

            
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

                        
                        uint16_t messageID = static_cast<uint16_t>(response[1]) & 0xFF | static_cast<uint16_t>(response[2]);
                        if(messageID == expectedMessageID){
                            return true;
                        }else{
                            std::cout << "Message ID: " << messageID << std::endl;
                        }
                        
                    }else{
                        std::vector<uint8_t> message;
                        message.push_back(0x00);
                        uint16_t messageID = static_cast<uint16_t>(response[1]) & 0xFF | static_cast<uint16_t>(response[2]);
                        message.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
                        message.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
                        this->sendDataUDP(message);
                    }
                }
            }
        }
        sendDataUDP(message);
    }

    std::cerr << "No confirmation received, connection lost.\n";
    exit(1); // HANDLE !!
}


/**
 * @brief Sends the data to the server using the UDP protocol.
 * @param message The message to send.
 * Sends message and then increments the messageID.
*/
void IPK24_Client::sendDataUDP(std::vector<uint8_t> message){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int bytes_tx = sendto(this->sock_fd, message.data(), message.size(), flags, this->address, address_len);

    if ( message[0] != 0x00){
        this->messageID += 0x0001;
    }
    if(bytes_tx < 0){
        printf("Error sending data\n");
        exit(1);
    }
}


/**
 * @brief Sends the data to the server using the TCP protocol.
 * @param message The message to send.
*/
void IPK24_Client::sendDataTCP(std::string message){
    int flags = 0;
    std::array<char, 1024> buffer{0};
    int bytes_tx = write(this->sock_fd, message.c_str(), message.size());
    
   
    if(bytes_tx < 0){
        printf("Error sending data\n");
        exit(1);
    }
    
}

/**
 * @brief Closes the socket.
*/
void IPK24_Client::closeConnectionTCP(){
    close(this->sock_fd);
   // printf("Socket closed\n");
    
}
/**
 * @brief Splits The string into a vector of strings.
 * 
 * This function splits the input string into a vector of strings based on the space character.
 * 
 */
std::vector<std::string> split(const std::string& str){
    std::string part;
    std::stringstream spart(str);

    std::vector<std::string> parts;

    while(std::getline(spart, part, ' ')){
        parts.push_back(part);
    }

    return parts;
}

/**
 * Joins the elements of a vector of strings into a single string.
 *
 * @param parts The vector of strings to be joined.
 * @return The resulting string after joining all the elements.
 */
std::string join(const std::vector<std::string>& parts) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1)
            result += " ";
    }
    return result;
}

/**
 * @brief Receives data over TCP connection and returns it as a string.
 * 
 * @return std::string The received data as a string.
 */
std::string IPK24_Client::recieveDataTCP(){
    int flags = 0;
    std::array<char, 1500> buffer{0};

    int bytes_rx = read(this->sock_fd, buffer.data(), 1500);
    if(bytes_rx < 0){
        printf("Error receiving data\n");
        exit(1);
    }



    // std::cout << "Data received: " << buffer.data() << std::endl;
    return buffer.data();
}


/**
 * @brief Prints a message to the standard output.
 * 
 * This method takes a message as input and prints it to the console in a specific format.
 * The message is expected to be in a specific format, where the first piece is the message type,
 * the second piece is a possible reply, the third piece is the user name, and the remaining pieces
 * are the message content.
 * 
 * @param message The message to be printed.
 */
void IPK24_Client::printMessage(std::string message){
    std::vector<std::string> pieces = split(message);
    if(pieces.size() < 3){
        std::cerr << "ERR: Invalid message format\n";
        return;
    }
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
        std::cout << usrName + ": " << join(pieces);
    }else if (msg == "ERR"){
        std::cerr << "ERR FROM " + usrName + ": "<< join(pieces);
    }else if (msg == "REPLY"){
        if(possibleReply == "OK"){
            std::cerr << "Success: " << join(pieces);
        }else{
            std::cerr << "Failure: " << join(pieces);
        }
    }
    
}


/**
 * @brief Sends the message to the server.
 * 
 * This method validates the userInputed message, and then sends the message to the server using the Clients protocol, can be TCP or UDP.
 * 
 * @param message The message to send.
 */
void IPK24_Client::sendMessage(std::vector<std::string> inputParts){
    // std::regex idRegex("^[A-Za-z0-9-]{1,20}$");
    // std::regex secretRegex("^[A-Za-z0-9-]{1,128}$");
    std::regex contentRegex("^[\\x20-\\x7E]{1,1400}$"); // validating message content.
    // std::regex dnameRegex("^[\\x20-\\x7E]{1,20}$");
    if(!std::regex_match(join(inputParts), contentRegex)){
        std::cerr << "Invalid message content\n";
        return;
    }
    if(this->sock_protocol == SOCK_STREAM){ // TCP protocol

        std::string joinedInputMsg = join(inputParts);
        this->message = "MSG FROM " + this->displayName + " IS " + joinedInputMsg + "\r\n";
        this->sendData(this->message);
    }
    else{ // UDP protocol
        std::vector<uint8_t> message;
        uint8_t refID = this->messageID;
        message.push_back(0x04); // MSG typ
        message.push_back(static_cast<uint8_t>(this->messageID >> 8)); 
        message.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); 
        for (char c : this->displayName) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Null byte

        for (char c : join(inputParts)) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Null byte
        this->sendDataUDP(message);
        this->receiveConfirmation(refID, message); // confirm
    }
}


/**
 * @brief Handles the incoming TCP data.
 * 
 * This method receives the TCP data, extracts the type and message ID from the response,
 * and performs different actions based on the type of the response.
 * @param inputParts The vector of input parts.
 */
void IPK24_Client::handleTCPincome(std::vector<std::string> inputParts){
    this->receiveData();
    std::vector<std::string> reply = split(this->reply);
    if((reply[0] == "AUTH" || reply[0] == "JOIN"))
    {
        this->sendData("ERROR FROM " + this->displayName + " IS " + join(inputParts) + "\r\n");
        this->state = Client_State::error;

    }

    if(reply[0] == "ERR"){
        this->sendData("BYE\r\n");
        this->state = Client_State::end;
        
    }
    
    if(reply[0] == "BYE"){
        this->state = Client_State::end;
        
    }
    if(reply[0] == "REPLY" && reply[1] == "OK"){
        this->replyReceived = true;
        return;
    }else{
        std::cerr << "ERR: Invalid message" << std::endl;
        this->sendData("ERROR FROM " + this->displayName + " IS " + join(inputParts) + "\r\n");
        this->state = Client_State::error;
    }
    return;
}



/**
 * @brief Handles the input from stdin and processes the input based on the client state on UDP protocol.
 * This method reads the input, validate it, and if its not command then it sends the message to the server.
 * 
*/
void IPK24_Client::handleUserInput(){
    std::vector<std::string> inputParts = split(this->message);
    if(inputParts.size() < 1){
        std::cerr << "Invalid input" << std::endl;
        exit(32);
        
    }
    std::string command = inputParts[0];
    if(command == "/join")
    {
        this->sendJoin(inputParts);
        this->state = Client_State::open;
        
        return;
    }
    else if (command == "/rename"){
        this->displayName = inputParts[1];
        return;
    }
    else if(command == "/help"){
        std::cout << "Commands: /join, /rename, /auth, /help" << std::endl;
        return;
    }
    else if(command == "/auth"){
        std::cerr << "ERR: You are already authenticated !" << std::endl;
        return;
    }
    else if(command[0] != '/'){
        // not command, must be message
        this->sendMessage(inputParts);
        
        return;

    }else{
        std::cerr << "ERR: Invalid command" << std::endl;
        return;
    }
}

/**
 * @brief Construct the JOIN message based on the protocol and then sends the JOIN message to the server.
 * Then it waits for the confirmation and sets replyReceived flag that indicates to not validate stdin.
 * @param inputParts The vector of input parts.
*/
void IPK24_Client::sendJoin(std::vector<std::string> inputParts){

    // MUUUUUUST FIX !!!!!!std::regex idRegex("^[A-Za-z0-9-]{1,20}$");



    if(this->sock_protocol == SOCK_STREAM){
        std::string joinedInputMsg = join(inputParts);
        this->message = "JOIN " + inputParts[1] + " AS " + this->displayName + "\r\n";
        this->sendData(this->message);
        this->replyReceived = false;
    }
    else{
        std::vector<uint8_t> message;
        uint8_t refID = this->messageID;
        message.push_back(0x03); // JOIN typ
        message.push_back(static_cast<uint8_t>(this->messageID >> 8)); // Horní bajt
        message.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); // Dolní bajt
        for (char c : inputParts[1]) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci
        for (char c : this->displayName) {
            message.push_back(static_cast<uint8_t>(c));
        }
        message.push_back(0x00); // Nulový bajt na konci
        this->sendDataUDP(message);
        this->receiveConfirmation(refID, message); // confirm
        this->replyReceived = false;
    }
}

/**
 * @brief Handles the incoming UDP data.
 * 
 * This function receives the UDP data, extracts the type and message ID from the response,
 * and performs different actions based on the type of the response.
 */
void IPK24_Client::handleUDPincome(){
    std::string response = this->recieveDataUDP();
    uint8_t type = static_cast<uint8_t>(response[0]);
    uint16_t messageID = static_cast<uint16_t>(response[1]) << 8 | static_cast<uint16_t>(response[2]);
    
    if(type == 0x04){
        std::vector<uint8_t> message;
        message.push_back(0x00);
        message.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
        message.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
        this->sendDataUDP(message);
    }else if(type == 0x01){
        
        std::vector<uint8_t> message2;
        message2.push_back(0x00);
        message2.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
        message2.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
        this->sendDataUDP(message2);
        this->replyReceived = true;

    }else if( type == 0xfe){
        std::vector<uint8_t> message2;
        message2.push_back(0x00);
        message2.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
        message2.push_back(static_cast<uint8_t>(messageID & 0xFF));
         // Dolní bajt
        this->sendDataUDP(message2);

        std::vector<uint8_t> messageBye;
        messageBye.push_back(0xFF);
        messageBye.push_back(static_cast<uint8_t>(this->messageID >> 8)); // Horní bajt
        messageBye.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); // Dolní bajt
        this->sendDataUDP(messageBye);
        this->receiveConfirmation(this->messageID - 1, messageBye);
        
    }else if( type == 0xFF){
        std::vector<uint8_t> message2;
        message2.push_back(0x00);
        message2.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
        message2.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
        this->sendDataUDP(message2);
        exit(0);
        
        
    }else{
        
        std::vector<uint8_t> message2;
        message2.push_back(0x00);
        message2.push_back(static_cast<uint8_t>(messageID >> 8)); // Horní bajt
        message2.push_back(static_cast<uint8_t>(messageID & 0xFF)); // Dolní bajt
        this->sendDataUDP(message2);

        std::vector<uint8_t> messageERR;
        messageERR.push_back(0xFE);
        messageERR.push_back(static_cast<uint8_t>(this->messageID >> 8)); // Horní bajt
        messageERR.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); // Dolní bajt

        for (char c : this->displayName) {
            messageERR.push_back(static_cast<uint8_t>(c));
        }
        messageERR.push_back(0x00); // Null byte

        for (char c : "Invalid message") {
            messageERR.push_back(static_cast<uint8_t>(c));
        }

        this->sendDataUDP(messageERR);
        this->receiveConfirmation(this->messageID - 1, messageERR);
        this->state = Client_State::end;
        


       
        
    }
}



/// @brief Construct the AUTH message and then sends the authentication message to the server. 
/// @param parsedParts 
void IPK24_Client::sendAuth(std::vector<std::string> parsedParts){

    std::string userName = parsedParts[0];
    this->displayName = parsedParts[2]; 
    std::string secret = parsedParts[1];

    std::vector<uint8_t> message;
    // Přidání typu zprávy
    message.push_back(0x02); // AUTH typ
    message.push_back(static_cast<uint8_t>(this->messageID >> 8)); // Horní bajt
    message.push_back(static_cast<uint8_t>(this->messageID & 0xFF)); // Dolní bajt
    // Přidání Username
    for (char c : userName) {
        message.push_back(static_cast<uint8_t>(c));
    }
    message.push_back(0x00); // Nulový bajt na konci
    
    for (char c : this->displayName) {
        message.push_back(static_cast<uint8_t>(c));
    }
    message.push_back(0x00); // Nulový bajt na konci
    for (char c : secret) {
        message.push_back(static_cast<uint8_t>(c));
    }
    message.push_back(0x00); // Nulový bajt na konci
    this->sendDataUDP(message);
  
    this->receiveConfirmation(this->messageID - 1, message); 

}
