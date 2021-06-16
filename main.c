#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson/cJSON.h"

// helper data structures to make my life easier
struct obj
{
   int int1;
   char* str1;
};

void printObj(struct obj myObj){
   printf("Printing new obj\n");
   printf("int1: %d \n", myObj.int1);
   printf("str1: %s \n", myObj.str1);
}

struct sampleData
{
   char* str1;
   char* str2;
   int int1;
   int int2;
   double float1;
   double float2;
   double float3; // this is LAST in the json file but should be parsed here as a test
   int* arrInt;
   int arrIntLen;
   struct obj* arrObj;
   int arrObjLen;
};

void printSampleData(struct sampleData myData){
   printf("Printing new sample data\n");
   printf("str1: %s \n", myData.str1);
   printf("str2: %s \n", myData.str2);
   printf("int1: %d \n", myData.int1);
   printf("int2: %d \n", myData.int2);
   printf("float1: %f \n", myData.float1);
   printf("float2: %f \n", myData.float2);
   printf("float3: %f \n", myData.float3);

   printf("arrInt: ");
   int i;
   for(int i = 0; i < myData.arrIntLen; i++){
      printf("%d ", myData.arrInt[i]);
   }
   printf("\n");

   printf("arrObj: ");
   for(int i = 0; i < myData.arrObjLen; i++){
      printObj(myData.arrObj[i]);
   }
   printf("\n");
}

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

   //now can actually parse the json
   cJSON *jObj = cJSON_Parse(fileStr); // create the json object 
 

   free(fileStr); // free the file string when done
}

int main() {
   readJson("testCase.json");

   return 0;
}