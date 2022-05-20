# include <iostream>
# include <fstream>
# include <sstream>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <semaphore.h>
# include <pthread.h>
# include <netinet/ip.h>
# include <sys/socket.h>
# include <unistd.h>
# include <ctime> // ec3 not used
# include <chrono> // ec3
# include <signal.h> // ec5 is disabled in this ec
# include <fcntl.h>
# include <sys/stat.h>
// use this one to handle all cmds
# include "cmdHandler.cpp"

using namespace std;

static pthread_mutex_t shutdownMux;
static pthread_mutex_t checkMux;
std::vector<pthread_t> threads;

// ec 5: signal handler for SIGINT
// void ignoreSigint(int signo){
//
// 	for (int i = 0; i < (int)threads.size(); i++) {
//         pthread_join(threads[i], NULL);
//     }
//
// 	if(shutdownserver() != "OK") {
// 		cout << "backup failed... shutting down..." << endl;
// 		if(!canlist.empty()){
// 	        for (auto i = canlist.begin(); i != canlist.end(); i++){ //free the candidate
// 	            delete i->second;
// 	        }
// 	        canlist.clear();
// 	    }
// 	} else {
// 		cout << "shutting down..." << endl;
// 	}
//
//     exit(0);
// }

void* handleClient(void* arg){
	int newSd = *((int *) arg);
    //create buffer to receive the message
	//receive the command from client,//client command form : "command argv1 argv2"
    std::vector<char> buffer(MAXNUM_CHAR);
    std::string input;
    if(recv(newSd, &buffer[0], buffer.size(), 0) <= 0) {
        //cout << "ERROR: receiving message" << endl;
        shutdown(newSd, SHUT_RDWR);
        pthread_exit(NULL);
    }
    buffer.push_back('\0');
	input.append(buffer.begin(), buffer.end());
	buffer.clear();
    //cout << "Received by client: " << input << endl;

    std::vector<std::string> cmdList;
    //parser the command
    cmdList = parseCmd(input, " ");
    if(cmdList.size() < 1) {
        sendback(newSd, "INVALID INPUT", MAXNUM_CHAR);
        shutdown(newSd, SHUT_RDWR);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&checkMux);
    bool b = checkfirst;
    pthread_mutex_unlock(&checkMux);

    if(b){ //check first command when server argument without -r
        if(cmdList.size()!=2 || cmdList[0].compare("start_election")!= 0 || cmdList[1].compare(pw)!= 0){ //check the command type
            sendback(newSd, "ERROR", MAXNUM_CHAR); // sent back client
            shutdown(newSd, SHUT_RDWR);
            pthread_exit(NULL);
        }else{ //start with correct command
            pthread_mutex_lock(&checkMux);
            checkfirst = false;
            pthread_mutex_unlock(&checkMux);
        }

    }

    //if the command is shutdown
    if(cmdList[0].compare("shutdown") == 0){
        pthread_mutex_lock(&shutdownMux);
        //check the password if the client is administrator
        if(cmdList.size()!=2 || cmdList[1].compare(pw)!= 0 || serverend){
            sendback(newSd,"ERROR", MAXNUM_CHAR); //sent the error message to client
            shutdown(newSd, SHUT_RDWR);
        }else{
            shutdown(sd, SHUT_RDWR);  //unblock the accept in main thread
            serverend = true;
            shutsd = newSd; //back up the sd for shutdown client
        }
        pthread_mutex_unlock(&shutdownMux);
        pthread_exit(NULL);
    }

    std::string response;
    response = handleCmd(cmdList);
    sendback(newSd, response, MAXNUM_CHAR);
    shutdown(newSd, SHUT_RDWR);
    pthread_exit(NULL);

}


int main(int argc, char *argv[]) {

    //initialize mutex
    std::vector<pthread_mutex_t> muxVec = {existMux, canMux, voteMux, votedMux, allVotersMux, checkMux, shutdownMux};
    for(int i = 0; i < (int)muxVec.size(); i++){
        if(0 != pthread_mutex_init(&muxVec[i], NULL)){
            throw "Failed to initialize a mutex";
            return -1;
        }
    }

    //some used variables
    const int PTHREAD_BUFFER_NUM = 10;
    // pthread_t* threads = new pthread_t[PTHREAD_BUFFER_NUM];
	int portnumber = 10000;
    //int* newSd = new int[PTHREAD_BUFFER_NUM];
	std::vector<int> newSd;
    //int index = 0;
    pw = "cit595";
	exist = false;
    checkfirst = true;

    //use the getopt to read the argument for ./server-api and set up the password, portnumber
    int opt;
    while ((opt = getopt(argc, argv, "a:p:r")) != -1) { //-a : password -p :portnumber -r readback();
		if (opt != 'a' && opt != 'p' && opt != 'r') {

			printf("options to choose:\n -a<password>\t set password \n -p<portnumber>\t set portnumber \n -r<filename>.txt\t\t readback from backup.txt by default or a specified file\n");
			return 2;
		}
		switch (opt) {
            case 'a':
                pw = optarg; //set the password
                break;
            case 'p':
                portnumber = std::stoi(optarg); //set the portnumber
                break;
            case 'r':
                checkfirst = false;
                readback(); // read back the previous state and check the election status
                break;
			default:
				cout << "invalid server argument" << endl;
           }
    }

    // prepare for networking by filling out a structure
    struct sockaddr_in st;
    st.sin_family = AF_INET; // use IPv4
    st.sin_addr.s_addr = INADDR_ANY; // bind to the current IP of this machine
    st.sin_port = portnumber; // use this port

    // create a stream socket on top of IPv4; can use 0 for the last argument: UDP is the default for datagram sockets
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd == -1) {
        cout << "ERROR: create socket" << endl;
        shutdown(sd, SHUT_RDWR);
        return -1;
    }
    // attach socket to a port
    if (bind(sd, (const struct sockaddr*) &st, sizeof(st)) == -1){
        cout << "ERROR: bind" << endl;
        shutdown(sd, SHUT_RDWR);
        return -1;
    }
    // announce that we're expecting incoming requests
    if (listen(sd, 10) < 0) {
      cout << "ERROR: listen" << endl;
      shutdown(sd, SHUT_RDWR);
      return -1;
    }

    //create the thread, deal with the client command

	// setup timer for backup purpose
	auto t_start = std::chrono::high_resolution_clock::now();
	auto t_end = std::chrono::high_resolution_clock::now();
	time_t rawtime;
	int BACKUP_INTERVAL = 10;
	// signal(SIGINT, ignoreSigint);

    while(true){
		// ec3 : update the backup.txt File at certain time interval
		t_end = std::chrono::high_resolution_clock::now();
		double elapsed_time_sec = std::chrono::duration<double, ratio<1,1>>(t_end-t_start).count();
		//cout << "time difference " <<((double)(curr - prev) / CLOCKS_PER_SEC)<<endl;
		pthread_mutex_lock(&shutdownMux);
		if (elapsed_time_sec >= BACKUP_INTERVAL) {
			//cout << "enter if block at " <<ctime (&rawtime)<<endl;
			if (update_backup() == "ERROR") {
				cout << "backup failed at " << ctime (&rawtime) << endl;
			}
			else {
				t_start = std::chrono::high_resolution_clock::now();
			}
		}
		pthread_mutex_unlock(&shutdownMux);

        //accept the new connection
        newSd.push_back(accept(sd, NULL, NULL));
        if(newSd.back() < 0) {
            break;
        }
        //create the multi-thread for the command
		pthread_t tid;
		threads.push_back(tid);
        pthread_create(&threads.back(), NULL, handleClient, &newSd.back());
        //index ++;

        if(threads.size() >= PTHREAD_BUFFER_NUM) {
            for (int i = 0; i < (int)threads.size(); i++) {
                pthread_join(threads[i], NULL);
            }
            //index = 0;
            //delete [] newSd;
            //newSd = new int[PTHREAD_BUFFER_NUM];
			newSd.clear();
			threads.clear();
        }
    }

    //pthread join all the threads
    for (int i = 0; i < (int)threads.size(); i++) {
        pthread_join(threads[i], NULL);
    }
    //delete [] newSd;


    if(shutdownserver().compare("OK")!=0){ //
        sendback(shutsd,"ERROR", MAXNUM_CHAR);
        shutdown(shutsd, SHUT_RDWR);
        shutdown(sd, SHUT_RDWR);
        return -1;
    }else{
        sendback(shutsd,"OK", MAXNUM_CHAR);
    }

    //destroy mutex
    for(int i = 0; i < (int)muxVec.size(); i++){
        pthread_mutex_destroy(&muxVec[i]);

    }

    shutdown(shutsd, SHUT_RDWR);
    shutdown(sd, SHUT_RDWR);
    return 0; //shut down the server

}
