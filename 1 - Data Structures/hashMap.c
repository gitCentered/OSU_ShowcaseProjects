/******************************************************************************
 * CS 261 Data Structures
 * Assignment 5 - Hash Table App
 * Name: Will Geller
 * Date: 3/11/2020
 * Description: Complete Hash Map ADT by implementing functions that are labeled
 *              IMPLEMENT. Functions labeled HELPER CODE were provided by the
 *              instructor.
 *****************************************************************************/

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

/*** HELPER CODE *************************************************************/
int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}


/*** HELPER CODE *************************************************************/
int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}


/*** HELPER CODE *************************************************************
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}


/*** HELPER CODE *************************************************************
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}


/*** HELPER CODE *************************************************************
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}


/*** HELPER CODE *************************************************************
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}


/*** IMPLEMENT ***************************************************************
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param maps
 */
void hashMapCleanUp(HashMap* map)
{
    assert(map != NULL);

    // Loop over each bucket to delete all HashLinks
    int i;
    for(i = 0; i < map->capacity; i++)
    {
        // Create helper HashLink ptr and assign to first link in bucket
        HashLink* garbage = map->table[i];
            
        // Traverse bucket to delete all HashLinks
        while(garbage != NULL)
        {
            // Assign first bucket HashLink to next link
            map->table[i] = garbage->next;
                
            // Delete garbage HashLink and assign garbage to bucket start
            hashLinkDelete(garbage);
            garbage = map->table[i];
            
            // Decrement size
            map->size--;
        }
    }
    //Free memory allocated for table
    free(map->table);
}


/*** HELPER CODE *************************************************************
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/*** IMPLEMENT ***************************************************************
 * Returns a pointer to the value of the link with the given key and skip 
 * traversing as well. Returns NULL if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    assert(map != NULL); 
    assert(key != NULL);
    
    // Get index for the given key
    int index = HASH_FUNCTION(key) % map->capacity;
    
    // Create helper HashLink ptr and assign to first link of bucket
    HashLink* curLink = map->table[index];
    
    // Traverse bucket  until curLink is NULL
    while(curLink != NULL)
    {
        // If curLink's key matches given key...
        if(strcmp(curLink->key, key) == 0)
        {
            // Return reference to the value
            return &(curLink->value);
        }
        // Else, assign curLink to next HashLink
        else
        {
            curLink = curLink->next;
        }
    }
    // If key is not found, return NULL
    return NULL;
}

/*** IMPLEMENT ***************************************************************
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    // FIXME: implement
    assert(map != NULL);
    
    // Create temp table to hold original values
    HashMap* tempMap = malloc(sizeof(HashMap));
    tempMap->table = map->table;
    tempMap->capacity = map->capacity;
    tempMap->size = map->size;

    // Initialize originally allocated hash map with new size
    hashMapInit(map, capacity);

    // Loop over each bucket of the temp map
    int i;
    for(i = 0; i < tempMap->capacity; i++)
    {
        // Create helper HashLink ptr and assign to first link of bucket
        HashLink* curLink = tempMap->table[i];

        // Traverse the HashLinks in the bucket until NULL...
        while(curLink != NULL)
        {
            // Put HashLink in the new map and assign helper to the next link
            hashMapPut(map, curLink->key, curLink->value);
            curLink = curLink->next;
        }
    }
    // Free links, table, and HashMap associated with tempMap
    hashMapDelete(tempMap);
}

/*** IMPLEMENT ***************************************************************
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    assert(map != NULL);
    assert(key != NULL);

    // If table load exceeds maximum, double table size
    if(hashMapTableLoad(map) >= MAX_TABLE_LOAD)
    {
        resizeTable(map, 2 * hashMapCapacity(map));
    }

    // Get index for the given key
    int index = HASH_FUNCTION(key) % map->capacity;

    // Create helper HashLink and assign to first link in bucket
    HashLink* curLink = map->table[index];

    // If curLink is NULL, add new value as first link in bucket
    if(curLink == NULL)
    {
        map->table[index] = hashLinkNew(key, value, NULL);
        map->size++;
    }
    // Else, traverse the bucket's chain to get location to put key value pair
    else
    {
        // Create loop control
        int put = 0;    

        // Traverse bucket until the key value pair is put into HashMap
        while(put == 0)
        {
            // If curLink's key equals the given key, replace the value
            if(strcmp(curLink->key, key) == 0)
            {
                curLink->value = value;
                put = 1;  // Change flag value to true
            }
            // If curLink's next ptr is NULL, create new link at curLink's next
            else if(curLink->next == NULL)
            {
                curLink->next = hashLinkNew(key, value, NULL);
                map->size++;
                put = 1;  // Change flag value to true
            }
            // Else, traverse to next link in bucket
            else
            {
                curLink = curLink->next;
            }
        }   
    }
}

/*** IMPLEMENT ***************************************************************
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    assert(map != NULL);
    assert(key != NULL);
    
    // Get index for the given key
    int index = HASH_FUNCTION(key) % map->capacity;

    // Create helper HashLink ptr to hold current link
    HashLink* curLink = map->table[index];
    
    // If curLink is not NULL and its key matches the given key...
    if(curLink != NULL && (strcmp(curLink->key, key) == 0))
    {
        // Assign first link in bucket to next, delete curLink, & decrement map size
        map->table[index] = curLink->next;
        hashLinkDelete(curLink);
        map->size--;
    }
    // If curLink is not NULL...
    else if(curLink != NULL)
    {
        // Create helper HashLink ptr to hold previous link & assign curLink to next
        HashLink* prev = curLink;
        curLink = curLink->next;
        int rmvFlag = 0;  // Flag indicating removal
        
        // Traverse bucket until curLink is NULL or HashLink removed
        while(curLink != NULL && !rmvFlag)
        {
            // If curLink's key matches given key...
            if(strcmp(curLink->key, key) == 0)
            {
                // Assign prev's next to curLink's next & delete curLink
                prev->next = curLink->next;
                hashLinkDelete(curLink);

                // Decrement size and set remove flag
                map->size--;
                rmvFlag = 1;
            }
            //If key does not match curLink's key...
            else
            {
                // Assign curLink to prev and curLink to curLink's next
                prev = curLink;
                curLink = curLink->next;
            }
        }
    }
}


/*** IMPLEMENT ***************************************************************
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    assert(map != NULL);
    assert(key != NULL);
    
    // Get index for the given key
    int index = HASH_FUNCTION(key) % map->capacity;

    // Create helper HashLink ptr and assign to first link in bucket
    HashLink* curLink = map->table[index];

    // Traverse bucket until curLink's key matches given key or curLink is NULL
    while(curLink != NULL)
    {
        // If key matches given key, return true
        if(strcmp(curLink->key, key) == 0)
        {
            return 1;
        }
        // Else, traverse to next link
        else
        {
            curLink = curLink->next;
        }
    }
    // Key is not found, return false
    return 0;
}

/*** IMPLEMENT ***************************************************************
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    assert(map != NULL);
    return map->size;
}

/*** IMPLEMENT ***************************************************************
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    assert(map != NULL);
    return map->capacity;
}

/*** IMPLEMENT ***************************************************************
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    assert(map != NULL);
    
    // Create accumulator variable and loop control
    int empty = 0;
    int b;

    // Loop over the hash map buckets and increment accumulator if empty
    for(b = 0; b < map->capacity; b++)
    {
        if(map->table[b] == NULL)
        {
            empty++;
        }
    }
    return empty;
}

/*** IMPLEMENT ***************************************************************
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    assert(map != NULL);
    return (map->size)/(map->capacity);
}

/*** IMPLEMENT ***************************************************************
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    assert(map != NULL);

    // Loop over each bucket
    int i;
    printf("\n");
    for(i = 0; i < map->capacity; i++)
    {
        // Create helper HashLink ptr and assign to first link of bucket
        HashLink* pLink = map->table[i];
        
        // Print bucket number
        printf("%d - ", i);

        // Traverse the bucket's chain...
        while(pLink != NULL)
        {
            // Print key value pair & set pLink to next
            printf("{%s, %d} ", pLink->key, pLink->value);
            pLink = pLink->next;
        }
        printf("\n");
    }
}