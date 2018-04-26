#ifndef ADDRESS_HPP
#define ADDRESS_HPP
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

class Address{
	private:
		int port;
		char* ip;

	public:
		//Costruttori
		Address();			//Costruttore vuoto
		Address(int, char*);		//Overload costruttore
		Address(struct sockaddr_in);	//Overload costruttore
		Address(const Address&); 	//metodo clonatore
		~Address();			//metodo distruttore
		//Altri metodi
		void init(int, char*);	//This'll be called by every ^
		char* to_string();
		void show();
		//Accessors
		void set_address(struct sockaddr_in);
		struct sockaddr_in get_address();
		char* get_ip();
		int get_port();
		void set_ip(char*);
		void set_port(int);
};

void	Address::show(){
		printf("%s:%d\n", ip, port);
}

char*	Address::to_string(){
		char s[22];
		sprintf(s, "%s:%d", ip, port);
		return strdup(s);
}

Address::Address(const Address& dolly){
	ip = strdup(dolly.ip);
	port = dolly.port;
}

void	Address::init(int p, char* i){
		port = p;
		ip = strdup(i);
}

Address::Address(struct sockaddr_in a){
	free(ip);
	port = ntohs(a.sin_port);
	ip = strdup(inet_ntoa(a.sin_addr));
}

Address::Address(){
	init(0, strdup("0.0.0.0"));
}

Address::Address(int p, char* i){
	init(p, i);
}

Address::~Address(){
}

char*	Address::get_ip(){return ip;}

int	Address::get_port(){return port;}

void	Address::set_ip(char* i){
		free(ip);
		ip = i;
}
void	Address::set_port(int p){
		port = p;
}
void 	Address::set_address(struct sockaddr_in address){
	free(ip);
	port = ntohs(address.sin_port);
	ip = strdup(inet_ntoa(address.sin_addr));
}
struct sockaddr_in Address::get_address(){
	struct sockaddr_in ret;
	inet_aton(ip, &ret.sin_addr);
	ret.sin_port = (htons(port));
	ret.sin_family = AF_INET;
	for(int i = 0; i < 8; i++){
		ret.sin_zero[i] = 0;
	}
	return ret;
}

#endif
