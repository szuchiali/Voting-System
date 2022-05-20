TARGETS = server client server_ec3 server_http server-api
CPPFLAGS = -g -Wall -Werror -pthread

all: $(TARGETS)

server_ec3: server_ec3.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

server_http: server_http.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

server: server.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

client: client.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

server-api: server-api.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

.PHONY : clean
clean::
	rm -fv $(TARGETS) *~ *.o
