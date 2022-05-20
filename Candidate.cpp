#include "Candidate.hpp"

Candidate::Candidate(const std::string& cname){
    name = cname;
    vote = 0; //initialize as one
}

Candidate::~Candidate(){

}

std::string Candidate::getname(){
    return name;
}

int Candidate::getvote(){
    return vote;
}

void Candidate::setvote(int vote){
    this->vote = vote;
}
