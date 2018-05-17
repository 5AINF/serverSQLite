/*
Author: Damiano Lorenzo, Vertone Diego
Github repository: https://github.com/5AINF/serverSQLite
*/

#include "socket_tcp.hpp"
#include <stdio.h>
#include "sqlite3.h"
#include <string.h>

#define PORT 8080
#define MAX_BUFFER 4096
#define MAX_FILE 409640
#define TAGSQL "<sql query=\""
#define HEADER "HTTP/1.1 200 OK\n\
Content-Type: text/html; charset=UTF-8\n\
Content-language: it\n"

/*
Callback function
La funzione callback() viene chiamata ogni volta che la funzione sqlite_exec() trova una riga nel database
Questa funzione costruisce una riga di tabella in html - tag: <tr> - usando i dati recuperati dal database
La riga viene poi inserita in 's', una stringa passata per parametro (corrisponde a 'queryResult' nel main)
La ripetuta chiamata di questa funzione costruirà le righe della tabella html che verrà sostituita al tag <sql>
*/
int callback(void *s, int count, char **data, char **columns) {
	//prende la stringa 's' passata per parametro e la mette in 'punt'
	char* punt = (char*)s;
	//si sposta al termine di punt
	punt = punt + strlen(punt);
	//apre la riga inserendo il tag <tr>
	sprintf(punt,"%s<tr>",punt);
	//inserisce una cella <td> per ogni elemento del record ricevuto dal database
	for(int i=0;i<count;i++) {
		sprintf(punt,"%s<td>%s</td>",punt,data[i]?data[i]:"NULL");
	}
	//chiude la riga
	sprintf(punt,"%s</tr>",punt);
	return 0;
}

int main(int argc, char* argv[]) {
	char* headerHtml;		//conterrà la prima parte della pagina html (precedente al tag <sql>)
	char queryResult[MAX_BUFFER+1];	//conterrà il risultato della query al database
	char* footerHtml;		//conterrà il resto della pagina html (dopo il tag <sql>)
	char msg[MAX_FILE+1];		//conterrà la risposta completa da inviare al client che ha effettuato la richiesta
	
	ServerTCP* myself = new ServerTCP(PORT,false);	//Crea un server
	Connection* conn = myself->accept();		//Accetta la connessione del client
	char* request = conn->receive();		//Riceve la richiesta di pagina dal client
	
	char content[MAX_BUFFER+1];	//Conterrà l'intero contenuto del file richiesto
	sqlite3* sql_conn;		//Crea una connessione a sqlite
	int ret = sqlite3_open("scuola.sqlite",&sql_conn);	//Apre la connessione a sqlite sul database "scuola.sqlite"
	
	FILE* f = fopen("index.html","r");	//Apre il file richiesto (in questo caso aprerà sempre index.html, necessario implementare la ricerca del file richiesto)
	char c;
	int i;
	for(i=0;(c=fgetc(f))!=EOF;i++) {	//Copia l'intero contenuto del file all'interno della stringa 'content'
		content[i]=c;	
	}
	content[i]='\0';	//Chiude la stringa 'content'
	fclose(f);	//Chiude il file
	char* myTag = strstr(content, TAGSQL);	//Si sposta all'interno di content fino a trovare il tag <sql>
	headerHtml = (char*)malloc(sizeof(char)*(strlen(content)-strlen(myTag)));	//Adesso che è possibile ricavare la lunghezza del file html prima del tag <sql>, creiamo l'area di memoria per memorizzarlo
	memcpy(headerHtml,content,(strlen(content)-strlen(myTag)));	//Copiamo la prima parte del file html all'interno di 'headerHtml'
	myTag += strlen(TAGSQL);	//ci spostiamo fino a trovare l'inizio esatto della stringa di query
	char* endTag;	//Punterà alla fine della query
	for(endTag = myTag; *endTag != '"'; endTag++) { }	//Si sposta nel file fino a trovare la fine della query
	char* query = (char*)malloc(sizeof(char)*(endTag-myTag+1));	//Crea l'area per memorizzare la query
	memcpy(query,myTag,(endTag-myTag));	//Prende la query e la memorizza all'interno di 'query'
	query[endTag-myTag]='\0';	//Chiude la stringa 'query'
	char* error;	//Qui verrà salvata la stringa di errore nel caso fallisca la sqlite_exec()
	footerHtml = strdup(endTag+4);	//Salva in 'footerHtml' il resto del file html
	sprintf(queryResult,"<table border=\"1\">");	//Apre la tabella
	char* punt;	//Punterà alla fine di 'queryResult' per poter correttamente incolonnare i dati
	punt = queryResult + strlen(queryResult);	//Si sposta alla fine di 'queryResult'
	sqlite3_exec(sql_conn,query,callback,punt,&error);	//Esegue la query sul database
	sprintf(queryResult,"%s</table>",queryResult);	//Chiude la tabella
	sqlite3_close(sql_conn);	//Chiude la connessione al database
	sprintf(msg,"%s\n%s%s%s\n",HEADER,headerHtml,queryResult,footerHtml);	//Costruisce il messaggio di risposta per il client
	conn->send(msg);	//Invia la risposta
	
	//delete(&myself);	//Segnala un errore, risolvere in socket_tcp.hpp
	free(headerHtml);
	free(footerHtml);
	free(queryResult);
	free(content);
	return 0;
}
