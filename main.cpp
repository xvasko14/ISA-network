///////////////////
////MICHAL VASKO///
////xvasko14///////
////ISA Projekt////
////IRC BOT////////
///////////////////


#include "main.h"

using namespace std;

parser::parser(){}
parser::parser(int argc,char ** argv):pocet_parametrov(argc),argumenty(argv){}
parser::~parser(){}


// main - volam funkcie ktore vlastne riadia cely program

int main(int argc, char *argv[])
{
  //spracovanie argumentov
parser spracovanie_argumentov(argc,argv);
if(!spracovanie_argumentov.parse_options()){
  return 1 ;
}
//volame na to aby sme sa mohli pripajat na hexchat napr
spracovanie_argumentov.irc_creator();
return 0;
}

// parsovanie prepinacov
// postupne zistujeme co ide za jednotlivymi prepinacmi a ukladam si to
bool parser::parse_options(){

   
  for(int j = 1;j<pocet_parametrov;j++){
    std::string option = argumenty[j];
    aktualny_parameter = 4;
    for (int i =0 ; i<5 ; i++){
      if(option == options[i]){
        flags [i] = true;
        aktualny_parameter = i;
      }
    }
    
    //prepinac -s a zistujeme co ide za nim
    if(option == "-s") {
      ++j;

      if(j==pocet_parametrov){
        std::cerr<< "nedostatok parametr -s.\n";
        return false;
      }
      option=argumenty[j];
      
      if(!this->resolve(option))
      {
        std::cerr<<"nebolo zadane syslog.\n";
        return false;
      }
      hostname=option;
      //break;
    }
    // prepinac -l a zistujeme co ide za nim
    if( option == "-l"){
      ++j;
      if(j==pocet_parametrov){
        std::cerr<< "nedostatok parametr -l.\n";
        return false;
      }
      
      this->highlight = argumenty[j];
      kluce = zbierka_klucov();

    }
    if(!flags[aktualny_parameter]) {
            // push back vlozi do vektoru nespracovany parameter
      nespracovane_param.push_back(option);
    }

  
    }

    if(flags [1] || flags[2]){
      this->show_help();
    }
    else {
      //zly pocet parametrov tak Error
       if (nespracovane_param.size() != 2) 
      {
        std::cerr << "Error !" << std::endl;
        exit(2);
      } 
      this->parse_parameters();
      


        }

   return true;
}
/// help
void parser::show_help(){
   std::cout 
        << "isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help] "<< std::endl
        << "HOST je název serveru (např. irc.freenode.net)" << std::endl 
        << "PORT je číslo portu, na kterém server naslouchá (výchozí 6667)" << std::endl
        << "CHANNELS obsahuje název jednoho či více kanálů, na které se klient připojí (název kanálu je zadán včetně " << std::endl
        << "úvodního # nebo &; v případě více kanálů jsou tyto odděleny čárkou)" << std::endl
        << "-s SYSLOG_SERVER je ip adresa logovacího (SYSLOG) serveru" << std::endl
        << "-l HIGHLIGHT seznam klíčových slov oddělených čárkou (např.ip,tcp,udp,isa)" << std::endl;
}

// prepinac -s 
// funckia na syslog server
bool parser::resolve(std::string arg){
  struct hostent *hostinfo;
   hostinfo = gethostbyname(arg.c_str());
   //ak prazdny tak chyba
   if(hostinfo == NULL) {
    std::cerr << "Zle Parametre SYSLOG\n";
    return false;
   }
   return true;
}

// funkcia ?msg
// posielame spravu ludom ktory su prave prihalseny na chate
void parser::posielanie_spravy (char *buf) {
std::string stringBuf(buf);
 if(stringBuf.find("?msg") != std::string::npos) {
  std::string msg;
//vyprsujem si buffer
 char nick[32], user[32], server[32], channel[32], mesidz[256], meno[256], sprava[512];

sscanf(buf, ":%31[^!]!~%31[^@]@%31s PRIVMSG #", 
                 nick,     user, server 
                 );
//robim kvoli tomu aby som nemusel parsovat znova ale az od PRIVMSG
char* position = strstr(buf, "PRIVMSG #");
position += 8;
sscanf(position, "%31s :%31s %31[^:]:%255[^\n] ", 
                 channel, mesidz , meno , sprava 
                 );

  // mena ludi ktory su na chate
  msg = "NAMES ";
  msg += std::string(channel);
  msg += "\r\n";
   //posielam
  if(send(sockfd, msg.c_str(), msg.length(), 0) == -1)  
    { 
      std::cerr << "Error with message!" << std::endl;
      exit(2);
    }
   int numbytes; 
   numbytes =recv(sockfd, buf, MAX_BUFFER -1, 0);
   buf[numbytes]='\0';


   stringBuf = buf;
   //ak najdem to meno ktoremu chcem poslat tak poslem
   if(stringBuf.find(meno) != std::string::npos) 
   {

      msg = "PRIVMSG ";
      msg += std::string(channel) ;
      msg += " :" ;
      msg += meno;
      msg += ":" ;
      msg += sprava;
      msg += "\r\n";

      if(send(sockfd, msg.c_str(), msg.length(), 0) == -1) 
      { 

        std::cerr << "Error with message!" << std::endl;
        exit(2);
      }
   }
   // inac si ulozim meno a spravu a idem dalej
   else 
      {
        ulozenie.push_back(std::string(meno) + ':' + std::string(sprava));
    }
  }
}

//funkci ?msg
// toto posle cloveku az ked sa prihlasi na join a ma dostat spravu
void parser::posielanie_spravy_join (char *buf) {
    std::string stringBuf(buf);
    std::string msg;
    // porovname ci sa dakto prihlasil
    if(stringBuf.find("JOIN") != std::string::npos) {
         //znova parsujem
         char nick[32], user[32], server[32], channel[32];
          sscanf(buf, ":%31[^!]!~%31[^@]@%31s PRIVMSG #", 
                 nick,     user, server 
                 );

          char* position = strstr(buf, "JOIN #");
          position += 5;
          sscanf(position, "%255[^\r\n] ", 
                           channel 
                           );
          // robim si cyklus nato aby som prehladal nicky a nasledne odoslal spravu ktoru ma dostat dany clovek
          for(unsigned int i = 0; i< ulozenie.size(); i++ ) {
            if (ulozenie[i].find(std::string(nick) + ':') != std::string::npos) {

              
                msg = "PRIVMSG ";
                msg += std::string(channel) ;
                msg += " :" ;
                msg += ulozenie[i];
                msg += "\r\n";
              
                ulozenie.erase(ulozenie.begin()+i);
                i--;
                if(send(sockfd, msg.c_str(), msg.length(), 0) == -1)
                { 
               
                  std::cerr << "Error with message!" << std::endl;
                  exit(2);
                }
                sleep(2);
                }

          }


     }

}





//funkcia ?today 
//posiela aktualny datum dna
void parser::today(char *buf){
 std::string prikaz = "";
 std::string stringBuf(buf);
 //ak sa zada ?today
 if(stringBuf.find("?today") != std::string::npos)  {

  char channel[32];
  char* position = strstr(buf, "PRIVMSG #");
          position += 8;
          sscanf(position, "%255[^ ] ", 
                           channel
                           );
      

    

  // vyparsovanie datumu
   time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d.%m.%Y",timeinfo);
  std::string str(buffer);

  // uvodzovky kvoli medzere na datum 
  prikaz = "PRIVMSG " + std::string(channel) +" "+ str + "\r\n";

  std::strcpy(buf,prikaz.c_str());
  // poslem data
  if(send(sockfd, prikaz.c_str(), prikaz.length(), 0) == -1)  //send info about user to server
    { 
      std::cerr << "Error can't send message!" << std::endl;
      exit(2);
    }
  }
}

// funkcia na parsovanie parametrov
// CHANNEL A HOST
int parser::parse_parameters()
{

   for (unsigned i = 0; i<nespracovane_param.size();i++){
 
    // parsovanie hostu
    std::string temp_string = std::string(nespracovane_param[i]);  
    std::size_t fnd=temp_string.find(":");

    //parsovanie channelu
    if (temp_string.find("#") != std::string::npos) {
      channel_irc = temp_string;
     
      continue;
    }

    //HOST ak dvojbodka
    if (temp_string.find(":") != std::string::npos)
    {
      std::string num_port = "";
        num_port = (temp_string.substr(fnd+1));
        if (!num_port.empty())
          IRC_port = std::atoi(num_port.c_str());
        // ak zadame iba dvojbodku tak da defaultny
        else
          IRC_port = DEAFULT_IRC_PORT;
        temp_string = temp_string.substr(0,fnd);
        namehost = temp_string;
        continue;
    } else {
      namehost = temp_string;
      IRC_port = DEAFULT_IRC_PORT; 

    }
  } 

    return 0;
}


// funkcia na pripojenie, posielanie sprav atd...teda komunikaciu s IRC serverom
int parser::irc_creator()
{
    struct hostent *hPtr;
    struct sockaddr_in remoteSocketInfo;
    char buf[MAX_BUFFER];
    std::string prikaz = "";
    // pomocna pre buffer
    int numbytes;
    memset(buf, 0, MAX_BUFFER);
    
    // snaziem sa ziskat IP adresu socketu
    if((hPtr = gethostbyname(namehost.c_str())) == NULL)
    {
        std::cerr << "DNS PROBLEM.\n";
        exit(2);
    }

    
    // vytvorim socket.
    if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        close(sockfd);
        std::cerr << "Socket creating problem. \n";
        exit(2);
    }

    // nacitam data do socket struktury
    memcpy((char *)&remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
    remoteSocketInfo.sin_family = AF_INET;
    remoteSocketInfo.sin_port = htons((u_short)IRC_port);      // Set port number

    // pripojim sa na socket
    if(connect(sockfd, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0)
    {
        std::cerr << "Socket creating problem. \n";
        close(sockfd);
        exit(2);
    }

    
    
    // zadanie mojho nicku ktory sa tam pripoi ako bot
    prikaz = "NICK xvasko14\r\n";
    std::strcpy(buf,prikaz.c_str());
    if(send(sockfd, buf, strlen(buf), 0) == -1) //send NICK to server
    {
      std::cerr << "Error with message!" << std::endl;
      exit(2);
    }

    prikaz = "USER xvasko14 xvasko14 " + namehost + " :Michal Vasko\r\n"; 
    std::strcpy(buf,prikaz.c_str());
    if(send(sockfd, buf, strlen(buf), 0) == -1)  //send info about user to server
    { 
      std::cerr << "Error with message!" << std::endl;
      exit(2);
    }

     do
    {
      //robime to pre buffer, aby mi to mohlo nacitat bez chyb
      numbytes =recv(sockfd, buf, MAX_BUFFER -1, 0);
      buf[numbytes]='\0';
    if(buf!= NULL){
      if (strstr(buf,"End of /MOTD command.") != NULL )
            break;
    }

    }while(1);

    prikaz = "JOIN " + channel_irc + "\r\n";  
    std::strcpy(buf,prikaz.c_str());
    if(send(sockfd, buf, strlen(buf), 0) == -1) // send name of channel.
    {
      std::cerr << "Error with message!" << std::endl;
      exit(2);
    }
    

     do
    {   
        //robime to pre buffer, aby mi to mohlo nacitat bez chyb
        numbytes =recv(sockfd, buf, MAX_BUFFER -1, 0);
        buf[numbytes]='\0';
    if(buf!= NULL){
      if (strstr(buf,"End of /NAMES list.\r\n") != NULL )
          break;
        
      }
    
    }while(1);

    do
    {   

        numbytes = recv(sockfd, buf, MAX_BUFFER, 0);
        buf[numbytes]='\0';
         
    if(buf== NULL)  
      continue;
    if ((strstr(buf,"PRIVMSG") != NULL) || (strstr(buf,"NOTICE") != NULL) ) // if come some PRIVMSG or NOTICE msg test if there is some keys.
    {
      najdi_kluce(buf);
    }
    //PING posel server a ja mu POSLEM PONG ako odozvu
    if (strstr(buf,"PING") != NULL ) // if come ping msg send pong.
    {
      prikaz = "PONG \r\n"; 
            std::strcpy(buf,prikaz.c_str());
            if(send(sockfd, buf, strlen(buf), 0) == -1)
      {
          std::cerr << "Error with message!" << std::endl;
    exit(2);
      }
    }
    // Tu volam vsetky potrebne funkcia na posielanie sprav a vypis datumu
    today(buf);
    posielanie_spravy(buf);
    posielanie_spravy_join(buf);
    }while(1);  
    
    
    return 0;  

}


// FUNKCIE K SYSLOGU
// Dostanem list klucov 
std::string *parser::zbierka_klucov()
{
  // predam mu higlight ktore idu po -l
  std::string temp_string = highlight;
  std::size_t n = std::count(temp_string.begin(), temp_string.end(), ',');
  std::string *kluce = new std::string[n+1];
  std::size_t counter = 0; 
  while (counter < n)
  {
      std::size_t pozicia=temp_string.find(",");
      kluce[counter] = temp_string.substr(0,pozicia);
      temp_string.erase(0,pozicia+1);
            counter++;
  }
  // posledny kluc
  kluce[counter] = temp_string; 
  velkost_klucov = counter;
  // temp_string posledny znak
  


  return kluce;

}

// funkcia ktora hlada kluce v buffry, teda ak niekto nieco napisal porovnava sa
int parser::najdi_kluce(char *buf)
{
    irc_msg=std::string(buf);
    std::size_t pozicia = 0;
    std::string temp_string = std::string(buf);
    unsigned int n=0;

    
    pozicia=temp_string.find("\r");
    temp_string= temp_string.substr(0,pozicia);
    
    while(n<=velkost_klucov)
    {
          if (strstr(temp_string.c_str(),kluce[n].c_str()) != NULL )
            if (sprava_na_syslog(irc_sprava_pars())==2)
             {break;}
            n++;
            
    }
    return 0;
}
//Vyparsovanie IRC spravy ktoru odosielame na syslog
std::string parser::irc_sprava_pars()
{
      

      std::time_t vyslednyC = std::time(NULL);
      std::string time;
      std::string syslog_msg;
      std::size_t pozicia=irc_msg.find("!");
      
      time = std::asctime(std::localtime(&vyslednyC));
      time = time.erase(19,27);
      time = time.substr(3,strlen(time.c_str()));
      
      syslog_msg = irc_msg.substr(0,pozicia);
      syslog_msg = syslog_msg.append(": ");
      
      pozicia=irc_msg.find(" :");
      irc_msg = irc_msg.erase(0,pozicia+2);
      syslog_msg.append(irc_msg.c_str());
      syslog_msg.erase(0,1);

      pozicia=syslog_msg.find(":");
      unsigned long p;
      while((p=syslog_msg.find('\n')) != std::string::npos){
        syslog_msg.erase(p);
      }
      while((p=syslog_msg.find('\r')) != std::string::npos){
        syslog_msg.erase(p);
      }
       
       // vysledne ako sa posiela sprava
      return "<134>" + time + " " + localhostname() + " isabot "+ syslog_msg.c_str();
      
}
// posle irc spravu syslogu
int parser::sprava_na_syslog(std::string sys_log_msg)
{
  int socksys=0;
  struct sockaddr_in servaddr;
  struct hostent *hPtr;
  
  socksys=socket(AF_INET,SOCK_DGRAM,0);
  
  // dostanem ip adresu syslog serveru
  if((hPtr = gethostbyname(hostname.c_str())) == NULL)
  {
      std::cerr << "DNS Error." << std::endl;
      exit(2);
  }
  //nacitam data
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr=inet_addr(hostname.c_str());
  servaddr.sin_port=htons(SYSLOG_PORT);
  //poslem
  sendto(socksys,sys_log_msg.c_str(),strlen(sys_log_msg.c_str()),0, (struct sockaddr *)&servaddr,sizeof(servaddr));
  
  // zavriem socket
  close (socksys);
  return 0;
}
// dostanem hostname localneho PC(mojho)
std::string localhostname()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            if ((ifa->ifa_flags & (IFF_LOOPBACK)))
                continue;
            else
            {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                addr = inet_ntoa(sa->sin_addr);
                break;
            }
        }
    }

    freeifaddrs(ifap);
    return addr;
}



