///////////////////
////MICHAL VASKO///
////xvasko14///////
////ISA Projekt////
////IRC BOT////////
///////////////////


#ifndef PARSE
#define PARSE

#define _BSD_SOURCE
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <regex>

class parser
{
public:
	bool parse_options(); 
	parser();
	parser(int, char **);
	void ukaz_premene();
	int irc_creator();
	int najdi_kluce(char *buf);
	std::string *zbierka_klucov();
	void today(char *buf);
	void posielanie_spravy (char *buf);
	void posielanie_spravy_join (char *buf);
	
	
	~parser();
private:
	int aktualny_parameter;
	int pocet_parametrov;
	char ** argumenty;
	int sockfd;

	std::vector<std::string> nespracovane_param;
	bool flags [5]={false,}; //vsetky argumenty
	const std::string options[5] = {"-s", "-h","--help", "-l",""}; // -s -h -l 
	bool resolve(std::string); // -s syslog server
	void show_help(); // help
	int parse_parameters(); // parsovanie aprametrov
	void host_name();//host
	int sprava_na_syslog(std::string irc_msg); //psoeilanie spravy na syslog
	

	std::vector<std::string> slova;
	std::vector<std::string> ulozenie;

	std::string namehost; //IRC server 
    int IRC_port; // IRC port
    std::string channel_irc; // IRC channel
    unsigned int velkost_klucov; // number of keys.
    std::string hostname; //SYSLOG server
    std::string *kluce;
    std::string irc_msg;

	std::string highlight;
	std::string irc_sprava_pars();

};



const unsigned int DEAFULT_IRC_PORT = 6667; // default irc port.
const unsigned int SYSLOG_PORT = 514; // default syslog port.
const int MAX_BUFFER=4096; // buffer size for connection
std::string localhostname();




#endif