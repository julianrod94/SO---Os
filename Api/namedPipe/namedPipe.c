#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include "namedPipe.h"
#include "../commons.h"
#include <signal.h>
#include "../../Database/databaseapi.h"

static const char * serverMsg[8] = {
  "Estudiante agregado con exito!",
  "El estudiante ya existe!",
  "Estudiante modificado con exito!",
  "Estudiante eliminado con exito!",
  "Tabla creada con exito!",
  "Tabla eliminada con exito!",
  "Leyendo data...",
  "Error en la base de datos!"
};

Connection * createConnection(int fd){
  Connection * connection;
  connection = malloc(sizeof(Connection));
  connection -> fd = fd;
  return connection;
}

int * openNamedPipe(char * namedPipeName) {
  char origin[] = "/tmp/";
  char myfifo[80];
  int * fd;
  fd = malloc(sizeof(int)*2);
  
  strcpy(myfifo,origin);
  strcat(myfifo,namedPipeName);
  mkfifo(myfifo, 0666);

  fd[0] = open(myfifo, O_RDONLY|O_NONBLOCK);
  fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) &~O_NONBLOCK);
  fd[1] = open(myfifo, O_WRONLY);

  return fd;
}

void writeNamedPipe(int fd, void * data, int size) {
  write(fd, data, size);
}

requestState writeRequest(Request * request, int fd) {
  writeNamedPipe(fd, &request -> action, sizeof(int));
  writeNamedPipe(fd, &request -> connection -> fd, sizeof(int));
  writeNamedPipe(fd, &request -> connection -> dataSize, sizeof(int));
  writeNamedPipe(fd, request -> connection -> data, request -> connection -> dataSize);
  return REQUEST_OK;
}

int readNamedPipe (int fd, void * buffer) {
  buffer = malloc(BLOCK * sizeof(char));
  int q = 0;
  q = read(fd, buffer, BLOCK);
  if (q > 0)
    buffer = (char *) realloc(buffer, q);
  return q;
}

int closeNamedPipe(int fd, char * name) {
  char origin[] = "/tmp/";
  char myfifo[80] = "";
  strcat(myfifo, origin);
  strcat(myfifo, name);
  close(fd);
  unlink(myfifo);

  return 0;
}

Request * getRequest(Connection * connection, int listened) {
  Request *request; 
  int action, fd = 0;
  int dataSize;
  Student * student;
  read(connection-> fd, &action, sizeof(int));
  read(connection-> fd, &fd, sizeof(int));
  read(connection-> fd, &dataSize, sizeof(int));
  student = malloc (dataSize);
  read(connection-> fd, student, dataSize);
  request = createRequest(action, fd, dataSize, (void*)student);
  return request;
}

Connection* openConnection (void){
  Connection * connection;
  int* fd = openNamedPipe(REQUEST_QUEUE);
  connection = createConnection(fd[0]);
  return connection;
}

int getResponse(Connection * connection) {
  int* fd;
  char answerPipe[10] = "";
  sprintf(answerPipe, "%d", connection ->fd);
  fd = openNamedPipe(answerPipe);

  int r = 0,cant;
  int nread = 0;
  char * readBuffer;
 
  r=read(fd[0], &cant, sizeof(int));
  readBuffer = malloc(cant+1);
  r=read(fd[0], readBuffer, cant+1);
  printf("%s\n", readBuffer);
  return nread;
}

int writeResponse (Request * request, int state) {
  int* responseFd;
  char answerPipe[10] = "";
  sprintf(answerPipe, "%d", request -> connection ->fd);
  responseFd = openNamedPipe(answerPipe);
  int dataSize = strlen(serverMsg[state]+1);
  int written = write(responseFd[1], &dataSize, sizeof(dataSize));
  written += write(responseFd[1], serverMsg[state], dataSize);
  close(responseFd[1]);
  return 0;
}

int requestServer(Connection * connection, int action, int dataSize, void * data) {
  Request * request;
  int* responseFd;
  int* queueFd;
  char answerPipe[10] = "";
  sprintf(answerPipe, "%d", getpid());

  queueFd = openNamedPipe(REQUEST_QUEUE);
  responseFd = openNamedPipe(answerPipe);

  request = createRequest(action, getpid(), dataSize, data);
  *connection = (*request->connection);

  if(request == NULL || request->connection == NULL){
    return FAILED_ON_CREATE_REQUEST;
  }

  writeRequest(request, queueFd[1]);
  return responseFd[0];
}

Request * createRequest(int action, int fd, int dataSize, void * data){
  Connection * connection = createConnection(fd);
  connection -> dataSize = dataSize;
  connection -> data = malloc(dataSize);
  connection -> data = data;
  Request *request = malloc(sizeof(Request));
  request -> action = action;
  request -> connection = malloc(sizeof(connection));
  request -> connection = connection;
  return request;
}

void monitorConnection(Connection * connection, fd_set* set){
  FD_ZERO(set);
  FD_SET(connection -> fd, set);
  return;
}

int listenConnection(Connection * connection){
    fd_set set;
    monitorConnection(connection, &set);
    return select(connection -> fd + 1, &set, NULL, NULL, NULL);
}