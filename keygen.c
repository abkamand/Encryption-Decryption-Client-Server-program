//creates a 'key' composed of random chars A-Z + ' ' of desired length

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_SIZE 100000


int main(int argc, const char * argv[]) 
{
  //grab key length and convert to int
  const char *input;
  input = argv[1];
  int key_len;
  key_len = atoi(input);

  //printf("key length: %i\n", key_len); //test statement

  //initialize random number generator
  srand (time(NULL));

  //create an array filled with key_len # of random characters and output to stdout
  char key[KEY_SIZE] = {0};
 
  //create alphabet array
  static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int gen;
  
  //fill key with random chars
  for (int i = 0; i < key_len; i++)
  {
    gen = rand() % 27;
    //printf("random num: %i\n", gen); //test statement

    key[i] = alphabet[gen];
    /*//testing
    printf("key char: %c\n", key[i]);
    printf("key thus far: %s\n", key);
    */
  }
  
  printf("%s\n", key);
  //printf("%lu", strlen(key)); //test statement
  
}
