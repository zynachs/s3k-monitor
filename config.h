// Configuration file for project to assist in testing

//----------------------------------------------------

/* App tests
   Test code in app/app.c:test and app/test.c:*
   Instructions:
   - SET the line which matches the tests you wish to run.
   - DO NOT SET multiple lines simultaneously.
   - If no tests are to be run, please SET the first line.
*/

#define TEST_MASK 0x0
//#define TEST_MASK 0x000000000000001E // test 1-4
//#define TEST_MASK 0x00000000000000E0 // test 5-7
//#define TEST_MASK 0x0000000000000300 // test 8-9

//----------------------------------------------------

/* Monitor security 
   SET = security enabled
   UNSET = security disabled
*/

#define SECURITY

//----------------------------------------------------

/* App signature 
   - SET = App signature purposely created to be invalid
   - UNSET = App signature valid
*/

//#define SIG_BROKEN

//----------------------------------------------------
