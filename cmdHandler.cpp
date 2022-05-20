# include "cmdHandler.hpp"
# include "parser.cpp"
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


bool check(std::string func){
    bool local;
    pthread_mutex_lock(&existMux);
    local = exist;
    if(exist && func.compare("end") == 0){
        exist = false;
    }else if(!exist && func.compare("start") == 0){
        exist = true;
    }
    pthread_mutex_unlock(&existMux);
    return local;
}


std::string start_election (std::string password){
    if(pw.compare(password)!=0){ //not the administrator
        return "ERROR";
    }
    if(check("start")){ //election already exist
        return "EXIST";
    }else{
        pthread_mutex_lock(&canMux);
        if(!canlist.empty()){
            for (auto i = canlist.begin(); i != canlist.end(); i++){ //free the candidate
                delete i->second;
            }
            canlist.clear(); //clear the map
        }
        pthread_mutex_unlock(&canMux);
        //clear the voters stuff
        pthread_mutex_lock(&votedMux);
        if(!voted.empty()){
            voted.clear();
        }
        pthread_mutex_unlock(&votedMux);

        pthread_mutex_lock(&allVotersMux);
        if(!allVoters.empty()){
            allVoters.clear();
        }
        pthread_mutex_unlock(&allVotersMux);

        pthread_mutex_lock(&voteMux);
        vote = 0;
        pthread_mutex_unlock(&voteMux);
    }
    return "OK";
}

std::vector<std::string> get_result(){
    std::vector<std::string> winner; //store the winner
    int max_vote = 0; //store the max_vote
    std::vector<std::string> res;

    if(canlist.empty()){ //dosen't add any candidate
        res.push_back("No Winner");
        return res;
    }
    for (auto i =canlist.begin(); i != canlist.end(); i++){
        Candidate* person = i->second;
        std::string name= i->first; //get name for candidate
        int vote = person->getvote(); //get vote for candidate
        //std::cout << "name: "<< name << "vote: " << vote <<std::endl;
        if(vote > max_vote){
            winner.clear();//clean the winner vector
            winner.push_back(name); //add the new winner
            max_vote = vote; //set the new max_vote
        }else if((vote > 0) && (vote == max_vote)){
            //add to winner
            winner.push_back(name);
        }
        //add the candidate result to res;
        name.append(":");
        name  = name + std::to_string(vote);
        res.push_back(name);
    }
    //check if there is no winner
    if(winner.size() == 0){
        res.push_back("No Winner");
        return res;
    }
    std::string winnername;
    if(winner.size() > 1){ //check if there is Draw
        //create the string for all winners
        winnername = "Draw:";
        for(int i = 0; i< (int)winner.size(); i++){
            winnername.append(winner[i]);
            if(i!=(int)winner.size()-1){
                winnername.append(", ");
            }
        }
    }else{ // only one winner
        winnername = "Winner:";
        winnername.append(winner[0]);
    }
    res.push_back(winnername);
    return res;
}

std::string end_election(std::string password){
    if(pw.compare(password)!=0){//not the administrator
        return "ERROR";
    }
    std::string res;
    //as long as the selection is ending, other function shuold wait until the end process is done
    if(!check("end")){ // no election
        return "ERROR";
    }else{
        //add map mutex
        pthread_mutex_lock(&canMux);
        std::vector<std::string> res_vector = get_result();
        pthread_mutex_unlock(&canMux);
        // loop over the vector and create the final string
        for(int i = 0; i < (int)res_vector.size(); i++){
            if(i != (int)res_vector.size() -1 ){
                res.append(res_vector[i]);
                res.append("\n");
            }else{
                res.append(res_vector[i]);
            }
        }
    }
    return res;
}

std::string add_candidate(std::string password, std::string candidate){
    if(pw.compare(password)!=0){ //not the administrator
        return "ERROR";
    }
    if(!check("add")){ // no election
        return "ERROR";
    }else{
        pthread_mutex_lock(&canMux);
        for (auto i = canlist.begin(); i != canlist.end(); i++){ //check if already in the list
            std::string name = i->first;
            if(name.compare(candidate) == 0){ //candidate already exist
                pthread_mutex_unlock(&canMux);
                return "EXIST";
            }
        }
        Candidate* newone = new Candidate(candidate);
        canlist.insert(std::pair<std::string, Candidate*>(candidate, newone));
        pthread_mutex_unlock(&canMux);
    }
    return "OK";
}

std::string update_backup() {
    //create the backup.txt
    //open file
    std::fstream newfile;
	newfile.open("backup.txt",std::fstream::out | std::ofstream::trunc); //clear the backup.txt and write
    if(newfile.is_open()){ //write the back up only when there is on going election
        //write the data
        newfile << "exist " << exist <<std::endl;
        newfile << "v " << vote <<std::endl;
        if(!canlist.empty()){
            for (auto i = canlist.begin(); i != canlist.end(); i++){
            std::string name = i->first;
            int can_vote = i->second->getvote();
            newfile << "c " << name << " " << can_vote << std::endl;
            }
        }
        //write the voters
        if(!allVoters.empty()){
            for (auto i = allVoters.begin(); i != allVoters.end(); i++){
                int id = *i;
                newfile << "allvoters " << id << std::endl;
            }
        }
        //write the voted
        if(!voted.empty()){
            for (auto i = voted.begin(); i != voted.end(); i++){
                int id = i->first;
                int vote = i->second;
                newfile << "voted " << id << " " << vote << std::endl;
            }
        }
    }else if(!newfile.is_open()){ //can't open the file
        return "ERROR";
    }
    newfile.flush();
    newfile.close();

    // assert writing
    // rename("backup.tmp", "backup.txt");
    return "OK";
}

std::string shutdownserver(){
    if (update_backup() == "ERROR") return "ERROR";
    //clear out all the stuff (free malloc)
    if(!canlist.empty()){
        for (auto i = canlist.begin(); i != canlist.end(); i++){ //free the candidate
            delete i->second;
        }
        canlist.clear();
    }

    return "OK"; //ready for shut down
}

void readback(){
    
    std::fstream newfile;
    newfile.open("backup.txt",std::ios::in);
    std::string line;
	if (newfile.is_open()){
      while(getline(newfile, line)){
        std::vector<std::string> res = parseCmd(line, " ");
        if(res[0].compare("exist") == 0){
            exist = std::stoi(res[1]);
        }
        if(res[0].compare("v")== 0){
            vote = std::stoi(res[1]);
        }
        if(res[0].compare("c")== 0){
            std::string name = res[1];
            int vote = std::stoi(res[2]);
            Candidate* newone = new Candidate(name);
            newone->setvote(vote);
            canlist.insert(std::pair<std::string, Candidate*>(name, newone));
        }
        if(res[0].compare("allvoters")== 0){
            allVoters.insert(std::stoi(res[1]));
        }
        if(res[0].compare("voted")== 0){
            voted.insert(std::pair<int,int>(std::stoi(res[1]),std::stoi(res[2])));
        }
      }
    }
    newfile.close();
}

std::string add_voter(int voterID){
    //int id = std::stoi(voterID);
    pthread_mutex_lock(&existMux);
    bool e = exist;
    pthread_mutex_unlock(&existMux);
    if(!e){
        return "ERROR";
    }
    if(voterID<1000 || voterID>9999){
        return "ERROR";
    }
    // check if this voter already exists
    pthread_mutex_lock(&allVotersMux);
    auto pos = allVoters.find(voterID);
    pthread_mutex_unlock(&allVotersMux);
    
    if(pos==allVoters.end()){
        pthread_mutex_lock(&allVotersMux);
        allVoters.insert(voterID);
        pthread_mutex_unlock(&allVotersMux);      
        return "OK";
    } else {
       return "EXISTS";
    }
    return "ERROR";
}

//Adds one vote to the total vote count of the candidate referred to by name if the
// voterid has not already voted.
std::string vote_for(std::string name, int voterID){
    pthread_mutex_lock(&existMux);
    bool e = exist;
    pthread_mutex_unlock(&existMux);
    if(!e){
        return "ERROR";
    }
    //int voterid = std::stoi(voterID);
    // check if the id is valid
    pthread_mutex_lock(&allVotersMux);
    auto it1 = allVoters.find(voterID);
    pthread_mutex_unlock(&allVotersMux);
    
    if(voterID<1000 || voterID>9999 ||it1 ==allVoters.end()){
        return "NOTAVOTER";
    }
    
    pthread_mutex_lock(&votedMux);
    auto it2 = voted.find(voterID);
    pthread_mutex_unlock(&votedMux);
    
    if(it2!=voted.end()){
        return "ALREADYVOTED";
    }
    pthread_mutex_lock(&canMux);
    auto it = canlist.find(name);
    pthread_mutex_unlock(&canMux);
    
    int magicn = rand();
    pthread_mutex_lock(&votedMux);
    voted.insert(std::pair<int,int>(voterID,magicn));
    pthread_mutex_unlock(&votedMux);
    
    pthread_mutex_lock(&voteMux);
    vote++;
    pthread_mutex_unlock(&voteMux);
    
    // a new candidate
    if(it==canlist.end()){
        pthread_mutex_lock(&canMux);
        Candidate* cand = new Candidate(name);
        canlist.insert(it,std::pair<std::string,Candidate*>(name,cand));
        canlist.find(name)->second->setvote(1);
        pthread_mutex_unlock(&canMux);
        return "NEW\n"+std::to_string(magicn);
    } else {
        // update votes
        pthread_mutex_lock(&canMux);
        (*it).second->setvote((*it).second->getvote()+ 1);
        pthread_mutex_unlock(&canMux);
        return "EXISTS\n"+std::to_string(magicn);
    }
    return "ERROR";
}

// Checks if the voterid is valid and has been registered. Return “EXISTS” if valid and
// registered, “INVALID” if invalid, “UNREGISTERED” if valid but hasn’t registered
// and “ERROR” if there is any error.
std::string check_registration_status(int voterID){
    //int id = std::stoi(voterID);
    if(voterID<1000 || voterID>9999){
        return "INVALID";
    }
    pthread_mutex_lock(&allVotersMux);
    auto it = allVoters.find(voterID);
    pthread_mutex_unlock(&allVotersMux);
    
    if(it!=allVoters.end()){
        return "EXISTS";
    } else {
        return "UNREGISTERED";
    }
    return "ERROR";
}
// Checks if the voterid has voted. Return “ALREADYVOTED” if voter has voted,
// “CHECKSTATUS” if voter isn’t registered, “UNAUTHORIZED” if magicno doesn’t
// match records and “ERROR” if there is any error.
std::string check_voter_status(int voterID, int magicno){
    
    pthread_mutex_lock(&votedMux);
    auto it = voted.find(voterID);
    pthread_mutex_unlock(&votedMux);
    
    if(it!=voted.end()){
        int magicNumber = it->second;
        if(magicno == magicNumber){
            return "ALREADYVOTED";
        } else {
            return "UNAUTHORIZED";
        }

    } else {
        return "CHECKSTATUS";
    }
    return "ERROR";
}


// Returns a list of candidates for the current election (but not their vote totals).
// Returns an empty list if the election has not started yet. The return format is as follows:
// <candidate1>
// <candidate2>
// ...
// <candidateN>
std::string list_candidates() {
//     std::map <std::string, Candidate*>::iterator it = canlist.begin();

    std::string candidateList;
    std::vector<std::string> can;
    pthread_mutex_lock(&canMux);
    for (auto it = canlist.begin(); it != canlist.end(); it++){
        std::string name = it->first;
        can.push_back(name);
    }
    pthread_mutex_unlock(&canMux);
    
    for(size_t i = 0; i < can.size(); i++){       
        if(i!=can.size()-1){
            candidateList = candidateList + can[i]+ "\n";
        }else{
            candidateList = candidateList + can[i];
        }           
    }
    
//     for (std::pair<std::string, Candidate*> element : canlist) {
//         // Accessing candidate name from element.
//         candidateList = candidateList + element.first + "\n";
//     }
    return candidateList;
}

// Returns the vote total for the candidate in string format referred to by name,
// or “-1” if the candidate isn't in the system or the election hasn’t started yet.
std::string vote_count(std::string name) {

    std::string res;

    pthread_mutex_lock(&canMux);
    auto it = canlist.find(name);
    if (it == canlist.end()) {
        res = "-1";
    }
    else {
        res = std::to_string(it->second->getvote());
    }
    pthread_mutex_unlock(&canMux);


    return res;
}

// Returns the list of candidates with their vote count as well as the winner of the election.
// Return ERROR if the election has not ended or there’s any other error.
// Refer to the e nd_election command for return format.
std::string view_result() {
    std::string res;
    pthread_mutex_lock(&existMux);
    bool e = exist;
    pthread_mutex_unlock(&existMux);
    if(!e) {

        pthread_mutex_lock(&canMux);
        std::vector<std::string> res_vector = get_result();
        pthread_mutex_unlock(&canMux);

        // loop over the vector and create the final string
        for(size_t i = 0; i < res_vector.size(); i++){
            if(i!=res_vector.size()-1){
                res = res + res_vector[i] + "\n";
            }else{
                res = res + res_vector[i];
            }           
        }

    }
    else {
        res = "ERROR";
    }
    return res;
}

void sendback(int newSd, std::string message, int max){
     
    int length = message.length();
    char response[length+1];
    strcpy(response, message.c_str());
    
    int byte_send = send(newSd, response, length+1, 0);
    if( byte_send < length+1) {
        std::cout << "ERROR: sening message" << std::endl;
    }
}

std::string handleCmd(std::vector<std::string> commands){
    std::string response;
    if (commands.size()==1) {
        if(commands[0].compare("list_candidates") == 0) {
            response = list_candidates();
        } else if(commands[0].compare("view_result") == 0) {
            response = view_result();
        }else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else if (commands.size() == 2) {
        if(commands[0].compare("start_election")== 0){
            std::string pw = commands[1];
            response = start_election(pw);
        }
        else if(commands[0].compare("shutdown")== 0){
            response = shutdownserver();
        }
        else if (commands[0].compare("end_election") == 0){
            std::string pw = commands[1];
            response = end_election(pw);
        }
        else if(commands[0].compare("add_voter") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";

            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";

            }
            if (id > 0) {
                response = add_voter(id);
            }
        }
        else if(commands[0].compare("check_registration_status") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }
            if (id > 0) {
                response = check_registration_status(id);
            }
        }
        else if(commands[0].compare("vote_count") == 0) {
            std::string name = commands[1];
            response = vote_count(name);
        } else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else if (commands.size() == 3){
        if (commands[0].compare("add_candidate") == 0) {
            std::string pw = commands[1];
            std::string can = commands[2];
            response = add_candidate(pw,can);
        }
        else if(commands[0].compare("vote_for") == 0) {
            std::string name = commands[1];
            int id = -1;
            try {
                id = std::stoi(commands[2]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }
            if (id > 0) {
                response = vote_for(name,id);
            }
        }
        else if(commands[0].compare("check_voter_status") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }

            int magic = -1;
            try {
                magic = std::stoi(commands[2]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }

            if (id > 0 && magic > 0) {
                response = check_voter_status(id,magic);
            }
        } else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else {
        for (const auto& s : commands) response += (s + " ");
        response = "Unable to parse command: " + response;
    }
    return response;
}

void* handleCmd2(void* cmdKeywords){
    std::vector<std::string> commands = *(std::vector<std::string>*)cmdKeywords;
    std::string response;
    if (commands.size()==1) {
        if(commands[0].compare("list_candidates") == 0) {
            response = list_candidates();
        } else if(commands[0].compare("view_result") == 0) {
            response = view_result();
        }else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else if (commands.size() == 2) {
        if(commands[0].compare("start_election")== 0){
            std::string pw = commands[1];
            response = start_election(pw);
        } 
        else if (commands[0].compare("end_election") == 0){
            std::string pw = commands[1];
            response = end_election(pw);
        }
        else if(commands[0].compare("add_voter") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";

            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";

            }
            if (id > 0) {
                response = add_voter(id);
            }
        }
        else if(commands[0].compare("check_registration_status") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }
            if (id > 0) {
                response = check_registration_status(id);
            }
        }
        else if(commands[0].compare("vote_count") == 0) {
            std::string name = commands[1];
            response = vote_count(name);
        } else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else if (commands.size() == 3){
        if (commands[0].compare("add_candidate") == 0) {
            std::string pw = commands[1];
            std::string can = commands[2];
            response = add_candidate(pw,can);
        }
        else if(commands[0].compare("vote_for") == 0) {
            std::string name = commands[1];
            int id = -1;
            try {
                id = std::stoi(commands[2]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }
            if (id > 0) {
                response = vote_for(name,id);
            }
        }
        else if(commands[0].compare("check_voter_status") == 0) {
            int id = -1;
            try {
                id = std::stoi(commands[1]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }

            int magic = -1;
            try {
                magic = std::stoi(commands[2]);
            } catch (const std::invalid_argument& ia) {
                response = "Invalid argument: ";
            } catch (const std::out_of_range& oor) {
                response = "Out of Range error: ";
            }

            if (id > 0 && magic > 0) {
                response = check_voter_status(id,magic);
            }
        } else{
            for (const auto& s : commands) response += (s + " ");
            response = "Unable to parse command: " + response;
        }
    }
    else {
        for (const auto& s : commands) response += (s + " ");
        response = "Unable to parse command: " + response;
    }

    std::cout << response << std::endl;
    pthread_exit(NULL);
    return NULL;

}
