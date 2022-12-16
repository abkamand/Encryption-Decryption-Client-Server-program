//decryption client for OTP
//most of the code from OTP module and Beej's

#define _POSIX_C_SOURCE 200809L                                                                                                                                                                   
                                                                                                                                                                                                  
#include <stdio.h>                                                                                                                                                                                
#include <stdlib.h>                                                                                                                                                                               
#include <unistd.h>                                                                                                                                                                               
#include <string.h>                                                                                                                                                                               
#include <sys/types.h>  // ssize_t                                                                                                                                                                
#include <sys/socket.h> // send(),recv()                                                                                                                                                          
#include <netdb.h>      // gethostbyname()                                                                                                                                                        
                                                                                                                                                                                                  
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

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  } 

  //get file inputs*********************************                                                                                                                                              
  FILE *input1;                                                                                                                                                                                   
  FILE *input2;                                                                                                                                                                                   
  char plaintext[SIZE] = {0};                                                                                                                                                                     
  char key[SIZE] = {0};                                                                                                                                                                           
  char decoded_text[SIZE] = {0};                                                                                                                                                                
                                                                                                                                                                                                  
  //open files, print errors on failure and exit with value 1                                                                                                                                     
  input1 = fopen(argv[1], "r");                                                                                                                                                                   
  if (input1 == NULL)                                                                                                                                                                             
  {                                                                                                                                                                                               
    perror("bad plaintext file\n");                                                                                                                                                                 
    exit(1);                                                                                                                                                                                      
  }                                                                                                                                                                                               
  input2 = fopen(argv[2], "r");                                                                                                                                                                   
  if (input2 == NULL)                                                                                                                                                                             
  {                                                                                                                                                                                               
    perror("bad key file\n");                                                                                                                                                                       
    exit(1);                                                                                                                                                                                      
  }                                                                                                                                                                                               
                                                                                                                                                                                                  
  //get data from files                                                                                                            
  fgets(plaintext, sizeof plaintext, input1);                                                                                                                                                     
  fgets(key, sizeof key, input2);                                                                                                                                                                 
                                                                                                                                                                                                  
  //get rid of trailing \n                                                                                                                                                                        
  //citation: idiomatic 1-liner :) - https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input                                                             
  plaintext[strcspn(plaintext, "\n")] = 0;                                                                                                                                                        
  key[strcspn(key, "\n")] = 0;                                                                                                                                                                    

  // error check if len_plain > len_key                                                   
  int len_plain = strlen(plaintext);                                                       
  int len_key = strlen(key);                                                               
                                                                                           
  if (len_key < len_plain)                                                                
  {                                                                                        
    fprintf(stderr, "Key is shorter than plaintext.\n");                                  
                                                                                           
    //test block                                                                           
    //printf("Key length: %i\n", len_key);                                                 
    //printf("Plaintext length: %i\n", len_plain);                                         
                                                                                           
    //test block                                                                           
    //printf("plaintext: %s\n", plaintext);                                                
    //printf("key: %s\n", key);                                                            
                                                                                           
    exit(1);                                                                               
  } 

  //test block                                                                                                                                                                                    
  //printf("plaintext: %s\n", plaintext);                                                                                                                                                         
  //printf("key: %s\n", key);                                                                                                                                                                     
  //end of get file inputs**************************
  //****************************************************************************

  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256] = "test";


  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket\n");
  }
  
   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting\n");
  }

                                                     
  //Send plaintext/key len to the server to utilize in our corresponding recv() calls server side                         
  //need to send a string of fixed size, so convert len_plain to a string and then pad with leading zeros                 
  char padded_len[] = "0000000000";                                                                                       
  char converted_len[11] = {0};                                                                                           
  int temp = strlen(converted_len);                                                                                       
  sprintf(converted_len, "%d", len_plain);                                                                                
  int count = 0;                                                                                                          
                                                                                                                          
  //test block                                                                                                            
  /*                                                                                                                      
  printf("Converted length: %s\n", converted_len);                                                                        
  printf("Converted length size: %lu\n", strlen(converted_len));                                                          
  printf("padded length size: %lu\n", strlen(padded_len)); //test statement                                               
  */                                                                                                                      
                                                                                                                          
  //pad                                                                                                                   
  for (int i = (strlen(padded_len) - strlen(converted_len)); i < strlen(padded_len); i++)                                 
  {                                                                                                                       
    padded_len[i] = converted_len[count];                                                                                 
    count++;                                                                                                              
  }
  int len_padded = strlen(padded_len);
                                                                                                                          
  //test block                                                                                                            
  /*                                                                                                                      
  printf("padded length: %s\n", padded_len); //test statement                                                             
  printf("padded length size: %lu\n", strlen(padded_len)); //test statement                                               
  printf("Made it to send\n"); //test statement                                                                                                                                                
  */                                                                                                                      
                                                                                                                                                                                                                                            
  //*********************************************************************************************                         
  //send section                                                                  
  //************************************************************************************************
  //send server name for validation
  int len_server = strlen(argv[0]);
  if (sendall(socketFD, argv[0], &len_server) == - 1)
  {
    perror("sendall: Not all server name bytes were properly sent.\n");
  }
  // Get return message from server
  // Clear out the buffer
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket\n");
  }
  //printf("CLIENT: I received this from the server (2 = good, 1 = wrong server): \"%s\"\n", buffer);
  // if we did not connect to the correct server aka received '1', exit(2)
  if (strcmp(buffer, "1") == 0)
  {
    fprintf(stderr,"We tried to connect to the wrong server at port %s\n", argv[3]);
    exit(2);
  }
  //************************************************************************************************
  //send size to the server and receive confirmation  
  if (sendall(socketFD, padded_len, &len_padded) == -1) 
  {
    perror("sendall: Not all padded_len bytes were properly sent.\n");
  }

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket\n");
  }
  //printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
  
  //***************************************************************************************************
  //send plaintext to the server and receive confirmation***********************************************
  if (sendall(socketFD, plaintext, &len_plain) == -1)
  {
    perror("sendall: Not all plaintext bytes were properly sent.\n");
  }
  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket\n");
  }
  //printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

  //***************************************************************************************************
  //send key to the server and receive confirmation***********************************************
  if (sendall(socketFD, key, &len_key) == -1)
  {
    perror("sendall: Not all key bytes were properly sent.\n");
  }

  
  //**********************************************************
  //receive decoded_text from server
  if (receiveall(socketFD, decoded_text, len_plain) == -1)
  {
    perror("receiveall: Not all encoded_text bytes were properly received.\n");
  }
  //printf("CLIENT: I received this from the server: \"%s\"\n", decoded_text);

  
  //output decoded text
  printf("%s\n", decoded_text);
  fflush(stdout);

  //close the files
  fclose(input1);
  fclose(input2);
  // Close the socket
  close(socketFD); 
  return 0;
}
