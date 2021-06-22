#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson/cJSON.h"

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

/* 
   Helper methods and structs to check if an element in the .json exists to the code or not
*/

// a dumb linked list for handling list of saved strings
typedef struct linkedList linkedList;
struct linkedList{
   linkedList *next; 
   char* str;
};

void dynAllocStr(const char *val, char **toReturn);

void initLL(linkedList **head){
   *head = (linkedList*) malloc(sizeof(linkedList));
   (*head)->next = NULL;
   dynAllocStr("", &((*head)->str)); // fill w blank string
}

void printLL(linkedList *head){
/*
   Dumb method to print each element in the linked list
   Useful for debugging
*/
   if (head == NULL) return;
   linkedList *curr = head;
   while(curr != NULL){
      printf("%s\n", curr->str);
      curr = curr->next;
   }
   
}

int compareList(linkedList *head, const char* val){
/*
   Returns 1 if val appears in the linkedList, 0 if not
*/

  linkedList *current = head;
  while (current != NULL){ // iterate through linked list until end
     if (!strcmp(current->str, val)){
        return 1; //if we find something w the correct val then return true
     }

     current = current->next; //iterate
  }

  return 0; // if you're here then it doesnt exist in the list
}

void push(linkedList * head, const char* val){
/*
   push a value to the end of the linked list
*/
   linkedList *curr = head;
   while (curr->next != NULL){
      curr = curr->next; //iterate to end of list
   }

   // alloc and add to end
   curr->next = (linkedList*) malloc(sizeof(linkedList));
   dynAllocStr(val, &curr->next->str);
   curr->next->next = NULL;
}

void freeLL(linkedList *head){
/*
   Free the specified linked list
*/
   if (head == NULL) return;

   linkedList *curr = head;
   do
   {
      linkedList *next = curr->next; //save for later
      free(curr->str);
      free(curr);
      curr = next;
   } while (curr != NULL); // specifically use a do-while loop here to free all

   return;
}

void verifyJson(cJSON *jObj, linkedList *jsonTagList, linkedList* arrayList){
/*
   A method to verify if there are any "unknown" json tags in the jObj.
   Method is recursive for arrays.
   It is assumed that all parsing is done BEFORE this, as that is necessary for
      the lists to be properly constructed
*/
   cJSON *childObj = NULL;
   cJSON_ArrayForEach(childObj, jObj){ // loop through children
      const char *jTag = childObj->string; // get json tag
      
      // check if this tag exists on the list of known objects
      if (!compareList(jsonTagList, jTag)){
         // throw a warning if it's not
         printf("JSON Read Warning: Found unrecognised json tag: %s. Tag will be ignored.\n", jTag);
      } else {
         // if tag exists, check if it is an array and verify the subarray if necessary
         if (compareList(arrayList, jTag)) {

            // assume the array is of a child objects, which need to 
            //    be manually parsed
            cJSON *arrayChild = NULL;
            cJSON_ArrayForEach(arrayChild, childObj){
               verifyJson(arrayChild, jsonTagList, arrayList);
            }
         }
      }
   }
}

/* 
   A variety of helper objects to read from Json and fill up a given data structure, and have some 
      mild segfault protection.
   These can be used to access elements within any given json object (ie, something surrounded by 
      {} )
   These SHOULD also be used to parse data from within array contained objects.
*/

cJSON getCJsonArray(cJSON *jObj, cJSON **toReturn, const char* val, 
   linkedList *jsonList, linkedList *arrayList, int type){
/*
   Set up a cJSON array ready for parsing, while adding to the necessary linked lists
   Type param: 0 for array of primitives, 1 for array of objects
*/
   *toReturn = cJSON_GetObjectItemCaseSensitive(jObj, val);
   push(jsonList, val);
   if (type)   push(arrayList, val); // if array of objects add to array list
}

int getJObjInt(cJSON *cJSONRoot, const char* jsonTag, int d, linkedList *head){
   /*
      Returns an integer object from the given cJSON file searching for a particular jsonTag.
      If no appropriate json tag is found then it will return default value d
   */
   push(head, jsonTag); // add jsonTag to head

   // cJson bits
   cJSON *jObj = cJSON_GetObjectItemCaseSensitive(cJSONRoot, jsonTag);
   if (jObj == NULL){ // check for non-existence
      return d; 
   }
   
   int buff = jObj->valueint; // make a buffer to return an appropriate val
   return buff;
}

double getJObjDou(cJSON *cJSONRoot, const char* jsonTag, double d, linkedList *head){
   /*
      Returns a double object from the given cJSON file searching for a particular jsonTag.
      If no appropriate json tag is found then it will return default value d
   */
   push(head, jsonTag); // add jsonTag to head

   // cJson bits
   cJSON *jObj = cJSON_GetObjectItemCaseSensitive(cJSONRoot, jsonTag);
   if (jObj == NULL){ // check for non-existence
      return d; 
   }
   
   double buff = jObj->valuedouble; // make a buffer to return an appropriate val
   return buff;
}

void dynAllocStr(const char *val, char **toReturn){
   /* 
      A helper function to dynamically allocate a string w value val to toReturn. You should be 
         passing it an object of form &myStr in the second arg.
   */
   int len = strlen(val) + 1; // length of memory to allocate
   *toReturn = malloc( len);
   if (*toReturn == NULL){ // stupid error checking
      printf("Failed to allocate memory for string: %s \n", val);
      return;
   }

   *toReturn = strcpy(*toReturn, val);
}

void getJObjStr(cJSON *cJSONRoot, const char* jsonTag, const char* d, char **toReturn, linkedList *head){
   /*
      NOTE: UNLIKE THE OTHER METHODS THIS IS NOT LEAK SAFE. ANYTHING YOU GET FROM THIS METHOD MUST 
         BE FREE'D TO BE LEAK FREE.
      Returns a string object from the given cJSON file searching for a particular jsonTag.
      If no appropriate json tag is found then it will return default value d
   */
   push(head, jsonTag); // add jsonTag to head

   // cJson bits
   cJSON *jObj = cJSON_GetObjectItemCaseSensitive(cJSONRoot, jsonTag);
   if (jObj == NULL){ // check for non-existence
      dynAllocStr(d, toReturn); // allocate the string dynamically
      return; 
   }
   
   const char* buff = jObj->valuestring; // make a buffer to return an appropriate val
   dynAllocStr(buff, toReturn); // allocate the string dynamically
   return;
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
   long numbytes = ftell(fptr) + 1;
   
   /* reset the file position indicator to 
   the beginning of the file */
   fseek(fptr, 0L, SEEK_SET);	
   
   /* grab sufficient memory for the 
   buffer to hold the text */
   *fileStr = (char*)calloc(numbytes, sizeof(char));	
   
   /* memory error */
   if(*fileStr == NULL){
      printf("Error allocating memory for file %s\n", inFile);
      return 2;
   }  
   
   /* copy all the text into the buffer */
   if (fread(*fileStr, sizeof(char), numbytes, fptr) == 0){
      printf("Could not read file %s\n", inFile);
      return 3;
   }

   fclose(fptr); // close file to free memory
   return 0;
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

   // set up "error checking" routines
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

   // TODO: verification stuff
   /*
      Need to do the following:
      - Go through the jObj item as a linked list (use a given method in the readme)
      - For each item, check the json tag:
         - If the json tag IS NOT on the jsonTagList linked list, throw an error for the user
            - Can do this using my method above
         - If the json tag is an array, then iterate within the array and do the same check
            - This is only down to a single level
      - Deallocate the linked list (as necessary, use valgrind)
   */

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