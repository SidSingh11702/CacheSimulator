/****************
  
* Authors : Akshat Meena, Siddharth Singh, Muskan Jeph

* Purpose : Cache Simulator

****************/

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include "stdc++.h"
#include <cstdlib>
#include <time.h>
using namespace std;

// Structure of CacheBlock
struct cacheBlock{

    int tag;    // tag value
    bool valid;     // valid bit
    bool dirty;     // dirty bit

    struct cacheBlock *next;    // pointer to the next cacheblock
};

/* Definition of Cache class begins */

class Cache{

private:

    cacheBlock** Set;       // Cache
    int cache_size;     // Size of cache (in bytes)
    int block_size;     // Size of one cache block (in bytes)
    int cache_type;     // Type of cache (0 for fully associative, 1 for direct-mapped, 2/4/8/16/32 for set-associative)
    int replacement_type;       // Type of replacement policy (0 for random, 1 for LRU, 2 for Pseudo-LRU)
    vector<vector<int> > PLRU;     // For tree-based Pseudo-LRU implementation

public:

    int read_access;        // no of read accesses
    int write_access;       // no of write accesses
    int read_misses;        // no of read misses
    int write_misses;       // no of write misses
    int dirty_evicted;      // no of dirty blocks evicted
    int capacity_misses;    // no of capacity misses
    int conflict_misses;    // no of conflict misses

    Cache(int cache_size, int block_size, int cache_type, int replacement_type);    // Constructor
    ~Cache();                               //Destructor
    void Add_block(int tag, int index);     // Adds a block into cache
    void Write(int tag, int index);     // Writes a block into cache
    void Read(int tag, int index);      // Reads a block from cache
    void Replacement(int tag, int index);       // Replaces a block in the cache using the appropriate replacement policies
    void Add_on_tree(int tag, int index);       // Adds a block into the tree for pseudo LRU policies
    void Evict_on_tree(int new_tag, int index);     // Evicts a block from the tree and cache according to the pseudo LRU policy
};

/* Definition of Cache class ends */


/* Implementation of Cache class begins */

// Constructor
Cache::Cache(int cache_size, int block_size, int cache_type, int replacement_type){

    this->cache_size = cache_size;
    this->block_size = block_size;
    this->cache_type = cache_type;
    this->replacement_type = replacement_type;

    read_access = 0;
    write_access = 0;
    read_misses = 0;
    write_misses = 0;
    dirty_evicted = 0;
    capacity_misses = 0;
    conflict_misses = 0;

    int num_of_blocks = cache_size / block_size;

    if(cache_type == 0){
        Set = new cacheBlock*[1];
        Set[0] = NULL;

         PLRU.resize(1,vector<int>(2*num_of_blocks-1,0));   // initializing the tree for pseudo LRU

    }else if(cache_type == 1){
        Set = new cacheBlock*[num_of_blocks];
        for(int i=0; i<num_of_blocks ; i++){
            Set[i] = NULL;
        }

        PLRU.resize(num_of_blocks,vector<int>(1,0));    // initializing the tree for pseudo LRU

    }else{
        int num_of_sets = num_of_blocks / cache_type;
        Set = new cacheBlock*[num_of_sets];
        for(int i=0; i<num_of_sets; i++){
            Set[i] = NULL;
        }

        PLRU.resize(num_of_sets,vector<int>(2*cache_type-1,0));     // initializing the tree for pseudo LRU
    }
}

//Destructor
Cache::~Cache(){

    int no_of_blocks = cache_size / block_size;
    int no_of_sets;

    if(cache_type == 0){
        no_of_sets = 1;
    }else if(cache_type == 1){
        no_of_sets = no_of_blocks;
    }else{
        no_of_sets = no_of_blocks / cache_type;
    }

    for(int i=0; i<no_of_sets; i++)
        delete[] Set[i];

    delete[] Set;
}

// Adds a block into cache
void Cache::Add_block(int tag, int index){

    cacheBlock* new_block = new cacheBlock;

    new_block->tag = tag;
    new_block->valid = true;
    new_block->dirty = false;
    new_block->next = Set[index];
    Set[index] = new_block;

    // Add the block into the tree also (in case of pseudo LRU)
    if(replacement_type == 2){
        Add_on_tree(tag, index);
    }

    return;
}

// Writes a block into cache
void Cache::Write(int tag, int index){

    write_access++;     // increment the no. of write accesss

    bool hit = false;
    cacheBlock* temp = Set[index];

    // search for the block in the corresponding set of the cache
    if(Set[index] != NULL){

        if(temp->tag != tag){

            while(temp->next != NULL){

                if(temp->next->tag == tag){
                    temp->next = temp->next->next;
                    hit = true;
                    Add_on_tree(tag,index);
                    break;
                    //cache hit + update blocks order
                }
                temp = temp->next;
            }

            if(!hit){
                write_misses++;
                Replacement(tag, index);     // evict the block according to replacement policies
            }

        }else{
            temp->dirty = true;
            return;
        }
    }
    else
        write_misses++;

    Add_block(tag,index);
    Set[index]->dirty = true;       //set the dirty bit

    return;
}

// Reads a block from cache
void Cache::Read(int tag, int index){

    read_access++;      // increment the no. of read accesses

    bool hit = false;
    cacheBlock* temp = Set[index];

    // search for the block in the corresponding set of the cache
    if(Set[index] != NULL){

        if(temp->tag != tag){

            while(temp->next != NULL){

                if(temp->next->tag == tag){

                    temp->next = temp->next->next;
                    hit = true;
                    Add_on_tree(tag,index);
                    break;
                    //cache hit + update blocks order
                }

                temp = temp->next;
            }

            if(!hit){
                read_misses++;
                Replacement(tag, index); // evict the block according to replacement policies
            }
        }
        else
            return;
    }
    else
        read_misses++;

    Add_block(tag,index);
    return;
}

// Replaces a block in the cache using the appropriate replacement policies
void Cache::Replacement(int tag, int index){

    int associativity = cache_size / block_size;    //no. of blocks per set
    if(cache_type != 0)
        associativity = cache_type;

    int size=0;                                     //calculate the current number of valid blocks in the corresponding set
    cacheBlock* temp = Set[index];

    while(temp != NULL){
        size++;
        temp = temp->next;
    }

    if(size < associativity){                       //if current number of valid blocks are less than total number of blocks in the set
        Add_on_tree(tag,index);                     //update the PLRU tree
        return;                                     //don't evict any valid block and return
    }

    if(replacement_type == 0){

        /* Random policy */

        int evict = rand() % associativity;

        if(cache_type == 0)
            capacity_misses++;
        else if(cache_type != 1)
            conflict_misses++;

        temp = Set[index];

        for(int i=0; i<evict; i++)
            temp = temp->next;

        if(temp->dirty == true)
            dirty_evicted++;

        if(cache_type == 1)
            Set[index] = NULL;
        else{
            temp = Set[index];

            if(evict == 0)
                Set[index] = Set[index]->next;

            else{
                for(int i=0; i<evict-1;i++)
                    temp = temp->next;

                temp->next = temp->next->next;
            }
        }

    }
    else if(replacement_type == 1){

        /* LRU policy */

        if(cache_type == 0)
            capacity_misses++;
        else if(cache_type != 1)
            conflict_misses++;

        temp = Set[index];

        while(temp->next != NULL)
            temp = temp->next;

        if(temp->dirty == true)
            dirty_evicted++;

        if(cache_type == 1)
            Set[index]=NULL;
        else{
            temp = Set[index];
            while(temp->next->next != NULL)
                temp = temp->next;

            temp->next = NULL;
        }
    }
    else if(replacement_type == 2){

        /* Pseudo LRU policy */

        if(cache_type == 0)
            capacity_misses++;
        else if(cache_type != 1)
            conflict_misses++;

        Evict_on_tree(tag,index);       // Evict from tree, then from the cache
    }

    return;
}

// Adds a block into the tree for pseudo LRU policies
void Cache::Add_on_tree(int tag, int index){

    int associativity = cache_size / block_size;
    if(cache_type != 0) associativity = cache_type;

    int last_idx = 2*associativity - 2;
    int last_idx_of_second_last_level = associativity - 2;

    bool found = false;
    int idx = last_idx;

    // Search for the block in the tree
    while(idx > last_idx_of_second_last_level){

        if(PLRU[index][idx] == tag){
            found = true;
            break;
        }

        idx--;
    }

    if(found){

        /* If the block is present in the tree */
        /* From the node, go up to the root, and change the bits of the tree nodes accordingly */

        int parent_idx = (idx-1) / 2;

        while(idx > 0){

            if(2*parent_idx + 1 == idx)
                PLRU[index][parent_idx] = 1;
            else
                PLRU[index][parent_idx] = 0;

            idx = parent_idx;
            parent_idx = (idx-1)/2;
        }

    }else{

        /* If the bloock is not present in the tree */
        /* Start from the root, go down accordingly (left if 0, right if 1) */

        idx = 0;

        while(idx <= last_idx_of_second_last_level){

            if(PLRU[index][idx] == 0){
                PLRU[index][idx] = 1;
                idx = 2*idx + 1;
            }else{
                PLRU[index][idx] = 0;
                idx = 2*idx + 2;
            }

        }

        // Place the tag value there
        PLRU[index][idx] = tag;
    }

    return;
}

// Evicts a block from the tree and cache according to the pseudo LRU policy
void Cache::Evict_on_tree(int new_tag, int index){

    int associativity = cache_size / block_size;
    if(cache_type != 0) associativity = cache_type;

    int last_idx = 2*associativity - 2;
    int last_idx_of_second_last_level = associativity - 2;

    /* Start from the root, go down accordingly (left if 0, right if 1) */

    int idx = 0;

    while(idx <= last_idx_of_second_last_level){

            if(PLRU[index][idx] == 0){
                PLRU[index][idx] = 1;
                idx = 2*idx + 1;
            }else{
                PLRU[index][idx] = 0;
                idx = 2*idx + 2;
            }

        }

    int tag_to_be_evicted = PLRU[index][idx];   // the block to be evicted is this block

    // evict it from the cache
    if(Set[index]->tag == tag_to_be_evicted)
        Set[index] = Set[index]->next;
    else{

        cacheBlock* temp = Set[index];

        while(temp->next != NULL){

            if(temp->next->tag == tag_to_be_evicted){
                if(temp->next->dirty){
                    dirty_evicted++;
                }
                temp->next = temp->next->next;
                break;
            }

            temp = temp->next;
        }

    }

    PLRU[index][idx] = new_tag;     // put the new block in the tree
    return;
}

/* Implementation of Cache class begins */

/* Main function begins */

int main(){

    srand(time(0));

    //initialize all the variables needed

    unsigned int tag, index, offset;
    int tag_bits, index_bits, offset_bits;
    int cache_size, block_size, cache_type, replacement_type;
    string file_name;

    //Ask for user input

    cout<<"Enter the Cache Size (in Bytes) : ";
    cin>>cache_size;
    cout<<"Enter the Block Size (in Bytes) : ";
    cin>>block_size;
    cout<<"Enter the Cache Type (0 for Fully Associative,  1 for Direct Mapped, 2/4/8/16/32 for Set-Associative) : ";
    cin>>cache_type;
    cout<<"Enter the Replacement Policy (0 for Random, 1 for LRU, 2 for Pseudo-LRU) : ";
    cin>>replacement_type;
    cout<<"Enter the name of file containing memory traces : ";
    cin>>file_name;


    Cache cache(cache_size,block_size,cache_type,replacement_type);

    int num_of_blocks = cache_size / block_size;

    //Calculate the number of corresponding bits

    offset_bits = log2(block_size);

    if(cache_type == 0){
        index_bits = 0;
        index = 0;
    }else{
        int num_of_sets = num_of_blocks/cache_type;
        index_bits = log2(num_of_sets);
    }

    tag_bits = 32-offset_bits-index_bits;

    int offset_size = pow(2,offset_bits);
    int index_size = pow(2,index_bits);

    int bits = 0;
    ifstream traces(file_name);
    unsigned int block_address=0;
    char hex[9];
    bool write;

    //Read the memory traces from the file

    traces.ignore(2,'x');
    traces >> hex[bits];

    while(!traces.eof()){

        while(hex[bits]!='w' && hex[bits]!='r'){
            bits++;
            traces>>hex[bits];
        }

        int multiplier = pow(16, bits-1);

        //Hex to decimal converter

        for(int i=0; i<bits; i++){
            if(hex[i]<='9'){
                block_address += (hex[i]-48)*multiplier;
                multiplier = multiplier>>4;
            }
            else if(hex[i] >= 'A' && hex[i] <= 'Z'){
                block_address += (hex[i]-55)*multiplier;
                multiplier = multiplier>>4;
            }
            else{
                block_address += (hex[i]-87)*multiplier;
                multiplier = multiplier>>4;
            }
        }

        //Set the write bool according to the input
        if(hex[bits] == 'w')
            write = true;
        if(hex[bits] == 'r')
            write = false;

        //Extract the required bits from the memory trace
        offset = block_address % offset_size;
        index = (block_address>>offset_bits) % index_size;
        tag = (block_address>>(32-tag_bits));

        if(write)
            cache.Write(tag,index);
        else
            cache.Read(tag,index);

        block_address = 0;
        traces.ignore(2,'x');
        bits = 0;
        traces>>hex[bits];
    }

    traces.close();

    //Output the required data

    cout<<endl;
    cout << cache_size << "\t #Cache Size" << endl;
    cout << block_size << "  \t #Block Size" << endl;

    if(cache_type == 0)
        cout << "Fully Associative Cache" << endl;
    else if(cache_type == 1)
        cout << "Direct-Mapped Cache" << endl;
    else
        cout << cache_type << "-Way Set Associative Cache" << endl;

    if(replacement_type == 0)
        cout << "Random Replacement" << endl;
    else if(replacement_type == 1)
        cout << "LRU Replacement" << endl;
    else
        cout << "Pseudo-LRU Replacement" << endl;

    cout << cache.write_access+cache.read_access << "\t #Cache Access" << endl;
    cout << cache.read_access << "\t #Read Access" << endl;
    cout << cache.write_access << "\t #Write Access" << endl;
    cout << cache.write_misses+cache.read_misses <<"\t #Cache Misses" << endl;
    cout << cache.write_misses+cache.read_misses - cache.capacity_misses-cache.conflict_misses << "\t #Compulsory Misses" << endl;
    cout << cache.capacity_misses << "\t #Capacity Misses" << endl;
    cout << cache.conflict_misses << "\t #Conflict Misses" << endl;
    cout << cache.read_misses << "\t #Read Misses" << endl;
    cout << cache.write_misses << "\t #Write Misses" << endl;
    cout << cache.dirty_evicted << "\t #Dirty Blocks Evicted" << endl;

    return 0;
}

/* Main function ends */
