# include "Candidate.cpp"
# include <netinet/ip.h>
# include <sys/socket.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <vector>
# include <tuple>
# include <map>
# include <string>
# include <set>

// need some variable to represent the global state such as semaphores...
const int MAXNUM_CHAR = 4096;
int sd;
int shutsd;
bool serverend; //check if a client shutdown server
bool checkfirst; //whether we need to check the first command from client (need to check when without -r flag)
std::string pw= ""; //password for the election (can not change)

bool exist; //whether the election exist
std::map <std::string, Candidate* > canlist; // map for candidate list; store <name, Candidate*>
int vote; // count the current vote
std::set<int> allVoters; //set to store all registered voters
std::map<int,int> voted; //map for keeping track of voters and their vote

static pthread_mutex_t existMux;
static pthread_mutex_t canMux;
static pthread_mutex_t votedMux;
static pthread_mutex_t voteMux;
static pthread_mutex_t allVotersMux;

// below 4 func are for administrators
// Szuchia
bool check(std::string func);
std::string start_election(std::string password) ;
std::string end_election(std::string password) ;
std::string add_candidate(std::string password, std::string candidate) ;
std::string update_backup() ;
std::string shutdownserver() ; //should check the password for the arg1 first and then call the shutdown function
std::vector<std::string> get_result() ; // calculate the result

// some function to handle read back if -r flag is detected
// Jiaxin
void readback();

// below 4 func are for voters
std::string add_voter(std::string voterID) ;
std::string vote_for(std::string name, std::string voterID) ;
std::string check_registration_status(std::string voterID) ;
std::string check_voter_status(std::string magicNumber) ;

// below 3 func are used for any voters
// Jiawei
std::string list_candidates() ;
std::string vote_count(std::string name) ;
std::string view_result() ;

// handleCmd used in server
void sendback(int newSd, std::string message, int max);
std::string handleCmd(std::vector<std::string> commands);
void* handleCmd2(std::vector<std::string>* cmdKeywords); //server-api
