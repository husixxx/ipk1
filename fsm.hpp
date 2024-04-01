#ifndef fsmcpp
#define fsmcpp
#include "client.hpp"


class FSM {



    public:
    
    /**
     * @brief Handles the start state of the FSM.
     * 
     * This method is responsible for processing the input command and transitioning to the appropriate state.
     * If the command is "/auth", it parses the input parts, checks for validity, and sends an authentication message to the client.
     * If the command is not "/auth", it displays an error message and sets the client state back to "start".
     * 
     * @param client The IPK24_Client object representing the client.
     * @param inputParts The vector of input parts.
     * @param command The input command.
     */
    void stateStart(IPK24_Client& client, std::vector<std::string> inputParts, std::string command);

    /**
     * @brief Parses the input parts to extract the username, secret, and display name.
     * 
     * This method takes the input parts vector and extracts the username, secret, and display name based on the expected format.
     * If the input parts are invalid or incomplete, an error message is displayed.
     * 
     * @param inputParts The vector of input parts.
     * @return A vector containing the username, secret, and display name.
     */
    std::vector<std::string> parseInput(std::vector<std::string> inputParts);


    /**
     * @brief Handles the authentication state for TCP connections.
     * 
     * This method is responsible for handling the authentication process for TCP connections.
     * It sends an authentication message to the client and transitions to the "auth" state.
     * 
     * @param client The IPK24_Client object representing the client.
     */
    void stateAuthTCP(IPK24_Client& client);

    /**
     * @brief Handles the authentication state for UDP connections.
     * 
     * This method is responsible for handling the authentication process for UDP connections.
     * It sends an authentication message to the client and transitions to the "auth" state.
     * 
     * @param client The IPK24_Client object representing the client.
     * @param inputParts The vector of input parts.
     */
    void stateAuthUDP(IPK24_Client& client, std::vector<std::string> inputParts);

};
#endif