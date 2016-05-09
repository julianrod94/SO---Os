#include "request.h"

#define NOT_FOUND_ERR   NULL

enum connectionsErrors { ERROR_CREATE_SERVER_RESPONSE_RECIEVER = 400, ERROR_OPEN_REQUEST_QUEUE } connectionError;

int requestServer(Connection * connection, int action, int type, size_t dataSize, void * data) {
  Request * request = createRequest(action, type, dataSize, data);

  if(request == NULL){
    printf("Failed to create request\n");   //no esta bueno poner printf en backend pero nos va a servir para debugear.
    return FAILED_ON_CREATE_REQUEST;                           //despues los borro
  }

  request->direction = fd[1];
  request->directionSize = sizeof(int);
  writeRequest(request);
  createConnection(connection);
  if(connection == NULL){
    return NULL;
  }
  connection -> direction = fd[0];
  return SUCCESS; 
}

int getRequest(Request * request) {
  int aux_err;
  int fd = 0;
  //TODO open pipe with its name and start reading from it

  // NAME is the named pipe name from where to read - should be a string
  fd = open(NAME, O_READONLY);
  request = (Request *)malloc(sizeof(Request));
  aux_err = read( fd, request, sizeof( Request ) );
  if ( aux_err )
    return ERROR;
  return NOT_FOUND_ERR; // return NULL
}


int processRequest(Request * r) {
  int to_execute = r -> action;
  int (*functionChooser[3]) (Request * r);

  functionChooser[0] = readRequest;
  functionChooser[1] = writeRequest;
  functionChooser[2] = deleteRequest;

  if (to_execute != READ && to_execute != CREATE && to_execute != DELETE)
    return ERROR;

  return (*functionChooser[to_execute]) (r);
}
