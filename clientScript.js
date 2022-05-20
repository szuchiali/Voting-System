
var sAddr = "127.0.0.1";
var sPort = "10000";
var pswd = "cit595";
const TTL = 6000;

function updateServerConfig(addr, port) {
    sAddr = addr !== undefined? addr: "127.0.0.1";
    sPort = port !== undefined? port: "10000";
    document.getElementById('server').innerHTML = 'server info set!';
    console.log("new sAddr is "+pswd+", new port is set to "+sPort);
    setTimeout(()=>{
        document.getElementById('server').innerHTML = "";
    }, 5000);
}

function updatePassword(newPswd) {
    pswd = newPswd !== undefined? newPswd: "cit595";
    document.getElementById('setpw').innerHTML = 'password set!';
    console.log("new psw is "+pswd);
    setTimeout(()=>{
        document.getElementById('server').innerHTML = "";
    }, 5000);
}

function startElection() {
    let url = "http://"+ sAddr + ":" + sPort + "/start_election";
    const myForm = new FormData();
    myForm.append("password", pswd);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        if (!res.ok) {
            console.log("response error happens!");
        }
        return res.text();
    }).then(text => {
        document.getElementById("start").innerHTML = text;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("start").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('start').innerHTML = "";
        }, 9000);
    })
    setTimeout(()=>{
        document.getElementById('start').innerHTML = "";
    }, 5000);
}

function addCandidate(name) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/add_candidate";
    const myForm = new FormData();
    myForm.append("password", pswd);
    myForm.append("candidateName", name);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        document.getElementById("addCan").innerHTML = text;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("addCan").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('addCan').innerHTML = "";
        }, 9000);
    })
}

function shutdownServer() {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/shutdown";
    const myForm = new FormData();
    myForm.append("password", pswd);

    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        document.getElementById("shut").innerHTML += text;
        document.getElementById("end").innerHTML = "";
        document.getElementById("addvoter").innerHTML = "";
        document.getElementById("votefor").innerHTML = "";
        document.getElementById("checkreg").innerHTML = "";
        document.getElementById("checkvoter").innerHTML = "";
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("shut").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('shut').innerHTML = "";
        }, 9000);
    })
    setTimeout(()=>{
        document.getElementById('shut').innerHTML = "";
    }, 5000);
}


function endElection() {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/end_election";
    const myForm = new FormData();
    myForm.append("password", pswd);

    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        console.log(text);
        let rlist = text.split("\n");
        console.log(rlist);
        let domList = "<ul>";
        rlist.forEach((row) => {
            domList += "<li>" + row + "</li>";
        });
        domList += "</ul>";
        document.getElementById("end").innerHTML = domList;
        document.getElementById("votecount").innerHTML = "";
        document.getElementById("votefor").innerHTML = "";
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("end").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('end').innerHTML = "";
        }, 9000);
    })
    setTimeout(()=>{
        document.getElementById('shut').innerHTML = "";
    }, 10000);
}

function addVoter(voterID) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/add_voter";
    const myForm = new FormData();

    myForm.append("voterID", voterID);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        document.getElementById("addvoter").innerHTML = text;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("addvoter").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('addvoter').innerHTML = "";
        }, 9000);
    })
}

function voteFor(candidateName, voterID) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/vote_for";
    const myForm = new FormData();
    //document.getElementById("votefor").innerHTML = "";
    let domRes = "";
    myForm.append("candidateName", candidateName);
    myForm.append("voterID", voterID);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        console.log(text);
        let newDom = "";
        if (text.includes("EXISTS") || text.includes("NEW")) {
            newDom = text.includes("EXISTS")? voterID +
            " has successfully voted for an existing candidate named "+
                candidateName +" ; Magic number for "+voterID+": "+text.substr(7):
                voterID + " has successfully voted for a new candidate named "+
                    candidateName +" ; Magic number for "+voterID+": "+text.substr(4);
        } else {
            newDom = text.includes("NOTAVOTER")? voterID+" is not a legal voter!" :
                text.includes("ALREADYVOTED")? voterID+" already voted!": "";

        }
        if (text.includes("ERROR")) {
            document.getElementById("voteforError").innerHTML =
                "<p>Invalid argument or election has end</p>";
            setTimeout(()=>{
                document.getElementById('voteforError').innerHTML = "";
            }, 5000);
        }
        console.log(newDom);
        domRes += newDom !== ""? "<ul><li>"+ newDom + "</li></ul>" : "" ;


        document.getElementById("votefor").innerHTML += domRes;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("votefor").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('votefor').innerHTML = "";
        }, 9000);
    })
}

function checkRegStatus(voterID) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/check_registration_status";
    const myForm = new FormData();
    document.getElementById("checkreg").innerHTML = ""
    myForm.append("voterID", voterID);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        document.getElementById("checkreg").innerHTML = text;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("checkreg").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('checkreg').innerHTML = "";
        }, 9000);
    })
}

function checkVoterStatus(voterID, magicNumber) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/check_voter_status";
    const myForm = new FormData();
    let domRes = "";
    document.getElementById("checkvoter").innerHTML = ""
    myForm.append("voterID", voterID);
    myForm.append("magicNumber", magicNumber);
    const myInit = {
        method: 'POST',
        timeout: TTL,
        body: myForm
    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        domRes += "<ul><li>state: "+ text + "</li></ul>";
        document.getElementById("checkvoter").innerHTML = domRes;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("checkvoter").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('checkvoter').innerHTML = "";
        }, 9000);
    })
}

function voteCount(candidateName) {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/vote_count?"+ candidateName;
    let domList = "";
    document.getElementById("votecount").innerHTML = "";
    const myInit = {
        method: 'GET',
        timeout: TTL,

    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        console.log(text);
        let newDom = text === "-1" ? "<ul><li>"+candidateName +" is not a candidate</li></ul>" :
            "<ul><li>"+candidateName+" has vote count : "+text + "</li></ul>";
        domList += newDom;
        document.getElementById("votecount").innerHTML += domList;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("votecount").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('votecount').innerHTML = "";
        }, 9000);
    });

}

function viewCandidates() {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/list_candidates";

    const myInit = {
        method: 'GET',
        timeout: TTL,

    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        console.log(text);
        let rlist = text.split("\n");
        console.log(rlist);
        let domList = "<ul>";
        rlist.forEach((row) => {
            domList += "<li>" + row + "</li>";
        });
        domList += "</ul>";
        console.log(domList);
        document.getElementById("listcan").innerHTML = domList;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("listcan").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('listcan').innerHTML = "";
        }, 9000);
    })
}

function viewResult() {
    let url = "http://"+ this.sAddr + ":" + this.sPort + "/view_result";

    const myInit = {
        method: 'GET',
        timeout: TTL,

    }
    fetch(url, myInit).then(res => {
        return res.text();
    }).then(text => {
        text = text.substr(0, text.length - 1);
        console.log(text);
        let rlist = text.split("\n");
        console.log(rlist);
        let domList = "<ul>";
        rlist.forEach((row) => {
            domList += "<li>" + row + "</li>";
        });
        domList += "</ul>";
        domList = text === "ERROR"? "<p>Election is still running, can not view result</p>":
            domList;
        document.getElementById("viewresult").innerHTML = domList;
    }).catch(err => {
        console.error('Error:', err);
        document.getElementById("viewresult").innerHTML = err;
        setTimeout(()=>{
            document.getElementById('viewresult').innerHTML = "";
        }, 9000);
    })
}
