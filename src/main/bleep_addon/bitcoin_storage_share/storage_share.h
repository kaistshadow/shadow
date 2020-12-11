#ifndef BLEEP_BITCOIN_STORAGE_SHARE_H
#define BLEEP_BITCOIN_STORAGE_SHARE_H



#ifdef __cplusplus
extern "C"
{
#endif

char* shadow_bitcoin_get_dat_file_path(char* processName, int fileno);
char* shadow_bitcoin_get_tmp_file_path(char* processName);
char* shadow_bitcoin_get_actual_path(int processID, int fileno);

int shadow_bitcoin_copy_dat_files(char* procName, int fileno);
int shadow_bitcoin_compare_dat_files(char* processName, int processID, int fileno);

//hyeojin made for compare dat_files
#define big_endian 0
#define little_endian 1
void PrintHex(char* data, int start, int size, char* dest) ;
unsigned int hexToInt(char* data, int size, int endianness);
unsigned int calcVarInt(char * data, int *bytepos) ;
void datParser(char* dat, unsigned int size, char* lastBlockMerkleRoot);

//hyeojin made for storage hash table
typedef struct _Hashlist{
    struct _Hashlist *next, *prev;
    int fileno;
    char* actual_path;
    char* lastBlockHashMerkleRoot;
    unsigned refCnt;

}Hashlist;

typedef struct _HashTblEntry{
    Hashlist* list;
    int listcnt;
}HashTblEntry;

typedef struct _HashTable{
    HashTblEntry* ents;
}HashTable;

//structure for hashNodetable
typedef struct _HashNodelist {
    struct _HashNodelist *next, *prev;
    unsigned int nodeID;
    char *actual_path;
    int fileno;
}HashNodelist;
typedef struct _HashNodeTblEntry{
    HashNodelist* list;
    int lastFileNo;
}HashNodeTblEntry;

typedef struct _HashNodeTable{
    HashNodeTblEntry* ents;
}HashNodeTable;

HashTable *FileInfotbl;
HashNodeTable *NodeInfotbl;

//method for storage, but not interposer method
void createHashTables();
void AddHashData(HashTable *hashTable, int fileno, char* actual_path, char* lastBlockHash);
char* getLastBlockHash(HashTable *hashTable, int fileno);
void DeleteHashData(HashTable *hashTable,int key, char* actual_path);
void DeleteNodeHashData(HashNodeTable *hashNodeTable,int key);
void printHashTable(HashNodeTable *hashtable,int key);

void AddNodeHashData(HashNodeTable *hashNodeTable,unsigned int nodeid,int fileno,char* path);
void AddDataToHashTable(int fileno, char* path, char * merkleroothash, int nodeid);


#ifdef __cplusplus
}
#endif


#endif
