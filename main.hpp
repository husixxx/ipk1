#ifndef maincpp
#define maincpp
#include "client.hpp"

void signalHandler(int signum);
void print_help();
std::vector<std::string> parseInput(std::vector<std::string> inputParts);
void stateStart(IPK24_Client& client, std::vector<std::string> inputParts, std::string command);
void caseAuth(IPK24_Client& client);
void caseAuthUDP(IPK24_Client& client , std::vector<std::string> inputParts);



#endif