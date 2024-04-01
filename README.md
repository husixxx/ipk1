# IPK Projekt 1: Klient pre chat server používajúci IPK24-CHAT protokol

## Obsah
- [Úvod](#úvod)
- [Architektúra aplikácie](#architektúra-aplikácie)
  - [UML diagramy](#uml-diagramy)
  - [Zaujímavé sekcie zdrojového kódu](#zaujímavé-sekcie-zdrojového-kódu)
- [Testovanie](#testovanie)
  - [Testovacie prostredie](#testovacie-prostredie)
  - [Testovacie prípady](#testovacie-prípady)
- [Extra funkcionality](#extra-funkcionality)
- [Bibliografia](#bibliografia)

## Úvod
Tento dokument obsahuje dokumentáciu k implementácii klienta pre komunikáciu so serverom používajúcim protokol IPK24-CHAT. Protokol má dve varianty založené na transportných protokoloch TCP a UDP. Dokumentácia sa zameriava na funkcionalitu, testovanie a porovnanie s nástrojmi(Wireshark).

## Architektúra aplikácie

### client[.cpp/.hpp]
Tento súbor obsahuje implementáciu triedy `IPK24_Client` . Trieda obsahuje jednotlivé metódy potrebné pri komunikácii so serverom. V jej konštruktori sa overuje správnosť čísla portu a dostupnosť serveru, vytvorí sa nový socket a získa sa adresa, ktora sa uloží pre následnú komunikáciu. Obsahuje metódy pre konštrukciu validných správ, validáciu prijatých správ a pre riešenie vstupu od uživateľa aplikácie. Ukladáju sa v nej informácie ako meno uživateľa `displayName` , aktuálne číslo správy, aktuálny stav interného automatu , znak o prijatej odpovedi a informácie o adrese pre komunikáciu.
### fsm[.cpp/.hpp]
Tento súbor obsahuje triedu `fsm` , ktorá obsahuje rozlišné metódy implementujúce jednotlivé stavy automatu , pre UDP aj TCP variantu stavu.
### main[.cpp/.hpp]
Logika, ktorá je za správaním klienta je implementovaná konečným stavovým automatom, ktorý obsahuje jednotlivé stavy `stateStart` `stateAuth` ` stateOpen` `stateEnd` `stateError`. Klient má rozlišné chovanie pre `UDP` a `TCP` variantu , medzi ktorými rozlišuje podľa vstupných argumentov. Pre každý stav existuje príslušná funkcia podľa daného protokolu, ktorá zabezpečuje spracovanie stavu a rozhoduje o presune do iného stavu , na základe prijatej správy od serveru. Spracovaním stavu sa myslí korektné načítanie vstupu zo `stdin`, korektné zostrojenie `správy` a následné odoslanie. Informáciu o aktuálnom stave interného automatu uchováva trieda client, ktorá ma atribút `Class_State::state` , kde `Class_State` je enum štruktúra, reprezentujúca jednotlivé stavy.
## Začiatok programu
Program vo funkcií `main` začína tým , že spracuje vstupné argumenty a uloží ich hodnoty. Následne vytvorí inštanciu triedy `IPK24_Client` , nastaví stav na počiatočný `Class_State::start` . Načíta prvý riadok zo `stdin` a začne vykonávať smyčku, počas ktorej skáče do jednotlivých stavov. Stavy pre jednotlivé protokoly sú rovnaké, no líší sa ich implementácia.
### Class_State::start
V tomto stave je pre užívateľa možné poslať iba autorizačný príkaz `/auth` inak sa jedná o nesprávnu akciu. Po obdržaní príkazu /auth zvaliduje či zadaný vstup spĺňa požiadavky, rozlíši sa aký protokol sa aktuálne používa a podľa toho sa vytvorí a odošle paket.
### Class_State::auth
#### Udp varianta
Po odoslaní paketu s typom `auth` správne počká na `confirm` správu a následnú `reply` správu , z ktorej zistí či bola autorizácia úspešná. V prípade nesprávnej autorizácie klient umožnuje uživateľovi nový pokus na autorizáciu , až do konca programu . Inak sa klient presúva do stavu `open`.
#### Tcp varianta
Po odoslaní správy s typom `auth` prijme `reply` správu podľa ktorej rozhodne o presuve do stavu `open` , alebo zostavne v stave `auth` , kde dáva uživateľovi ďalší pokus.
### Class_State::open
V tomto stave sa používá na zaistenie neblokujúceho príjmania správ zo soketu a neblokujúceho čakania na správu zo `stdin` funkcia `select` z `libc`. Program beží v smyčke a kontroluje tieto vstupy, väčšiu prioritu má správa zo soketu , validné správy su iba s typom `reply` `msg` , pripadne `confirm` pri UDP. To znamená že pri príchody takejto správy, server ju podľa príslušného protokolu prijme a skontroluje či splňuje tvar správy podľa `IPK24_Chat` protokolu, prípadne na ňu odpovie pri UDP. Pri príchode nevhodnej správy zo soketu, či už zdeformovanej alebo v túto chvíľu nevhodnú ako `auth` alebo `join`, klient sa presúva do stavu `error`.
Keď je soket prázdny a je vstup na `stdin` tak klient načíta vstup a skontroluje jeho konštrukciu. Skontroluje či je zadaný validný príkaz, ak nie je to považované za správu. Pred poslaním sa správa zvaliduje podľa gramatiky/hlavičky daného protokolu. Pri chybnom príkaze alebo pri snahe o poslanie invalidnej správy , užívateľ dostane chybové hlášenie a klient bude pokračovať v ďalšiej prípadnej komunikácii.
Užívateľ ukončuje spojenie iba poslaním signálu `ctrl-c` alebo poslaním `EOF` . Klient validne spracováva obidva spôsoby , tým že prechádza do stavu `bye`
### Class_State::error
V tomto stave sa pošle príslušná správa typu `err` a klient sa ukončí s nenulovou návratovou hodnotou.
### Class_State::bye
V tomto stave klient pošle správu typu `bye` , pri udp počká aj na jej potvrdenie , a úspešne ukončí program, tj. navratová hodnota sa rovná nule.
## Zaujímavé sekcie zdrojového kódu
### Kód realizujúci automat
```
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
```


## Testovanie
Testovanie prebiehalo na operačnom systéme Ubuntu 22.04. Najviac použivaný a najúčinnejší nástroj bol `Wireshark`.
### Testovanie TCP varianty
Správna funkcionalita bola overená využíváním referenčným serverom, avšak len limitovane. Pre podrobnejšiu analýzu a testy bola využitá dummy serverová inštancia vytvorená pomocou netcat s príkazom `nc -l -p 12345`. kde príkaz `-l` slúži na počúvanie na špecifikovanom porte (v tomto prípade 12345) a `-p` špecifikuje port. Tento prístup umožnil simuláciu serverového prostredia a efektívne testovanie TCP komunikácie.
### Testovanie UDP varianty
Okrem referenčného servera bol využitý aj vlastný veľmi primitívny server, čo umožnilo testovanie UDP funkčnosti pod kontrolou. S využitím nástroju `Wireshark` bolo možné sledovať detaily odoslaných a prijatých správ medzi klientom a serverom, overujúc obsah správ a reakcie servera na tieto požiadavky.




## Testovanie funkcionality 

- **Konštrukcia správy:** Overenie , že správy su správne formátované a v súlade s IPK24_chat protokolom
- **Príkaz /rename a ošetrenie vstupu od užívateľa:** Testovanie reakcie na rôzne vstupy pri použití príkazu /rename, vrátane nesprávnych alebo nevalidných vstupov.
- **Generovanie `messageID` pri UDP:** Overenie schopnosti aplikácie generovať jedinečné identifikátory správ pre UDP komunikáciu.
- **Opakovaná autorizácia v stave `auth`:** bol testovaný mechanizmus, ktorý umožňuje opakovanú autentizáciu v prípade, že predchádzajúci pokus o autentizáciu bol neúspešný.
Tento test zabezpečil, že aplikácia môže správne reagovať na neúspešné pokusy o autentizáciu tým, že umožní užívateľovi zopakovať proces autentizácie a čaká na nové reply


## Testovanie hraničných prípadov 
- **Odpovedanie na všetky spŕavy pri UDP s `confirm`:** Aplikácia bola testovaná na schopnosť správne reagovať na prijaté správy s potrebnými potvrdeniami.
- **Časový limit a opakované odosielanie správ:** Testovanie zahrňovalo overenie mechanizmu pre časový limit 250 ms na čakanie na confirm a logiku pre opakované odosielanie správ, maximálne trikrát, v prípade, že confirm nedojde.
- **Zohľadnenie prichádzajúcich správ počas čakania na `confirm`:** Aplikácia bola testovaná na schopnosť spracovať prichádzajúce správy bez negatívneho vplyvu na čakanie na potvrdenie.



## Bibliografia
* c++ referencia: https://en.cppreference.com/w/ cited [2024-03-10]
* Wikipedia, the free encyclopedia[online]: http://en.wikipedia.org/wiki/Pcap cited [2024-03-10] 
* ChatGPT[online]: https://chat.openai.com/ cited [2024-03-10] 
* DemoTCP material[online]: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoTcp cited [2024-03-10] 
* Petr Matoušek, Síťové aplikace a jejich architektura[kniha]: https://www.martinus.sk/629407-sitove-aplikace-a-jejich-architektura/kniha cited [2024-03-10]


