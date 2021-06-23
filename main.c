#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson.h"

// helper data structures to make my life easier
typedef struct
{
   int int1;
   char* str1;
} obj;

void printObj(obj myObj){
   printf("Printing new obj\n");
   printf("int1: %d \n", myObj.int1);
   printf("str1: %s \n", myObj.str1);
}

typedef struct
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
   obj* arrObj;
   int arrObjLen;
} sampleData;

void printSampleData(sampleData myData){
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
   for(i = 0; i < myData.arrIntLen; i++){
      printf("%d ", myData.arrInt[i]);
   }
   printf("\n");

   printf("arrObj: ");
   for(i = 0; i < myData.arrObjLen; i++){
      printObj(myData.arrObj[i]);
   }
   printf("\n");
}

void freeSampleData(sampleData myData){
   free (myData.str1);
   free (myData.str2);
   int i; 
   // for (i = 0; i < myData.arrIntLen; i++) free(myData.arrObj[i].str1);
}

int readJson(char* inFile){   
   /*
      Reads the json file specified by inFile.
   */
   char* fileStr = NULL;
   if(getFileStr(inFile, &fileStr) != 0){ // read, return on error
      return 1;
   } 
   printf("==== Read JSON =====\n%s", fileStr);
   printf("\n\n");

   //now can actually parse the json
   cJSON *jObj = cJSON_Parse(fileStr); // create the json object 
   sampleData myData; // create the data object we hope to fill
   if (jObj == NULL) // error checking
   {
      const char *error_ptr = cJSON_GetErrorPtr();
      if (error_ptr != NULL)
      {
         fprintf(stderr, "Json read error. \nError before: %s\n", error_ptr);
      }
      return 1;
   }

   // set up input validation routines
   linkedList *jsonTagList = NULL;
   initLL(&jsonTagList);
   linkedList *arrayList = NULL;
   initLL(&arrayList);

   // get strings 
   getJObjStr(jObj, "string", "", &myData.str1, jsonTagList);
   getJObjStr(jObj, "string2", "", &myData.str2, jsonTagList);

   // get primitives
   myData.int1 = getJObjInt(jObj, "int", -999999, jsonTagList);
   myData.int2 = getJObjInt(jObj, "int2", -999999, jsonTagList);
   myData.float1 = getJObjDou(jObj, "float", -999999.99, jsonTagList);
   myData.float2 = getJObjDou(jObj, "float2", -999999.99, jsonTagList);
   myData.float3 = getJObjDou(jObj, "float3", -999999.99, jsonTagList);

   // get int array (can't do custom methods for this, have to parse manually)
   cJSON *arrInt = NULL;
   getCJsonArray(jObj, &arrInt, "arrayInt", jsonTagList, arrayList, 0);
   myData.arrIntLen = cJSON_GetArraySize(arrInt); // get array size for parsing
   int i;
   int intBuff[myData.arrIntLen]; // create a temp buffer to hold the int array data items
   for (i = 0; i < myData.arrIntLen; i++) 
      intBuff[i] = cJSON_GetArrayItem(arrInt, i)->valueint; //get the individual int array items
   myData.arrInt = intBuff; // write to data

   // get obj array (again, have to parse manually but handle the objects differently)
   cJSON *arrObj = NULL;
   getCJsonArray(jObj, &arrObj, "arrayObj", jsonTagList, arrayList, 1);
   myData.arrObjLen = cJSON_GetArraySize(arrObj); // get array size for parsing
   obj objBuff[myData.arrObjLen]; // create a temp buffer to hold the int array data items
   for (i = 0; i < myData.arrObjLen; i++) {
      cJSON *objElem = cJSON_GetArrayItem(arrObj, i); // get the individual cjson object of this elem
      objBuff[i].int1 = getJObjInt(objElem, "int", -999999, jsonTagList);
      getJObjStr(objElem, "str", "", &objBuff[i].str1, jsonTagList);
   }
   myData.arrObj = objBuff; // write to data

   // input verification step
   verifyJson(jObj, jsonTagList, arrayList);
   printf("\n");

   // handle sample data
   printSampleData(myData); // print the sample data at the end for verification
   freeSampleData(myData);  // free

   // memory cleanup
   cJSON_Delete(jObj); // free the json object
   free(fileStr); // free the file string when done
   freeLL(jsonTagList); // free the linked lists
   freeLL(arrayList);
}

int main() {
   readJson("testCase.json");

   return 0;
}