// LiSpSciPaRo.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>		// rand
#include <ctime>
#include <iostream>		// cout, cin
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <cstdint>		// uint8_t
#include <iomanip>      // setprecision
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/functional/hash.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <FFJSON.h>
#include <logger.h>
#include "config.h"
#include "connect.h"


#define PLAYERS "Players.bin"
using namespace std;

enum PAWN:uint8_t
{
   LIZARD      = 1<<0,
   SPOCK       = 1<<1,
   SCISSORS    = 1<<2,
   PAPER	      = 1<<3,
   ROCK        = 1<<4
};

struct Pawn_ {
   uint8_t self;
   uint8_t winners;
   uint8_t loosers;
};
class Player_;
typedef boost::shared_ptr<Player_>  PlayerPtr_;

boost::hash<string>                 StringHash;
typedef map<string, PlayerPtr_>     PlayerMap_;
typedef map<PAWN, unsigned int>     PawnCountMap_;
class Player_ {
public:
   friend class boost::serialization::access;
   Player_ () {
   }
   Player_ (string sName, string sPassword) : Player_ () {
      Name = sName;
      PasswordHash = StringHash(sPassword);
   }
   string say_name(){
      return Name;
   }
   bool is_valid_password(string sPassword){
      return PasswordHash==StringHash(sPassword);
   }
   bool operator< (const Player_ &other) const {
      if(WinPercent==other.WinPercent)return (Name<other.Name);
      return WinPercent < other.WinPercent;
   }
   float WinPercent=50.0f;
   PawnCountMap_ PawnCountMap = { { LIZARD, 0 }, { SPOCK, 0 }, { SCISSORS, 0 },
      { PAPER, 0 }, { ROCK, 0 }
   };
   unsigned int GamesDraw = 0;
   
private:
   string Name;
   size_t PasswordHash;
   template<class Archive>
   void serialize(Archive& ar, const unsigned int version) {
      ar & Name;
      ar & PasswordHash;
      ar & WinPercent;
      ar & PawnCountMap;
   }
   
};

static FFJSON ffPlayers("file://Players.json");

bool operator<(const PlayerPtr_ p1, const PlayerPtr_ p2) {
   return *p1<*p2;
};

BOOST_CLASS_VERSION(Player_, 1)

PlayerPtr_        Player_P;
PlayerPtr_        NewPlayer_P;
list<PlayerPtr_>  Players_PLS;

void handle_new_client (tcp_connection::pointer new_connection) {
   PlayerPtr_ pPlayer(new Player_());
   NewPlayer_P= PlayerPtr_(new Player_());
   if(new_connection->read(*NewPlayer_P)<=0){
      NewPlayer_P.reset();
      return;
   }
   Players_PLS.push_back(NewPlayer_P);
   cout << endl << NewPlayer_P->say_name() << " is waiting to play with you!" <<
   endl;
}

int main() {
   string sPawn;
   cout << R"dewed(
   ---------------------------------------
   |     Lizard[Li]  >  Spock[Sp]        |
   |        >  \/       /\   >           |
   | Rock[Ro]     ./ > '\   Scissors[Sc] |
   |           <          <              |
   |             Paper[Pa]               |
   ---------------------------------------
   )dewed" << endl;
   cout << "anything else other than the above will **exit** the game" << endl;
   
   PlayerMap_ Player_M;
   vector<PlayerPtr_> PlayerPtr_V;
   string player;
   string password;
   string decision;
   static map<PAWN, Pawn_>       PawnsEnumMap;
   static map<string, PAWN>      PawnStrEnumMap;
   static map<PAWN, string>      PawnEnumStrMap;
   static map<uint8_t, string>   PawnIdActionMap;
   PawnCountMap_ EachPawnCount = { { LIZARD, 0 }, { SPOCK, 0 }, { SCISSORS, 0 },
      { PAPER, 0 }, { ROCK, 0 }
   };
   PawnCountMap_ EachOppPawnCount = { { LIZARD, 0 }, { SPOCK, 0 },
      { SCISSORS, 0 }, { PAPER, 0 }, { ROCK, 0 }
   };
   
   PawnsEnumMap[LIZARD]		= { LIZARD, ROCK | SCISSORS, SPOCK | PAPER };
   PawnsEnumMap[SPOCK]		= { SPOCK, LIZARD | PAPER, SCISSORS | ROCK };
   PawnsEnumMap[SCISSORS]	= { SCISSORS, ROCK | SPOCK, PAPER | LIZARD };
   PawnsEnumMap[PAPER]		= { PAPER, SCISSORS | LIZARD, ROCK | SPOCK };
   PawnsEnumMap[ROCK]		= { ROCK, PAPER | SPOCK, LIZARD | SCISSORS };
   
   PawnIdActionMap[SCISSORS | LIZARD]  = "decapitated";
   PawnIdActionMap[SCISSORS | SPOCK]   = "smashed";
   PawnIdActionMap[SCISSORS | PAPER]   = "cut";
   PawnIdActionMap[SCISSORS | ROCK]    = "blunted";
   PawnIdActionMap[LIZARD | SPOCK]     = "poisoned";
   PawnIdActionMap[LIZARD | PAPER]     = "eaten";
   PawnIdActionMap[LIZARD | ROCK]      = "crushed";
   PawnIdActionMap[SPOCK | PAPER]      = "disproved";
   PawnIdActionMap[SPOCK | ROCK]       = "vaporized";
   PawnIdActionMap[PAPER | ROCK]       = "covered";
   
   PawnEnumStrMap[SPOCK]      = "Spock";
   PawnEnumStrMap[SCISSORS]   = "Scissors";
   PawnEnumStrMap[LIZARD]     = "Lizard";
   PawnEnumStrMap[PAPER]      = "Paper";
   PawnEnumStrMap[ROCK]	      = "Rock";
   
   PawnStrEnumMap["li"]	      = LIZARD;
   PawnStrEnumMap["lizard"]   = LIZARD;
   PawnStrEnumMap["spock"]	   = SPOCK;
   PawnStrEnumMap["sp"]	      = SPOCK;
   PawnStrEnumMap["scissors"]	= SCISSORS;
   PawnStrEnumMap["sc"]	      = SCISSORS;
   PawnStrEnumMap["paper"]	   = PAPER;
   PawnStrEnumMap["pa"]	      = PAPER;
   PawnStrEnumMap["rock"]     = ROCK;
   PawnStrEnumMap["ro"]       = ROCK;
   
   
   // Load players form archive
   ifstream ifs(PLAYERS);
   if (ifs.is_open()) {
      boost::archive::binary_iarchive ia(ifs);
      ia >> Player_M;
      ifs.close();
   }
   cout << "Players in the archive:" << endl;
   PlayerMap_::iterator it=Player_M.begin();
   while(it!=Player_M.end()){
      PlayerPtr_V.push_back(it->second);
      ++it;
   }
   sort(PlayerPtr_V.begin(),PlayerPtr_V.end());
   int count = 1;
   for (int i=PlayerPtr_V.size()-1;i>=0;--i) {
      cout << count++ << ".\t" << PlayerPtr_V[i]->say_name() << " (" <<
      setprecision(4) << PlayerPtr_V[i]->WinPercent << "%)" << endl;
   }
Login:
   cout << "Name: ";
   cin >> player;
EnterPassword:
   cout << "Password: ";
   cin >> password;
   it = Player_M.find(player);
   if (it == Player_M.end()) {
      cout << "Proceed with new player with name " << player << " and password "
      << password << "? [y/n/x(exit)]: ";
      cin >> decision;
      if (decision == "y") {
      createplayer:
         Player_P = PlayerPtr_(new Player_(player,password));
         Player_M[player]=Player_P;
      }else if(decision=="n"){
         goto Login;
      }else{
         return 0;
      }
   } else {
      if (!it->second->is_valid_password(password)) {
         cout << "Invalid Password! retry[r]? create new player with this password? [r/y/n]: ";
         cin >> decision;
         if(decision=="y"){
            goto createplayer;
         }else if(decision=="r"){
            goto EnterPassword;
         }else{
            goto Login;
         }
      }else{
         Player_P=it->second;
         
      }
   }
   
CreateServer:
   boost::asio::io_service IOService;
   boost::shared_ptr<tcp_server> ServerPtr;
   try
   {
      ServerPtr = boost::shared_ptr<tcp_server>(
         new tcp_server(IOService,&handle_new_client)
      );
   }
   catch (std::exception& e)
   {
      std::cerr << e.what() << std::endl;
   }
   boost::thread_group thr_grp;
   thr_grp.create_thread(boost::bind(&tcp_server::start_accept, ServerPtr));
   
ChooseOpponent:
   PlayerPtr_ OpponentPtr(new Player_());
   string OpponentName;
   cout << "Whom do you wanna play with? [computer/player(name/address)]: ";
   cin >> OpponentName;
   tcp_connection::pointer ConnectionPtr;
   if (OpponentName!="computer") {
      list<PlayerPtr_>::iterator it = Players_PLS.begin();
      int i=0;
      while (it!=Players_PLS.end() && (*it)->say_name()!=OpponentName) {
         ++it;++i;
      }
      if (it==Players_PLS.end()) {
         cout << "connecting to " << OpponentName << endl;
         if (!(ConnectionPtr = 
            ServerPtr->connect(OpponentName, to_string(PORT)))
         ){
            cout << "couldn't connect to the opponent. choose again!";
            goto ChooseOpponent;
         }
         if (ConnectionPtr->write(*Player_P) <= 0) {
            ConnectionPtr.reset();
            cout << "Couldn't connect to " << OpponentName << endl;
            goto ChooseOpponent;
         }
         cout << "waiting for the " << OpponentName << endl;
         if (ConnectionPtr->read(*OpponentPtr) <= 0) {
            ConnectionPtr.reset();
            cout << "Couldn't connect to " << OpponentName <<endl;
            goto ChooseOpponent;
         }
         Players_PLS.push_back(OpponentPtr);
         OpponentName = OpponentPtr->say_name();
         cout << OpponentName << " connected!" << endl;
      } else {
         OpponentName = (*it)->say_name();
         list<tcp_connection::pointer>::iterator itc
         = ServerPtr->GetConnections().begin();
         while (i>0) {
            itc++;
            i--;
         }
         ConnectionPtr = *itc;
         if (ConnectionPtr->write(*Player_P)<=0) {
            cout<< "couldn't connect "<<OpponentName << endl;
            ConnectionPtr.reset();
            remove(Players_PLS.begin(),Players_PLS.end(),*it);
            goto ChooseOpponent;
         }
         cout << "connected!" <<endl;
         OpponentPtr = *it;
      }
   } else {
      OpponentPtr = Player_M[OpponentName];
      if (!OpponentPtr) OpponentPtr=boost::shared_ptr<Player_>(
         new Player_(OpponentName,"1631729")
      );
   }
   Player_M[OpponentName] = OpponentPtr;
   cout << "-----------------------------Good Luck!---------------------------" 
   << endl;
   unsigned int TotalGames = 0;
   unsigned int GamesWon   = 0;
   float WinPercent        = 50;
   unsigned int GamesDraw  = 0;
Play:
   while (true) {
      if (!OpponentName.length()) goto ChooseOpponent;
      cout << "Choose your pawn: ";
      cin >> sPawn;
      for (int i = 0; sPawn[i]; i++) sPawn[i] = tolower(sPawn[i]);
      if (PawnStrEnumMap.find(sPawn) == PawnStrEnumMap.end()) {
         goto ScoresOnExit;
      }
      
      PAWN self = PawnStrEnumMap[sPawn];
      PAWN opponent;
      if(ConnectionPtr){
         try {
            if(ConnectionPtr->write(self)<=0)throw exception();
            if(ConnectionPtr->read(opponent)<=0)throw exception();
         } catch (exception e) {
            cout << "connection lost with " << OpponentName << endl;
            remove(Players_PLS.begin(),Players_PLS.end(),OpponentPtr);
            ConnectionPtr.reset();
            OpponentName.clear();
            goto ScoresOnExit;
         }
      } else {
         opponent = (PAWN)(1 << (rand() % 5));
      }
      ++TotalGames;
      EachPawnCount[self]++;
      EachOppPawnCount[opponent]++;
      if (self == opponent) {
         GamesDraw++;
         cout << OpponentName <<" also came with " << PawnEnumStrMap[opponent] 
         << "!\n***----***" << endl;
      } else if ((PawnsEnumMap[self].loosers&opponent) == opponent) {
         ++GamesWon;
         cout << "Your " << PawnEnumStrMap[self] << " has " << 
         PawnIdActionMap[self | opponent] << " " << OpponentName << "'s " << 
         PawnEnumStrMap[opponent] << endl <<"***Win!***" << endl;
      } else {
         cout << OpponentName<< "'s " << PawnEnumStrMap[opponent] << " has " <<
         PawnIdActionMap[self | opponent] << " your " << PawnEnumStrMap[self] <<
         endl << "***xxxx***" << endl;
      }
      
   }
ScoresOnExit:
   if (TotalGames-GamesDraw) 
      WinPercent = (float)GamesWon * 100 / (TotalGames-GamesDraw);
   cout << "You have won " << setprecision(4) << WinPercent << 
   "% of games played" << endl;
   
   // find the number of occassions in which player has made each of the
   // five possible choices
   unsigned int AllPawnsChoosenCount_I = 0;
   map<PAWN,unsigned int>::iterator PawnCount_Itr = EachPawnCount.begin();
   if (PawnCount_Itr != EachPawnCount.end()) {
      AllPawnsChoosenCount_I = PawnCount_Itr->second;
      ++PawnCount_Itr;
      while (PawnCount_Itr != EachPawnCount.end()) {
         if (AllPawnsChoosenCount_I > PawnCount_Itr->second)
            AllPawnsChoosenCount_I = PawnCount_Itr->second;
         ++PawnCount_Itr;
      }
   }
   cout << "In " << AllPawnsChoosenCount_I << 
   " occasions the player has made each of the five possible choices." << endl;
   
   if (WinPercent == 50.00f) {
      cout << "Its a draw!" << endl;
   }
   else if (WinPercent > 50.00f) {
      cout << "You won!" << endl;
   }
   else {
      cout << "You lost!" << endl;
   }
   
   cout << "Saving the scores..." << endl;
   PawnCountMap_::iterator itc = Player_P->PawnCountMap.begin();
   unsigned int NetGamesPlayed = 0;
   while(itc!=Player_P->PawnCountMap.end()){
      NetGamesPlayed += itc->second;
      itc->second += EachPawnCount[itc->first];
      itc++;
   }
   itc = OpponentPtr->PawnCountMap.begin();
   unsigned int OppNetGamesPlayed = 0;
   while (itc != OpponentPtr->PawnCountMap.end()) {
      OppNetGamesPlayed += itc->second;
      itc->second += EachOppPawnCount[itc->first];
      itc++;
   }
   unsigned int NetGamesWon = 
   Player_P->WinPercent * (NetGamesPlayed-Player_P->GamesDraw) / 100;
   unsigned int OppNetGamesWon = 
   OpponentPtr->WinPercent*(OppNetGamesPlayed-OpponentPtr->GamesDraw) / 100;
   Player_P->WinPercent = (float)(NetGamesWon + GamesWon) * 100 / 
   (NetGamesPlayed + TotalGames - Player_P->GamesDraw - GamesDraw);
   OpponentPtr->WinPercent = 
   (float)(OppNetGamesWon + (TotalGames - GamesDraw - GamesWon)) * 100 / 
   (OppNetGamesPlayed + TotalGames - OpponentPtr->GamesDraw - GamesDraw);
   Player_P->GamesDraw += GamesDraw;
   OpponentPtr->GamesDraw += GamesDraw;
   std::ofstream ofs(PLAYERS);
   if(ofs.is_open()){
      boost::archive::binary_oarchive oa(ofs);
      oa << Player_M;
      ofs.close();
   }
   cout << "Do you want to stop playing? [y/n]: ";
   cin >> sPawn;
   if (tolower(sPawn[0]) == 'n') goto Play;
   Player_P = NULL;
   return 0;
}

