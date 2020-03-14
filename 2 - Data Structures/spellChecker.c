/******************************************************************************
 * CS 261 Data Structures
 * Assignment 5 - Hash Table App
 * Name: Will Geller
 * Date: 3/11/2020
 * Description: Implement a spell checker utilizing the Hash Map ADT created.
 *              The user inputs a word (string with only characters, case 
 *              insensitive) and they are informed if the word was spelled correctly
 *              or incorrectly. If incorrect, the user is provided 5 suggested
 *              words they may have meant to type utilizing the Levenshtein Distance.
 *****************************************************************************/

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*** HELPER CODE *************************************************************
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}


/*** IMPLEMENT ***************************************************************
 * Loads the contents of dictionary.txt into the HashMap ADT.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    // FIXME: implement
    assert(file != NULL && map != NULL);
    
    // Asssign word to first word & set starting value to 0 for all words
    char* word = nextWord(file);
    int distance = 0;

    // Loop until end of file is reached
    while(word != NULL)
    {
        // Put word in map with distance value of 0
        hashMapPut(map, word, distance);
        
        // Free word and assign word to next word
        free(word);
        word = nextWord(file);
    }
}


/*** IMPLEMENT ***************************************************************
 * Checks user input and validates it contains a valid word. Strings that contain 
 * only alpha characters are considered valid words. 
 * @param userInput
 * @return boolean indicating if input is a word
*/
int isWord(const char* userInput)
{
    // Create loop control variables
    int ch = 0;
    int len = strlen(userInput);
    int alpha = 1;

    //Loop through string until end or non-alpha character is found
    while(ch < len && alpha)
    {
        alpha = isalpha(userInput[ch]);
        ch++;
    }
    // Returns true if string is all letters, false if it is not
    return alpha;
}


/*** IMPLEMENT ***************************************************************
 * Calculates the Levenshtein Distance between the user's word and each word in
 * the dictionary.
 * @param usrWord user input
 * @param mapWord word from dictionary
 * @return Levenshtein Distance between the two words
*/
int calcDistance(const char* usrWord, const char* mapWord)
{
    // Get the length of both words plus one for matrix dimensions and declare matrix
    int usrLen = strlen(usrWord) + 1;
    int mapLen = strlen(mapWord) + 1;
    int distance[usrLen][mapLen];
    
    // Initialize loop variables
    int i;
    int j;
    // Assign all indices of matrix to 0
    for(i = 0; i < usrLen; i++) 
    {
        for(j = 0; j < mapLen; j++)
        {
            distance[i][j] = 0;
        }
    }
    
    // Add the cost of adding/dropping each letter across first row & down first column
    for(i = 1; i <= usrLen; i++)
    {
        distance[i][0] = i;
    }
    for(j = 1; j < mapLen; j++)
    {
        distance[0][j] = j;
    }

    //Loop through remainder of matrix to calculate the cost of substitution, adding, or deleting 
    for(i = 1; i < usrLen; i++)
    {
        for(j = 1; j < mapLen; j++)
        {
            int subCost = 0;
            // If the two letters at the given index are not equal
            if(usrWord[i - 1] != mapWord[j - 1])
            {
                // Set subCost to 1
                subCost = 1;
            }
            
            // Calculate the cost of substituting, adding, and removing a letter
            int sub = distance[i - 1][j - 1] + subCost;
            int add = distance[i][j - 1] + 1;
            int rmv = distance[i - 1][j] + 1;
            
            //Set the value of the index to the minimum of substituting, adding, or removing
            if(sub <= rmv && sub <= add)
            {
                distance[i][j] = sub;
            }
            else if(add <= sub && add <= rmv)
            {
                distance[i][j] = add;
            }
            else
            {
                distance[i][j] = rmv;
            }
        }
    }

    // The Levenshtein Distance equals the value located at the last row, last col index
    return distance[usrLen - 1][mapLen - 1];
}


/*** IMPLEMENT ***************************************************************
 * Loops through the dictionary and calculate the Levenshtein Distance between
 * each word and the user's word. Five words with the lowest differences are
 * suggestions for misspelled word and returned to caller.
 * @param map
 * @param altWords array of HashLinks
 * @param usrWord user input
 * @return altWords, an array of 5 alternate words
 */
void spellCheck(HashMap* map, HashLink** altWords, const char* usrWord)
{
    // Set max to 0 and count to 0
    int max = 0;
    int count = 0;

    // Loop over each bucket
    int i;
    for(i = 0; i < map->capacity; i++)
    {
        //Create helper HashLink ptr to hold current link
        HashLink* curLink = map->table[i];

        // Traverse the bucket until link is NULL...
        while(curLink != NULL)
        {
            // Replace current key's value with the Levenshtein Distance
            hashMapPut(map, curLink->key, calcDistance(usrWord, curLink->key));
            
            // If five words are not in the array...
            if(count < 5)
            {
                // Add word to first available index and incremnent count
                altWords[count++] = curLink;

                // If the distance is greater than the max, set max to value
                if(curLink->value > max)
                {
                    max = curLink->value;
                }
            }
            // Else, the array is full and if the curLink value is less than max...
            else if(curLink->value < max)
            {
                // Find the HashLink with the max value and replace it with curLink
                int index = 0;
                int replaced = 0;
                while(index < count && !replaced)
                {
                    if(altWords[index]->value == max)
                    {
                        altWords[index] = curLink;
                        replaced = 1;
                    }
                    index++;
                }
                // Create loop control, reset max value, and recalculate
                int j;
                max = 0;
                for(j = 0; j < count; j++)
                {
                    if(altWords[j]->value > max)
                    {
                        max = altWords[j]->value;
                    }
                }
            }
            // Traverse to next link in bucket
            curLink = curLink->next;
        }
    }
}


/*** MAIN *********************************************************************
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    /*** HELPER CODE *************************************************************/
    HashMap* map = hashMapNew(1000);

    // Open dictionary file and load to hash map
    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    // Create input buffer and loop flag
    char inputBuffer[256];
    int quit = 0;
    
    /*** IMPLEMENT *************************************************************/
    while (!quit)
    {
        //Get input up to newline & discard newline
        printf("\nEnter a word or \"quit\" to quit: ");
        scanf("%[^\n]", inputBuffer);
        scanf("%*c");
        
        // Loop over string to make lowercase
        int i;
        for(i = 0; inputBuffer[i]; i++)
        {
            inputBuffer[i] = tolower(inputBuffer[i]);
        }

        // If the user typed quit, set quit flag to true
        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
        // If the user input characters and it is a word...
        else if(strlen(inputBuffer) && isWord(inputBuffer))
        {
            // If the word is spelled properly (HashMap has key)...
            if(hashMapContainsKey(map, inputBuffer))
            {
                // Inform user
                printf("The inputted word... is spelled correctly\n");
            }
            // Else, the word is spelled improperly...
            else
            {
                // Inform the user
                printf("The inputted word... is spelled incorrectly\n");
                
                // Allocate memory for an array to hold 5 HashLinks
                HashLink** alts = malloc(sizeof(HashLink*) * 5);
                
                //Calculate the Levenshtein Distance of all numbers and display suggestions
                spellCheck(map, alts, inputBuffer);
                printf("Did you mean ");
                printf("%s, %s, %s, %s, or %s?\n", alts[0]->key, alts[1]->key, alts[2]->key, alts[3]->key, alts[4]->key);
                
                // Free allocated memory and set to NULL
                free(alts);
                alts = NULL;
            }
        }
        // If the string entered contains non-alpha chars, inform the user
        else
        {
            printf("The input contained no characters or invalid characters. You may only enter letters.\n");
        }
    }
    // Delete HashMap dictionary & array of HashLinks
    hashMapDelete(map);
    
    return 0;
}