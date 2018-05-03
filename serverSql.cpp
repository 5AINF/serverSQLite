#include "socket_tcp.hpp"
#include <stdio.h>
#include "sqlite3.h"
#include <string.h>

#define PORT 8080
#define MAX_BUFFER 4096
#define MAX_FILE 409640
#define TAGSQL "<sql query=\""
#define HEADER "HTTP/1.1 200 OK\n \
		Content-Type: text/html; charset=UTF-8\n \
		Content-language: it\n "

int callback(void *s, int count, char **data, char **columns) {
	char* punt = (char*)s;
	//Inserire nomi colonne
	punt = punt + strlen(punt);
	sprintf(punt,"%s<tr>",punt);
	for(int i=0;i<count;i++) {
		sprintf(punt,"%s<td>%s</td>",punt,data[i]?data[i]:"NULL");
	}
	sprintf(punt,"%s</tr>",punt);
	return 0;
}

int main(int argc, char* argv[]) {
	char* headerHtml;
	char queryResult[MAX_BUFFER+1];
	char* footerHtml;
	char msg[MAX_FILE+1];
	
	ServerTCP* myself = new ServerTCP(PORT,true);
	Connection* conn = myself->accept();
	char* request = conn->receive();
	
	char buffer[MAX_BUFFER+1];
	char content[MAX_BUFFER+1];
	sqlite3* sql_conn;
	int ret = sqlite3_open("scuola.sqlite",&sql_conn);
	
	FILE* f = fopen("index.html","r");
	char c;
	int i;
	for(i=0;(c=fgetc(f))!=EOF;i++) {
		content[i]=c;	
	}
	content[i]='\0';
	fclose(f);
	char* myTag = strstr(content, TAGSQL);
	headerHtml = (char*)malloc(sizeof(char)*(strlen(content)-strlen(myTag)));
	memcpy(headerHtml,content,(strlen(content)-strlen(myTag)));
	myTag += strlen(TAGSQL);
	char* endTag;
	for(endTag = myTag; *endTag != '"'; endTag++) { }
	char* query = (char*)malloc(sizeof(char)*(endTag-myTag+1));
	memcpy(query,myTag,(endTag-myTag));
	query[endTag-myTag]='\0';
	char* error;
	footerHtml = strdup(endTag+4);
	sprintf(queryResult,"<table>");
	char* punt;
	punt = queryResult + strlen(queryResult);
	sqlite3_exec(sql_conn,query,callback,punt,&error);
	sprintf(queryResult,"%s</table>",queryResult);
	sqlite3_close(sql_conn);
	sprintf(msg,"%s\n%s%s%s\n",HEADER,headerHtml,queryResult,footerHtml);
	conn->send(msg);
	
	delete(&sql_conn);
	delete(conn);
	delete(&myself);
	return 0;
}
