/*
   Helper code to set up JSON parsers in C.
   Uses the cJson library by Dave Gramble released under MIT license.
   Started by Timofey Kozhukhov, Summer 2021.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJson.h"

// list of exclusions for "comment" tags
char* commentTags[] = {"c", "comment", "//", "#"};
const int commentTagCount = 4;

/* 
   Helper methods and structs to check if an element in the .json exists to the code or not
*/

void initLL(linkedList **head){
/*
    Initialise a linked list for usage, at the head (expected to be a NULL
        pointer passed as &head)
*/
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

int isComment(const char* tag){
   // Returns 1 if tag appears in the comment list, 0 if not
   int i; // counter
   for (i = 0; i < commentTagCount; i++) {
      if (strcmp(tag, commentTags[i])) return 1;
   }

   return 0; // if you get here then the tag isn't in the comment list
}

void pushLL(linkedList * head, const char* val){
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
   int validJson = 1; // flag on whether the JSON is valid or not.

   cJSON_ArrayForEach(childObj, jObj){ // loop through children
      const char *jTag = childObj->string; // get json tag
      
      // check if this tag exists on the list of known objects
      if (!compareList(jsonTagList, jTag)){
         if (!isComment(jTag)){ // check if the tag is a comment
            // throw a warning if it's not
            printf("JSON Read Warning: Found unrecognised json tag: %s. Tag will be ignored.\n", jTag);
            validJson = 0;
         }
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

   if (!validJson){
      printf("JSON Read Error: Errors found.\n");
      exit(EXIT_FAILURE);
   }
}

/* 
   A variety of helper objects to read from Json and fill up a given data structure, and have some 
      mild segfault protection.
   These can be used to access elements within any given json object (ie, something surrounded by 
      {} )
   These SHOULD also be used to parse data from within array contained objects.
*/

void getCJsonArray(cJSON *jObj, cJSON **toReturn, const char* val, 
   linkedList *jsonList, linkedList *arrayList, int type){
/*
   Set up a cJSON array ready for parsing, while adding to the necessary linked lists
   Type param: 0 for array of primitives, 1 for array of objects
*/
   *toReturn = cJSON_GetObjectItemCaseSensitive(jObj, val);
   pushLL(jsonList, val);
   if (type)   pushLL(arrayList, val); // if array of objects add to array list
}

int getJObjInt(cJSON *cJSONRoot, const char* jsonTag, int d, linkedList *head){
   /*
      Returns an integer object from the given cJSON file searching for a particular jsonTag.
      If no appropriate json tag is found then it will return default value d
   */
   pushLL(head, jsonTag); // add jsonTag to head

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
   pushLL(head, jsonTag); // add jsonTag to head

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
   pushLL(head, jsonTag); // add jsonTag to head

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