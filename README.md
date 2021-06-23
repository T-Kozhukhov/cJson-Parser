# cJson Parser
A basic implementation of Json for parsing of input data in simulations. 
Designed for use with C for the Shendruk Lab's MPCD code, but _should_ also work with C++ (although untested). 

Based entirely around cJson, designed to be lightweight and minimise fuss. 

## How to install to a project
Copy the following from the root directory of this repo to your project directory:
- The cJson folder **and all contents**
- cJson.c and cJson.h

Edit the `#include "..."` statements in cJson.c and cJson.h as necessary for your file placement, and edit your makefile to ensure proper linking.

## Usage
_This software is provided "as is", without warranty of any kind, express or implied_.

For an example of implementation of the parser see the method `int readJson` in main.c. 
The rough flow of this parser is important.
Some key points of interest:
- Lines 70 through 73 extract the json file into a format used by cJson.
- Line 78 sets up the cJson object, and the rest of the block does basic error checking.
- Lines 90 through 94 are necessary for the so called validation routines which will inform a user if they used an unknown json name (ie, json tag).
- Lines 96 through 105 get primitive data types. 
Note that while the int and double gets are standard, the string get requires a reference to the string variable you wish to write to.
Further, this string data is **dynamically allocated** and will need to be free'd later.
- Lines 107 through 115 handle an array of primitives. 
Note line 109 has a `type` argument at the end, which should be of value 0 for an array of primitives. 
Also note that you cannot parse the array using the gets included and have to use the `cJSON_GetArrayItem` method.
- Lines 117 through 127 handle an array of child objects.
Note that line 119 now takes argument `type` of value 1.
This is slightly different to arrays of primitives in the sense that, for each child object, you set up a cJson element pointer (as in line 123) and then can use the get methods on this element pointer.
- Line 130 performs input validation and must be called **after** parsing has been attempted.
- Finally memory can be clread as in lines 138-141.

## Credits
Uses the [cJson library](https://github.com/DaveGamble/cJSON) by Dave Gramble.