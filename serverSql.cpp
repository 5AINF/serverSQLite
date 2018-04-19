#include "Socket.hpp"

#define PORT 8080
#define MAX_BUFFER 4096
#define TAGSQL "<sql query=\""
#define HEADER "HTTP/1.1 200 OK\n \
		Content-Type: text/html; charset=UTF-8\n \
		Content-language: it\n "

int main(int argc, char* argv[]) {
	char* headerHtml;
	char queryResult[MAX_BUFFER+1];
	char* footerHtml;
	char* msg:
	
	ServerTCP* myself = new ServerTCP(PORT,true);
	Connection* conn = myself->accetta();
	char* request = conn->ricevi();
	
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
	headerHtml = (char*)malloc(sizeof(char)*(myTag));
	memcpy(headerHtml,0,myTag);
	myTag += lenstr(TAGSQL);
	char* endTag;
	for(endTag = myTag; *endTag != '"'; endTag++) { }
	char* query = (char*)malloc(sizeof(char)*(endTag-myTag+1));
	memcpy(query,myTag,(endTag-myTag));
	query[endTag-myTag]='\0';
	
	char* error;
	footerHtml = dupstr(endTag+2);
	sprintf(queryResult,"<table>");
	char* punt;
	punt = queryResult + lenstr(queryResult);
	sqlite3_exec(conn,query,foo,NULL,&error);
	sprintf(queryResult,"%s</table>",queryResult);
	sqlite3_close(conn);
	
	sprintf(msg,"%s\n%s%s%s\n",HEADER,headerHtml,queryResult,footerHtml);
	conn->invia(msg);
	
	delete(sql_conn);
	delete(conn);
	delete(myself);
	free(request);
	free(headerHtml);
	free(queryResult);
	free(footerHtml);
	return 0;
}

void* foo(void* last) {
	
}



















