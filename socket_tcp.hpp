#ifndef __SOCKETTCP_HPP
#define __SOCKETCTP_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "Address.hpp"
#include <list>
#include <errno.h>

#define IP_LOOPBACK "127.0.0.1"
#define IP_MYSELF "0.0.0.0"
#define MAX_CONN 50
#define MAX_MSG 4096

using namespace std;

//____________________________________________________________________

void errore(char* func, int error){
	printf("Errore bloccante sulla funzione %s\n", func);
	printf("%s\n", strerror(errno));
	exit(error);
}

//____________________________________________________________________
void* duplica(char* buffer, int* len){
	char* ret = (char*) malloc(sizeof(char) * (*len +1 ));
	for(int i = 0; i <= *len; i++)	*(ret+i) = *(buffer + i);
	return ret;
}

//__________________________________________________________________
//START SocketTCP

class SocketTCP{
protected:	int sock_id;	
public:		SocketTCP();	//API: socket()
		~SocketTCP();	//API: close()

		bool set_broadcast(bool);
};

SocketTCP::SocketTCP(){
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
}

SocketTCP::~SocketTCP(){
	close(sock_id);
}

bool SocketTCP::set_broadcast(bool broadcast){
//returns true if set_broadcast didn't work well.
//Usage: if(set_broadcast(true)) printf("Error");
	int optionvalue = broadcast? 1:0;
	int ret = setsockopt(sock_id,
			SOL_SOCKET,
			SO_BROADCAST,
			(void*)&optionvalue,
			(socklen_t)sizeof(int));
	return ret != 0;
}

//END SocketTCP
//__________________________________________________________________
//START Connection

class Connection{
private:	int conn_id;
		bool is_fuffa;
//if fuffa is true, the connection is a fake connection and the
//connection ID is a socket ID in disguise.
public:		Connection(int id_conn, bool fuffa); 
		~Connection();		
		bool send_raw(void*, int); //API: send()
		void* receive_raw(int* len);
		//API: recv()
		bool send(char* msg);
		char* receive();
		int get_id();
};

int Connection::get_id(){
	return conn_id;
}

Connection::Connection(int id_conn, bool fuffa){
	conn_id = id_conn;
	is_fuffa = fuffa;
}

bool Connection::send_raw(void* msg, int len){
	int ret = (int) ::send(conn_id, msg, (size_t)len, 0);
	return(ret != len);
}

void* Connection::receive_raw(int* lenga){
	char buffo[MAX_MSG+1];
	*lenga = recv(conn_id,
			buffo,
			MAX_MSG,
			0);
	if(*lenga <= 0)	return NULL;
	return duplica(buffo, lenga);
}

bool Connection::send(char* msg){
	fflush(stdout);	
	//msg[strlen(msg)] = '\0';
	return send_raw((void*) msg, strlen(msg)+1);
}

char* Connection::receive(){
	int len = 0;
	char* ret = (char*)receive_raw(&len);
	if(ret == NULL)	return NULL;
	ret[len] = '\0';
	return ret;
}

Connection::~Connection(){
	if(!is_fuffa)	shutdown(conn_id, SHUT_RDWR);
}

//END Connection
//________________________________________________________________
//START ServerTCP

class ServerTCP: public SocketTCP{
//Inherits from SocketTCP
private:	std::list<Connection> connections;
public:		ServerTCP(int port, bool loopback);
		//API: bind() | listen()
		~ServerTCP();
		void send_everyone(char* msg);
		void disconnect(Connection c);
		Connection* accept(); //API: accept()
};

void ServerTCP::disconnect(Connection c){
	//connections.remove(c);
}

void ServerTCP::send_everyone(char* msg){
	for(Connection c : connections)	c.send(msg);
}

ServerTCP::ServerTCP(int port, bool loopback):SocketTCP(){
	Address* myself;
	struct sockaddr_in myself_addr;
	if(loopback)
		myself = new Address(port, strdup(IP_LOOPBACK));
	else
		myself = new Address(port, strdup(IP_MYSELF));
	myself_addr = myself->get_address();
	bind(sock_id,
		(struct sockaddr*) &myself_addr,
		(socklen_t)sizeof(struct sockaddr));
	listen(sock_id, MAX_CONN);
}

ServerTCP::~ServerTCP(){
	connections.clear();
	//Automatic call to superclass destructor
}

Connection* ServerTCP::accept(){
	Connection* ret;
	Address* sender;
	struct sockaddr_in sender_addr;
	int addrlen = sizeof(struct sockaddr);
	int conn_id = ::accept(sock_id,
			(struct sockaddr*) &sender_addr,
			(socklen_t*)&addrlen);
	
	sender = new Address(sender_addr);
	if(conn_id <= 0)	return NULL;
	ret = new Connection(conn_id, false);
	connections.push_back(*ret);
	return ret;
}

//END ServerTCP
//________________________________________________________________
//START ClientTCP

class ClientTCP: public SocketTCP{
//Inherits from SocketTCP
private:	Connection* connection;
public:		ClientTCP();
		//API: connect()
		~ClientTCP();
		bool connect(Address);
		bool send(char* message);
		bool send_raw(void* message, int len);
		char* receive();
		void* receive_raw(int* len);
};

void* ClientTCP::receive_raw(int* len){
	return connection->receive_raw(len);
}

bool ClientTCP::send_raw(void* message, int len){
	return connection->send_raw(message, len);
}

ClientTCP::ClientTCP(){}

bool ClientTCP::connect(Address server){
	struct sockaddr_in server_addr;
	server_addr = server.get_address();	
	int ret = ::connect(sock_id,
		(struct sockaddr*) &server_addr,
		(socklen_t) sizeof(struct sockaddr_in));
	connection = new Connection(sock_id, true);
	return ret;
}

bool ClientTCP::send(char* message){
	return connection->send(message);
}

char* ClientTCP::receive(){
	return connection->receive();
}

ClientTCP::~ClientTCP(){
	delete connection;
}

//END ClientTCP____________________________________________________

#endif
