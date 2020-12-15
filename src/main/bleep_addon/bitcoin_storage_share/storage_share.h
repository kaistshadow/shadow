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
int shadow_bitcoin_compare_dat_files(char* processName, unsigned int processID, int fileno);

//hyeojin made for compare dat_files
#define big_endian 0
#define little_endian 1
void PrintHex(unsigned char* data, int start, int size, unsigned char* dest) ;
unsigned int hexToInt(unsigned char* data, int size, int endianness);

unsigned int calcVarInt(unsigned char * data, int *bytepos) ;
void datParser(unsigned char* dat, unsigned int size, unsigned char* lastBlockMerkleRoot);
void PrintArray(unsigned char* data,int size);


//method for storage, but not interposer method
void AddHashData(int fileno, char* actual_path, unsigned char* lastBlockHash);
//std::string getLastBlockHash(int fileno);
//void DeleteHashData(int key, char* actual_path);
//void DeleteNodeHashData(int key);
//void printHashTable(int key);

void AddNodeHashData(unsigned int nodeid,int fileno,char* path, unsigned char* lastblockhash);
void AddDataToHashTable(int fileno, char* path, unsigned char * merkleroothash, unsigned int nodeid);


#ifdef __cplusplus
}
#endif


#endif
