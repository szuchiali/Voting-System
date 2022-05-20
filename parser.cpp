#include "parser.hpp"
#include <vector>
#define PRINT 0
using namespace std;
// # include "initializer.h"    // used for initialization

// I suspect we also need to deal with char* later on... then we have to change the logic
// namely, borrow from HW3 parser , delim would be <space>
// currently, it is kind of trival...
std::vector<std::string> parseCmd(const std::string& raw_line, const std::string& delim) {

    // 1. detect if the input is for server or client

    // if it is for server, detect flags and initialize the state

    // if client
    // detect arg2

    // 2.validate the sequence to ensure the output in std::vector<std::string> is correct

    // 3.generate a std::vector<std::string>
    std::vector<std::string> res;
    if (raw_line == "") {
        #if  PRINT
        std::cout << "empty string" <<std::endl;
        #endif
        return res;
    }else {
        char * strs = new char[raw_line.length() + 1] ;
        std::strcpy(strs, raw_line.c_str());

        char * d = new char[delim.length() + 1];
        std::strcpy(d, delim.c_str());

        char *p = std::strtok(strs, d);
        while(p) {
            std::string s = p;
            //remove the '\"'
            if(s[0] =='\"'){
               s = s.substr(1, s.length());
            }
            if(s[s.length()-1] == '\"'){
                s = s.substr(0, s.length()-1);
            }
            res.push_back(s);
            p = std::strtok(NULL, d);
        }
        delete[] strs;
        delete[] d;
        delete[] p;
        return res;
    }


}

std::vector<std::string> parseHttpCmd(const std::string& raw_line ) {
    std::vector<std::string> res;
    size_t found = raw_line.find("GET");
    char* getstrs;
    char* getp;
    char* strs;
    char* p;
    char* operation;
    char* ptemp;
    if (found != std::string::npos) {
        // parse GET
        getstrs = new char[1024] ;
        //cout << "find POST"<<endl;
        std::strcpy(getstrs, raw_line.substr(found+5, 1023).c_str());
        getstrs[1023] = 0;
        getp = std::strtok(getstrs, " ");
        std::string s = getp;
        //cout << "find operation "<<s<<endl;
        size_t qpos = s.find("?");
        if (qpos != std::string::npos && qpos < s.size() - 1) {
            res.push_back(s.substr(0, qpos)); // operation
            res.push_back(s.substr(qpos+1)); // attribute
        } else {
            res.push_back(s);
        }


        // int ctr = 0;
        // while(p) {
        //     std::string s = p;
        //     ctr ++;
        //     res.push_back(s);
        //     if(ctr > 2) break;
        //     p = std::strtok(NULL, " \r\n");
        // }
        delete [] getstrs;

    }else {
        // parse POST
        found = raw_line.find("POST");
        operation = new char[1024] ;
        //cout << "find POST"<<endl;
        std::strcpy(operation, raw_line.substr(found+6, 1023).c_str());
        operation[1023] = 0;
        ptemp = std::strtok(operation, " ");
        std::string s = ptemp;
        //cout << "find operation "<<s<<endl;
        res.push_back(s);

        //cout << "everything great so far parsing verb "<<res.back()<<endl;
        size_t pos = raw_line.find("name=");
        strs = new char[raw_line.length() - pos + 10] ;
        std::strcpy(strs, raw_line.substr(pos).c_str());

        // char * d = new char[delim.length() + 1];
        // std::strcpy(d, delim.c_str());

        p = std::strtok(strs, "\r\n");
        while(p) {
            std::string s = p;

            //remove the '\"'
            // if(s[0] =='\"'){
            //    s = s.substr(1, s.length());
            // }
            // if(s[s.length()-1] == '\"'){
            //     s = s.substr(0, s.length()-1);
            // }
            if ((pos = s.find("name=")) != std::string::npos) {
                //res.push_back(s.substr(pos+5));
                p = std::strtok(NULL, "\r\n");
                s = p;
                //cout << "find element "<<s<<endl;
                res.push_back(s);
            }

            p = std::strtok(NULL, "\r\n");
        }
        //cout << "finished parsing"<<endl;
        delete[] operation;

        delete[] strs;

    }





    return res;
}
