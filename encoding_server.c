//encryption server

#define _POSIX_C_SOURCE 200809L                                                                                                                            
                                                                                                                                                           
#include <stdio.h>                                                                                                                                         
#include <stdlib.h>                                                                                                                                        
#include <unistd.h>                                                                                                                                        
#include <string.h>                                                                                                                                        
#include <sys/types.h>  // ssize_t                                                                                                                         
#include <sys/socket.h> // send(),recv()                                                                                                                   
#include <netdb.h>      // gethostbyname() 
#include <signal.h>   
#include <errno.h>
#include <sys/wait.h>
                                                                                                                                                           
#define SIZE 100000                                                                                                                                        
                                

//sendall() straight from Beej's, receiveall() adapted from this                 
//function to make sure all bytes are sent                                                                                                                 
//args: s = socket, buf = data, len = number of bytes in buf                                                                                               
int sendall(int s, char *buf, int *len)                                                                                                                    
{                                                                                                                                                          
  int total = 0;        // how many bytes we've sent                                                                                                       
  int bytesleft = *len; // how many we have left to send                                                                                                   
  int n;                                                                                                                                                   
                                                                                                                                                           
  while(total < *len)                                                                                                                                      
  {                                                                                                                                                        
    n = send(s, buf+total, bytesleft, 0);                                                                                                                  
    if (n == -1) { break; }                                                                                                                                
    total += n;                                                                                                                                            
    bytesleft -= n;                                                                                                                                        
  }                                                                                                                                                        
                                                                                                                                                           
  *len = total; // return number actually sent here                                                                                                        
                                                                                                                                                           
  return n==-1?-1:0; // return -1 on failure, 0 on success                                                                                                 
}                                                                                                                                                          
                                                                                                                                                           
//sendall function mirrored to receiveall                                                                                                                  
//same exact functionality, but for recv()                                                                                                                 
int receiveall(int s, char *buf, int size)                                                                                                                 
{                                                                                                                                                          
  int total = 0;                                                                                                                                           
  int bytesleft = size;                                                                                                                                    
  int n;                                                                                                                                                   
                                                                                                                                                           
  while(total < size)                                                                                                                                      
  {
    //test statements
    //printf("Bytes expecting: %i\n", size);
    

    n = recv(s, buf+total, bytesleft, MSG_WAITALL);                                                                      
    if (n == -1) { break; }                                                                                                                                
    total += n;                                                                                                                                            
    bytesleft -= n; 

    //printf("Bytes received: %i\n", total);
    //printf("Bytes left: %i\n", bytesleft);
  }                                                                                                                                                        
                                                                                                                                                           
  size = total;                                                                                                                                            
                                                                                                                                                           
  return n==-1?-1:0;                                                                                                                                       
}                                                                                                                                                          
//***************************************************************************************************************************************   
//straight from beej's to reap child processes
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  //initalize variables to hold plaintext and key
  char plaintext[SIZE] = {0};
  char key[SIZE] = {0};

  int connectionSocket, charsRead;
  char buffer[256];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket\n");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding\n");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  //straight from beej's to reap child processes
  struct sigaction sa;
  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
      perror("sigaction\n");
      exit(1);
    }
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept\n");
    }

    //test statements
    /*
    printf("SERVER: Connected to client running at host %d port %d\n", 
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));
    */

    //fork child process
    if (!fork())
    {
      close(listenSocket); //child doesn't need listener
      //receive server name for validation
      char server_name[SIZE] = {0};
      int server_len = 12;
      int one_byte = 1;
      int len_size = 10;
      int validation_len = 39;

      if (receiveall(connectionSocket, server_name, server_len) == -1)
      {
        perror("receiveall: Not all server name bytes were received.\n");
      }
      //printf("server_name: %s", server_name); //test statement
      if (strcmp("./enc_client", server_name) != 0)
      {
        if (sendall(connectionSocket, "1", &one_byte) == -1)
        {
          perror("sendall: Not all server validation bytes were properly sent.\n");
        }
        close(connectionSocket);
        exit(0);
      }
      if (sendall(connectionSocket, "2", &one_byte) == -1)
      {
        perror("sendall: Not all server validation bytes were properly sent.\n");
      }
      
      //*******************************************************************************
      //receive, transform, and store input_size
      if (receiveall(connectionSocket, buffer, len_size) == -1)
      {
        perror("receiveall: Not all size bytes were received.\n");
      }

      //printf("SERVER: I received this from the client: \"%s\"\n", buffer);

      //convert received size to int
      int input_size = atoi(buffer);
      //printf("SERVER: Converted size: \"%i\"\n", input_size);
      
      //send confirmation to client
      if (sendall(connectionSocket, "I am the server, and I got your message", &validation_len) == -1)
      {
        perror("sendall: First confirmation send - not all bytes were properly sent.\n");
      }

      //***********************************************************************************
      // receive and store plaintext
      if (receiveall(connectionSocket, plaintext, input_size) == -1)
      {
        perror("receiveall: Not all plaintext bytes were received.\n");
      }
      if (sendall(connectionSocket, "I am the server, and I got your message", &validation_len) == -1)
      {
        perror("sendall: Second confirmation send - not all bytes were properly sent.\n");
      }
      //printf("SERVER: Plaintext: %s\n", plaintext);
    
      //***********************************************************************************
      // receive and store key
      if (receiveall(connectionSocket, key, input_size) == -1)
      {
        perror("receiveall: Not all key bytes were received.\n");
      }
      //test statement
      //printf("SERVER: key: %s\n", key);

      //encode plaintext via one-time-pad A-Z + ' ' algorithm and send back
      char encoded_buffer[SIZE];
      int temp;
      for (int i = 0; i < input_size; i++)
      {
          //convert ' ' to '[' aka ascii value 91 for ascii math purposes
          if (plaintext[i] == ' ')
          {
              plaintext[i] = '[';
          }
          if (key[i] == ' ')
          {
              key[i] = '[';
          }
          
          //add ASCII values
          temp = (plaintext[i] + key[i]) - 130;
          if (temp > 26)
          {
              temp = temp - 27;
          }
          //store encrypted char
          encoded_buffer[i] = temp + 65;
          
          //convert '[' back to ' '
          if (encoded_buffer[i] == '[')
          {
            encoded_buffer[i] = ' ';
          }
          //test statement
          //printf("encode: %i\n", encoded_buffer[i]);
      }
      if (sendall(connectionSocket, encoded_buffer, &input_size) == -1)
      {
        perror("sendall: Third confirmation send - not all bytes were properly sent.\n");
      }

      shutdown(connectionSocket, 1);
      exit(0);
    }


    // Close the connection socket for this client
    close(connectionSocket); 
  }
  return 0;
}
