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
#include "config.h"
#include "connect.h"


#define PLAYERS "Players.bin"
using namespace std;

enum PAWN:uint8_t
{
    LIZARD		= 1<<0,
    SPOCK		= 1<<1,
    SCISSORS	= 1<<2,
    PAPER		= 1<<3,
    ROCK		= 1<<4
};

struct Pawn {
    uint8_t self;
    uint8_t winners;
    uint8_t loosers;
};
class Player;
typedef boost::shared_ptr<Player> PlayerPtr;

boost::hash<string> StringHash;
typedef map<string,PlayerPtr> PlayerMap;
typedef map<PAWN, unsigned int> PawnCountMap;
class Player{
public:
    friend class boost::serialization::access;
    Player() {
    }
    Player(string sName, string sPassword):Player(){
        m_sName=sName;
        m_szPassword = StringHash(sPassword);
    }
    string GetName(){
        return m_sName;
    }
    bool isPasswordValid(string sPassword){
        return m_szPassword==StringHash(sPassword);
    }
    bool operator< (const Player &other) const {
        if(m_usWinPercentage==other.m_usWinPercentage)return (m_sName<other.m_sName);
        return m_usWinPercentage < other.m_usWinPercentage;
    }
    float m_usWinPercentage=50.0f;
    PawnCountMap m_mPawnCount={ { LIZARD,0 },{ SPOCK,0 },{ SCISSORS,0 },{ PAPER,0 },{ ROCK,0 } };
    unsigned int m_uiGamesDraw=0;
    
private:
    string m_sName;
    size_t m_szPassword;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & m_sName;
        ar & m_szPassword;
        ar & m_usWinPercentage;
        ar & m_mPawnCount;
    }
    
};

bool operator<(const PlayerPtr p1, const PlayerPtr p2) {
    return *p1<*p2;
};

BOOST_CLASS_VERSION(Player, 1)

PlayerPtr pPlayer;
PlayerPtr pNewPlayer;
list<PlayerPtr> lspPlayers;

void newclienthandle(tcp_connection::pointer new_connection){
    PlayerPtr pPlayer(new Player());
    pNewPlayer= PlayerPtr(new Player());
    if(new_connection->read(*pNewPlayer)<=0){
        pNewPlayer.reset();
        return;
    }
    lspPlayers.push_back(pNewPlayer);
    cout << endl << pNewPlayer->GetName() << " is waiting to play with you!"<<endl;
}

int main()
{
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
    
    PlayerMap mPlayer;
    vector<PlayerPtr> vPlayer;
    string player;
    string password;
    string decision;
    
    
    // Load players form archive
    ifstream ifs(PLAYERS);
    if(ifs.is_open()){
        boost::archive::binary_iarchive ia(ifs);
        ia >> mPlayer;
        ifs.close();
    }
    cout << "Players in the archive:" << endl;
    PlayerMap::iterator it=mPlayer.begin();
    while(it!=mPlayer.end()){
        vPlayer.push_back(it->second);
        ++it;
    }
    sort(vPlayer.begin(),vPlayer.end());
    int count = 1;
    for(int i=vPlayer.size()-1;i>=0;--i){
        cout << count++ <<".\t"<<vPlayer[i]->GetName() << " (" << setprecision(4) << vPlayer[i]->m_usWinPercentage <<"%)"<<endl;
    }
login:
    cout << "Name: ";
    cin >> player;
enterpassword:
    cout << "Password: ";
    cin >> password;
    it=mPlayer.find(player);
    if(it==mPlayer.end()){
        cout << "Proceed with new player with name " << player << " and password " << password << "? [y/n/x(exit)]: ";
        cin >> decision;
        if(decision=="y"){
        createplayer:
            pPlayer = PlayerPtr(new Player(player,password));
            mPlayer[player]=pPlayer;
        }else if(decision=="n"){
            goto login;
        }else{
            return 0;
        }
    }else{
        if(!it->second->isPasswordValid(password)){
            cout << "Invalid Password! retry[r]? create new player with this password? [r/y/n]: ";
            cin >> decision;
            if(decision=="y"){
                goto createplayer;
            }else if(decision=="r"){
                goto enterpassword;
            }else{
                goto login;
            }
        }else{
            pPlayer=it->second;
            
        }
    }
    
createserver:
    boost::asio::io_service io_service;
    boost::shared_ptr<tcp_server> pserver;
    try
    {
        pserver=boost::shared_ptr<tcp_server>(new tcp_server(io_service,&newclienthandle));
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    boost::thread_group thr_grp;
    thr_grp.create_thread(boost::bind(&tcp_server::start_accept, pserver));
    
chooseopponent:
    PlayerPtr pOpponent(new Player());
    string sOpponent;
    cout << "Whom do you wanna play with? [computer/player(name/address)]: ";
    cin >> sOpponent;
    tcp_connection::pointer pConnection;
    if(sOpponent!="computer"){
        list<PlayerPtr>::iterator it = lspPlayers.begin();
        int i=0;
        while( it!=lspPlayers.end() && (*it)->GetName()!=sOpponent){it++;i++;}
        if(it==lspPlayers.end()){
            cout << "connecting to " << sOpponent << endl;
            if(!(pConnection=pserver->connect(sOpponent, to_string(PORT)))){
                cout << "couldn't connect to the opponent. choose again!";
                goto chooseopponent;
            }
            if(pConnection->write(*pPlayer)<=0){
                pConnection.reset();
                cout << "Couldn't connect to "<<sOpponent<<endl;
                goto chooseopponent;
            }
            cout << "waiting for the " << sOpponent << endl;
            if(pConnection->read(*pOpponent)<=0){
                pConnection.reset();
                cout << "Couldn't connect to "<<sOpponent<<endl;
                goto chooseopponent;
            }
            lspPlayers.push_back(pOpponent);
            sOpponent=pOpponent->GetName();
            cout << sOpponent << " connected!" << endl;
        }else{
            sOpponent=(*it)->GetName();
            list<tcp_connection::pointer>::iterator itc=pserver->GetConnections().begin();
            while(i>0){
                itc++;
                i--;
            }
            pConnection=*itc;
            if(pConnection->write(*pPlayer)<=0){
                cout<< "couldn't connect "<<sOpponent << endl;
                pConnection.reset();
                remove(lspPlayers.begin(),lspPlayers.end(),*it);
                goto chooseopponent;
            }
            cout << "connected!" <<endl;
            pOpponent=*it;
        }
    }else{
        pOpponent=mPlayer[sOpponent];
        if(!pOpponent)pOpponent=boost::shared_ptr<Player>(new Player(sOpponent,"1631729"));
    }
    mPlayer[sOpponent]=pOpponent;
    cout << "---------------------------------Good Luck!-----------------------------" << endl;
    static map<PAWN, Pawn> mPawns;
    static map<string, PAWN> msPawn;
    static map<PAWN, string> mpPawn;
    static map<uint8_t, string> mpPawnsAction;
    PawnCountMap mPawnCount= { { LIZARD,0 },{ SPOCK,0 },{ SCISSORS,0 },{ PAPER,0 },{ ROCK,0 } };
    PawnCountMap mOPawnCount = { { LIZARD,0 },{ SPOCK,0 },{ SCISSORS,0 },{ PAPER,0 },{ ROCK,0 } };
    
    mPawns[LIZARD]		= { LIZARD, ROCK | SCISSORS, SPOCK | PAPER };
    mPawns[SPOCK]		= { SPOCK, LIZARD | PAPER, SCISSORS | ROCK };
    mPawns[SCISSORS]	= { SCISSORS, ROCK | SPOCK, PAPER | LIZARD };
    mPawns[PAPER]		= { PAPER, SCISSORS | LIZARD, ROCK | SPOCK };
    mPawns[ROCK]		= { ROCK, PAPER | SPOCK, LIZARD | SCISSORS };
    
    mpPawnsAction[SCISSORS | LIZARD] = "decapitated";
    mpPawnsAction[SCISSORS | SPOCK] = "smashed";
    mpPawnsAction[SCISSORS | PAPER] = "cut";
    mpPawnsAction[SCISSORS | ROCK] = "blunted";
    mpPawnsAction[LIZARD | SPOCK] = "poisoned";
    mpPawnsAction[LIZARD | PAPER] = "eaten";
    mpPawnsAction[LIZARD | ROCK] = "crushed";
    mpPawnsAction[SPOCK | PAPER] = "disproved";
    mpPawnsAction[SPOCK | ROCK] = "vaporized";
    mpPawnsAction[PAPER | ROCK] = "covered";
    
    mpPawn[SPOCK]		= "Spock";
    mpPawn[SCISSORS]	= "Scissors";
    mpPawn[LIZARD]		= "Lizard";
    mpPawn[PAPER]		= "Paper";
    mpPawn[ROCK]		= "Rock";
    
    msPawn["li"]		= LIZARD;
    msPawn["lizard"]	= LIZARD;
    msPawn["spock"]		= SPOCK;
    msPawn["sp"]		= SPOCK;
    msPawn["scissors"]	= SCISSORS;
    msPawn["sc"]		= SCISSORS;
    msPawn["paper"]		= PAPER;
    msPawn["pa"]		= PAPER;
    msPawn["rock"]		= ROCK;
    msPawn["ro"]		= ROCK;
    unsigned int uiTotalGames = 0;
    unsigned int uiGamesWon = 0;
    float uiWinPercentage = 50;
    unsigned int uiGamesDraw = 0;
play:
    while (true) {
        if(!sOpponent.length())goto chooseopponent;
        cout << "Choose your pawn: ";
        cin >> sPawn;
        for (int i = 0; sPawn[i]; i++) sPawn[i] = tolower(sPawn[i]);
        if (msPawn.find(sPawn) == msPawn.end()) {
            goto scoresonexit;
        }
        
        PAWN self = msPawn[sPawn];
        PAWN opponent;
        if(pConnection){
            try{
                if(pConnection->write(self)<=0)throw exception();
                if(pConnection->read(opponent)<=0)throw exception();
            }catch(exception e){
                cout << "connection lost with " << sOpponent << endl;
                remove(lspPlayers.begin(),lspPlayers.end(),pOpponent);
                pConnection.reset();
                sOpponent.clear();
                goto scoresonexit;
            }
        }else{
            opponent = (PAWN)(1 << (rand() % 5));
        }
        ++uiTotalGames;
        mPawnCount[self]++;
        mOPawnCount[opponent]++;
        if (self == opponent) {
            uiGamesDraw++;
            cout << sOpponent <<" also came with " << mpPawn[opponent] << "!\n***----***" << endl;
        }
        else if ((mPawns[self].loosers&opponent) == opponent) {
            ++uiGamesWon;
            cout << "Your " << mpPawn[self] << " has " << mpPawnsAction[self | opponent] << " "<<sOpponent<<"'s " << mpPawn[opponent] << endl <<"***Win!***" << endl;
        }
        else {
            cout << sOpponent<<"'s " << mpPawn[opponent] << " has " << mpPawnsAction[self | opponent] << " your " << mpPawn[self] <<endl<< "***xxxx***" << endl;
        }
        
    }
scoresonexit:
    if(uiTotalGames-uiGamesDraw)uiWinPercentage = (float)uiGamesWon * 100 / (uiTotalGames-uiGamesDraw);
    cout << "You have won " << setprecision(4) << uiWinPercentage << "% of games played" << endl;
    
    // find the number of occassions in which player has made each of the five possible choices
    unsigned int iAllPawnsChoosenCount = 0;
    map<PAWN,unsigned int>::iterator itPawnCount= mPawnCount.begin();
    if (itPawnCount != mPawnCount.end()) {
        iAllPawnsChoosenCount = itPawnCount->second;
        ++itPawnCount;
        while (itPawnCount != mPawnCount.end()) {
            if (iAllPawnsChoosenCount > itPawnCount->second)
                iAllPawnsChoosenCount = itPawnCount->second;
            ++itPawnCount;
        }
    }
    cout << "In " << iAllPawnsChoosenCount << " occasions the player has made each of the five possible choices." << endl;
    
    if (uiWinPercentage == 50.00f) {
        cout << "Its a draw!" << endl;
    }
    else if (uiWinPercentage > 50.00f) {
        cout << "You won!" << endl;
    }
    else {
        cout << "You lost!" << endl;
    }
    
    cout << "saving the scores..." << endl;
    PawnCountMap::iterator itc = pPlayer->m_mPawnCount.begin();
    unsigned int uiNetGamesPlayed=0;
    while(itc!=pPlayer->m_mPawnCount.end()){
        uiNetGamesPlayed += itc->second;
        itc->second+=mPawnCount[itc->first];
        itc++;
    }
    itc = pOpponent->m_mPawnCount.begin();
    unsigned int uiONetGamesPlayed=0;
    while(itc!=pOpponent->m_mPawnCount.end()){
        uiONetGamesPlayed += itc->second;
        itc->second+=mOPawnCount[itc->first];
        itc++;
    }
    unsigned int uiNetGamesWon = pPlayer->m_usWinPercentage*(uiNetGamesPlayed-pPlayer->m_uiGamesDraw) / 100;
    unsigned int uiONetGamesWon = pOpponent->m_usWinPercentage*(uiONetGamesPlayed-pOpponent->m_uiGamesDraw) / 100;
    pPlayer->m_usWinPercentage=(float)(uiNetGamesWon+uiGamesWon)*100/(uiNetGamesPlayed+uiTotalGames-pPlayer->m_uiGamesDraw-uiGamesDraw);
    pOpponent->m_usWinPercentage=(float)(uiONetGamesWon+(uiTotalGames-uiGamesDraw-uiGamesWon))*100/(uiONetGamesPlayed+uiTotalGames-pOpponent->m_uiGamesDraw-uiGamesDraw);
    pPlayer->m_uiGamesDraw += uiGamesDraw;
    pOpponent->m_uiGamesDraw += uiGamesDraw;
    std::ofstream ofs(PLAYERS);
    if(ofs.is_open()){
        boost::archive::binary_oarchive oa(ofs);
        oa << mPlayer;
        ofs.close();
    }
    cout << "Do you want to stop playing? [y/n]: ";
    cin >> sPawn;
    if (tolower(sPawn[0]) == 'n')goto play;
    pPlayer=NULL;
    return 0;
}

