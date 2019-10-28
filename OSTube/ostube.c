// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "apitools.c"
#define PORT 8080

//Function to open files as a string
char* openFile(char* filename){
	FILE* file = fopen(filename,"r");
	char* content = malloc(sizeof(char));
	int readChar = 0;
	char entry;

	if (!file){
		perror("\nCannot open file.\n");
		return NULL;
	}

	while ( (entry= fgetc(file)) != EOF ) {
		content = realloc(content, sizeof(char) * (readChar + 1) );
		content[readChar++] = entry;
	}

	content = realloc(content, sizeof(char) * (readChar + 1) );
	content[readChar++] = NULL;
	fclose(file);
	return content;
}


void home(int new_socket, struct request* req){
	char* index = openFile("index.html");

	char sizeBuffer[50] = {0};
	char sizeData [100] = "Content-length: ";
	sprintf(sizeBuffer, "%d", strlen(index)); //Append the number of characters
	strcat(sizeData, sizeBuffer);
	strcat(sizeData, "\n");

	//Header de la respuesta
	write(new_socket, "HTTP/1.1 200 OK\n", 16);
	write(new_socket, sizeData, strlen(sizeData));
	write(new_socket, "Content-Type: text/html\n\n", 25);
	//Aqui deberia ir un send file apropiado con un open por debajo

	write(new_socket , index , strlen(index) ); //Envio de datos :p
	printf("Home html sent\n");
	close(new_socket);
}


//Sorry, esta hecho un caos esto, dejaré notas para que entienda
//Ahorita, con esto tenemos lo más dificil

//Ya el get esta casi listo, si lo nota parseo las peticiones en path y query
//Con eso reconozco la funcion a usar
//Todas deben de tener este formato de socket y request eh!
//Ahorita, se pude tomar el filename de req, digamos (con el parser que falta :p)
//Bueno, la info de cada video debe de estar en un .txt, así que no hay que hacer nada raro
//Solo leer el txt y mostrarlo, eso no debería de costar mucho
//El log también esta en pendientes, por aquello

//Para esto del video, le recomiendo mucho, mucho, que hable con Juanjo

//Ojo, nosotros pasamos el video, solo que no se reproduce por el header
//Esta malo y yo tengo mucho sueño hahah
//Si entra a la dirección del video va a ver que se intenta cargar pero no lo logra
//por el header :(
//esa cosa le dice el tipo de información y tamaño a recibir, es lo que hay que arreglar de acá
//Para hacer el streaming.
//En fin, para esta función, hable con juanjo, legal, él ya lo tiene y se puede ahorrrar horas de
//horas si le ayuda
void watch(int new_socket, struct request* req){
	printf("\nInitialize with video\n");

	FILE * fpIn = fopen("video/im_a_kitty_cat.mp4", "r"); //Abrir el archivo, normal
	if (!fpIn){
		perror("\nCannot open file.\n");
		return;
	}
	char sizeBuffer[50] = {0};
	char sizeData [100] = "Content-length: ";

	while (1){ //Esto para leer por partes


		char buffer[10000] = {0}; //El ttamaño de un chunks de lectura

		ssize_t bytesRead = fread(buffer, 1, sizeof(buffer), fpIn); //Toma de ese chunk

		if (bytesRead <= 0) { //This is the last part --EOF
			write(new_socket, "HTTP/1.1 200 OK\n", 16); //para indicar que ya, se acabó
			sprintf(sizeBuffer, "%d", bytesRead); //Append the number of characters
			write(new_socket, sizeData, strlen(sizeData));
			write(new_socket, "Content-Type: video/mp4\n\n", 25);
			printf("\nFound EOF\n");
			break;
		}

		//Esto es el header, siempre se tiene que enviar
		sprintf(sizeBuffer, "%d", bytesRead); //Append the number of characters
		strcat(sizeData, sizeBuffer);
		strcat(sizeData, "\n");

		write(new_socket, "HTTP/1.1 206 OK\n", 16); //206 para indicar que es un parte
		write(new_socket, sizeData, strlen(sizeData));
		write(new_socket, "Content-Type: video/mp4\n\n", 25);

		printf("\nReading\n");

		if (send(new_socket, buffer, bytesRead, 0) != bytesRead){
			printf("\nFInish the reading\n");
			break;
		}
	}



	printf("\nFinalize with video\n");
	//upload a video somehow
}

void watchTest(int new_socket, struct request* req){

	FILE * fpIn = fopen("video/sample.mp4", "r"); //Abrir el archivo, normal
	if (!fpIn){
		perror("\nCannot open file.\n");
		return;
	}
	fseek(fpIn, 0L, SEEK_END);
	int fsize = ftell(fpIn);
	rewind(fpIn);
	fsize++;
	char *fileData = malloc(fsize);
	fread(fileData, 1, fsize, fpIn);

	int charSize = (int)((ceil(log10(fsize))+1) * sizeof(char));
	int finalSize = charSize + 18;

	char *sizeData = malloc(finalSize);

	strcpy(sizeData, "Content-length: ");

	//Esto es el header, siempre se tiene que enviar
	sprintf(sizeData, "%d", fsize);
	strcat(sizeData, "\r\n");

	write(new_socket, "HTTP/1.1 200 OK\r\n", 17); //206 para indicar que es un parte
	write(new_socket, sizeData, finalSize);
	//write(new_socket, "Accept-Ranges: bytes\r\n", 22);
	write(new_socket, "Content-Type: video/mp4\r\n", 25);
	write(new_socket, "\r\n", 2);

	int headerSize = finalSize + 17 + 25 + 2 + fsize;

	if (send(new_socket, fileData, headerSize, 0) != fsize){
		printf("\n FINISH \n");
	} else {
		printf("\n FINISH > ERROR\n");
	}

	fclose(fpIn);

}


int main(int argc, char const *argv[]) {
	//Main mounts the server
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};

	callsList = malloc(sizeof(struct callback*));
	printf("\nstart the server\n");
	new_callback("/", home);
	new_callback("/watch", watchTest);


	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//At this point, everyting is fine, we can take on clients from here on
	printf("Listening on port %d \n", PORT);


	while (1){
		//Code for the index
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}


		valread = read(new_socket , buffer, 1024);
		printf("%sBuffer\n", buffer );

		struct request *req = parseRequest(buffer);
		printf("\nRequest done, time to call \n");
		struct callback* call = getCallback(req->path);

		//Aqui se elije la funcion a ejcutar
		//Esta directo a home ahorita
		//home(new_socket);

		if (call != NULL){
			call->function(new_socket, req);
			printf("Calling?");
		}else {
			printf("We have nothing for that");
		}
	}



	return 0;
}
