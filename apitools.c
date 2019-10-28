#include <string.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 

struct request;


int callsRegister = 0;
struct callback** callsList;

struct callback{
    void (*function)(int, struct request*);
    char path[50];
};

void addCallback(struct callback* newCall){
    callsRegister++;
    callsList = realloc(callsList, sizeof(struct callback*) * callsRegister);
    callsList[callsRegister-1] = newCall;
}

struct callback* new_callback(char* path, void (*function)(int, struct request*)){
    struct callback* newStruct = malloc(sizeof(struct callback));

    strcpy(newStruct->path, path);
    newStruct->function = function;

    addCallback(newStruct);
    return newStruct;
}

struct callback* getCallback(char* path){
    struct callback* call = NULL;
    struct callback* aux = NULL;
    for (int i = 0; i < callsRegister; i++){
        printf("\nSegmentation core ");
        aux = callsList[i];
        if ( strcmp(aux->path, path) == 0){
            call = aux;
            break;
        }
    }

    return call;
}

struct request { //Lista de querys al final
    int values;
	struct queryValue* query;
    char path[50];
};

struct queryValue { //Valor de cada query
    char key[50];
    char value[200];
};

struct request* parseQuery(char *text){    
    struct request* req = malloc(sizeof(struct request));
    req->query = malloc( sizeof(struct queryValue) * 10);

    char* aux = text;
    char* end = malloc(sizeof(char));
    int addedRow = 0;

    while (*aux != 0 && strstr(aux, "=")){ //Mientras se tenga un parametro
        end = strstr(aux, "=");
        char key[50] = {0};
        char value[200] = {0};
        int keyAdded = 0;
        int valueAdded = 0;
        
        while (aux != end){
            key[keyAdded++] = *aux;
            aux++;
        }

        printf("\nparameter: %s\n", key);
        aux++;

        end = strstr(aux, "&");
        int i = 0;

        while (*aux != 0){
            if (end && aux == end) break;
            
            value[valueAdded++] = *aux;
            aux++;
            i++;
            
        }
        printf("value: %s\n", value);

        struct queryValue temp;// = malloc(sizeof(struct queryValue));
        strcpy(temp.key, key);
        strcpy(temp.value, value);
        req->query[addedRow++] = temp;
    }

    return req;
}

struct index { //There will be two of them, one for post and one for get
	char* path;
	void (*function)(struct request);
};

struct index* getFuction(char* path, struct index indexes[]){
    struct index* aux = indexes;

    while (aux != NULL){
        if ( strcmp(aux->path, path) ) break;
        aux++;
    }

    return aux;
}

//The idea is to parse the request into direction and query
struct request* parseRequest(char* buffer){
	char* aux = buffer+4; //Inicia acá porque todos son get
	char path[100] = {0}; 
	char query[1000] = {0}; 

	int pathAdded = 0;
	int queryAdded = 0;
	
	char* endRead = strstr(buffer, " HTTP/1.1");
	
	while (aux != NULL){
		if (aux == endRead){
			goto parsingDone;
		} else if (*aux == '?')	break;
		
		path[pathAdded++] = *aux;
		aux++;
	}
	aux++; //To skip the ?

	while (aux != NULL){ //Parsing the query
		if (aux == endRead)	break;		
		query[queryAdded++] = *aux;
		aux++;
	}
    
    parsingDone:
    printf("Parse result");
	printf("\n%s\n", path);
	printf("\n%s\n", query);

    struct request* req = parseQuery(query);
    strcpy(req->path, path);

	
	

    printf("That ended");
    return req;

}

