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

    
    printf("INFO: Server socket: %s : %d \n",
        inet_ntoa(server_address.sin_addr),
        ntohs(server_address.sin_port));

    this->address = (struct sockaddr *)&this->server_address;
    
    printf("Client created\n");
    
}

void IPK24_Client::closeConnection(){
    close(this->sock_fd);
    printf("Socket closed\n");
}

void IPK24_Client::recieveData(){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    std::array<char, 1024> buffer{0};
    int bytes_rx = recvfrom(this->sock_fd, buffer.data(), 1024, flags, this->address, (socklen_t*)&address_len);
    if(bytes_rx == -1){
        printf("Error receiving data\n");
        exit(1);
    }
    printf("Data received\n");
    printf("Data: %s\n", buffer.data());
}

void IPK24_Client::sendData(std::string message){
    int address_len = sizeof(this->server_address);
    int flags = 0;
    int bytes_tx = sendto(this->sock_fd, message.c_str(), message.size(), flags, this->address, address_len);
    if(bytes_tx == -1){
        printf("Error sending data\n");
        exit(1);
    }
    printf("Data sent\n");
}

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int port = std::atoi(argv[2]);
    std::string ip = argv[1];

    IPK24_Client client(port, ip, SOCK_DGRAM);

    std::string msg;
    while (std::getline(std::cin, msg)){
        client.sendData(msg);
        
    }
    
    client.closeConnection();



    
    
    return 0;

}