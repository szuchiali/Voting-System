#include <string>
#include <semaphore.h>

class Candidate{
    private:
        std::string name;
        int vote;
        sem_t* sem;
    public:
        Candidate(const std::string& name);
        ~Candidate();
        std::string getname();
        int getvote();
        void setvote(int vote);

};
