#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson/cJSON.h"

int getFileStr(char* inFile, char** fileStr){
   /*
      Get a full string of the contents of file at path inFile.
      Need to pass this &str for the second argument due to dynamic memory realloc
   */
   printf("Reading file %s \n", inFile);
   
   FILE *fptr;
   // read file in with basic error checking
   if ((fptr = fopen(inFile, "r")) == NULL){ 
      printf("Error opening file %s\nQuitting.\n", inFile);
      return 1;
   }

   /* Get the number of bytes */
   fseek(fptr, 0L, SEEK_END);
   long numbytes = ftell(fptr);
   
   /* reset the file position indicator to 
   the beginning of the file */
   fseek(fptr, 0L, SEEK_SET);	
   
   /* grab sufficient memory for the 
   buffer to hold the text */
   *fileStr = (char*)calloc(numbytes, sizeof(char));	
   
   /* memory error */
   if(*fileStr == NULL)
      return 2;
   
   /* copy all the text into the buffer */
   fread(*fileStr, sizeof(char), numbytes, fptr);

   fclose(fptr); // close file to free memory
   return 0;
}

int readJson(char* inFile){   
   /*
      Reads the json file specified by inFile.
   */
   char* fileStr = NULL;
   if(getFileStr(inFile, &fileStr) != 0){ // read, return on error
      printf("Error reading file %s\n", inFile);
   } 
   printf("==== Read JSON =====\n%s", fileStr);

   free(fileStr); // free the file string when done
}

int main() {
   readJson("testCase.json");

   return 0;
}