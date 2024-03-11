#include "client.hpp"

class fsm {
public:
    IPK24_Client client;

    // Konstruktor, který přijímá klienta jako parametr a nastaví ho do interní property
    fsm(IPK24_Client& client): client(client) {}

    // Příklad veřejné metody, která může volat metodu na klientovi
    void caseAuthTCP();
    void caseAuthUDP();

    void caseStartUDP(std::vector<std::string> inputParts , std::string command);
    void caseStartTCP(std::vector<std::string> inputParts , std::string command);

    // Další metody, které chcete vystavit pro manipulaci s klientem
};