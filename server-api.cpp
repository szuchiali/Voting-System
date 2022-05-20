# include <iostream>
# include <fstream>
# include <sstream>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <semaphore.h>
# include <pthread.h>
// use this one to handle all cmds
# include "cmdHandler.cpp"

int portnumber;

int main(int argc, char *argv[]) {

    if(0 != pthread_mutex_init(&existMux, NULL)){
		throw "Failed to initialize a mutex";
	}

    const int PTHREAD_BUFFER_NUM = 10;
    pthread_t* threads = new pthread_t[PTHREAD_BUFFER_NUM];
    std::vector<std::string>* cmdList = new std::vector<std::string>[PTHREAD_BUFFER_NUM];
    int tid = 0;

    bool restart = false; //check if the server is restarted
    //default value for pw and portnumber
    pw = "cit595";
    portnumber = 10000;
    //use the getopt to read the argument for ./server-api and set up the password, portnumber
    int opt;
    while ((opt = getopt(argc, argv, "a:p:r")) != -1) { //-a : password -p :portnumber -r readback();
        switch (opt) {
            case 'a':
                pw = optarg; //set the password
                break;
            case 'p':
                portnumber = std::stoi(optarg); //set the portnumber
                break;
            case 'r':
                restart = true;
                exist = true;
                readback(); // read back the previous state
                break;
           }
    }

    //create the thread, deal with the client command
    // current version, just manually key in the command
    bool first = true;
    while(true){
        //get the input from the user
        std::string input;
		std::cout <<"enter the command: \n"<<std::flush;
		getline(std::cin, input);
        //parser the command
        cmdList[tid] = parseCmd(input, " ");
        if(cmdList[tid].size() < 1) {
            std::cout <<"invalid input with all space!"<<std::endl;
            continue;
        }
        //client command form : "command argv1 argv2"
        //if there is no -r command, first command should be "start_election"
        if(restart == false && first){
            if(cmdList[tid].size()!=2 || cmdList[tid][0].compare("start_election")!= 0 || cmdList[tid][1].compare(pw)!= 0){ //check the command type
                std::cout << "Error" <<std::endl; // sent back client, I guess is some TCP stuff
                continue; // keep looping until get the start_election comment
            }else{ //start with correct command
                first = false;
            }
        }
        //if the command is shutdown
        if(cmdList[tid][0].compare("shutdown") == 0){
            //check the password if the client is administrator
            if(cmdList[tid].size()!=2 || cmdList[tid][1].compare(pw)!= 0){
                std::cout << "Error" <<std::endl; //sent the error message to client
                continue;
            }else{
                break; // break the loop , pthread join and then call the shut down function
            }
		}
		fflush(stdin);

        //create the multi-thread for the command
        pthread_create(&threads[tid], NULL, (void* (*) (void*))handleCmd2, (void*)&cmdList[tid]);
        tid ++;

        if(tid >= PTHREAD_BUFFER_NUM) {
            for (int i = 0; i < PTHREAD_BUFFER_NUM; i++) {		    
                   pthread_join(threads[i],NULL);
            }

            tid = 0;
            delete [] threads;
            delete [] cmdList;
            threads = new pthread_t[PTHREAD_BUFFER_NUM];
            cmdList = new std::vector<std::string>[PTHREAD_BUFFER_NUM];
        }

    }

    //pthread join all the threads
    for (int i = 0; i < tid; i++) {	        
        pthread_join(threads[i],NULL);       
    }
    delete [] cmdList;
    delete [] threads;

    //destroy mutex
    pthread_mutex_destroy(&existMux);

    //after ended all the thread, call the shut down to back up the status
    if(shutdownserver().compare("OK")!=0){ //
        std::cout << "ERROR" << std::endl;
        return -1;
    }else{
        std::cout << "OK" << std::endl;
    }

    return 0; //shut down the server

}
