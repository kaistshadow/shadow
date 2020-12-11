#include "storage_share.h"

#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include <unordered_set>


//해당 위치부터 사이즈 만큼 읽기(뒤집어 진것 똑바로 읽기)
void PrintHex(char* data, int start, int size, char* dest) {
    for (int i = 0; i <size; i++){
        if (dest)
            dest[i]= data[start+size-i-1];
    }
}

//해당 array print
void PrintArray(char* data,int size) {
    for (int i = 0; i <size; i++){
        printf("%02x",data[i]);
    }
}

// input: 변환할 hex array, array 길이
// action: hex값을 그대로 decimal로 전환
// ouput: 변환된 unsigned int decimal
unsigned int hexToInt(char* data, int size, int endianness) {
    int res=0;
    if(big_endian) {
        for(int i=0;i<size;i++) {
            res = (res << 8) + data[i];
        }
    } else {
        for (int i=size-1; i>-1; i--) {
            res = (res << 8) + data[i];
        }
    }
    return res;
}

//input :data, variable 의 memory 주소값을 늘릴 주소
//action: var int 를 계산법에 따라서 계산
//output: 계산된 unsigned int
unsigned int calcVarInt(char * data, int *bytepos) {

    unsigned int varint =0;
    char _data;
    PrintHex(data, 0, 1, &_data);
    unsigned int data_num = _data;
    unsigned int fc_num = 0xfc;

    if(data_num <= fc_num){
        varint=0;
    }
    else {
        varint= data_num-fc_num;
    }

    unsigned int tx_cnt = 0;
    if(varint) {
        int readMoreByte=1;
        while(varint>0) {
            readMoreByte *= 2;
            varint--;
        }
        *bytepos += 1;
        tx_cnt = hexToInt(data, readMoreByte, LITTLE_ENDIAN);
        *bytepos += readMoreByte;
    } else {
        // dat[bytepos] is count itself
        tx_cnt = *data;
        *bytepos += 1;
    }

    return tx_cnt;

}

void datParser(char* dat, unsigned int size, char* lastBlockMerkleRoot) {

    int blockNum = 0;
    int byteIdx = 0;
    byteIdx += 4;   //magic byte

    // calcuate size from SIZE: 4 Byte
    unsigned int blockSetSize = 0;
    for(int i=3; i>-1; i--) {
        blockSetSize = (blockSetSize << 8) + dat[byteIdx+i];
    }
    byteIdx += 4;//block size


    while(size>=blockSetSize+byteIdx) {


        // block header
        byteIdx += 4;  //version

        char prevBlockHash[32];
        PrintHex(dat,byteIdx,32,prevBlockHash);
        byteIdx+=32;// previous block hash

        PrintHex(dat,byteIdx,32,lastBlockMerkleRoot);
        byteIdx += 32; //merkelroothash

        byteIdx += 4;   //time
        byteIdx += 4;   //bits
        byteIdx += 4; // nonce ------->finish block header

        //for debug
//        printf("[block%d Parse Result]\n",blockNum);
//        printf("[1. block Size] %d \n",blockSetSize);
//        printf("[2. prevBlockHash] ");
//        PrintArray(prevBlockHash, 32);
//        printf("\n");
//        printf("[3. merkleRootHash] block %d / ",blockNum);
//        PrintArray(prevBlockHash, 32);
//        printf(" / ");
//        PrintArray(lastBlockMerkleRoot,32);
//        printf("\n");


        // tx count: VarInt
        unsigned int tx_cnt= calcVarInt(&dat[byteIdx],&byteIdx);
//        printf("[4. Tx count ] %d \n",tx_cnt);
        for(unsigned int i = 0; i < tx_cnt; i++) {
            //check version
            unsigned int txVersion = hexToInt(&dat[byteIdx], 4, 0);
            byteIdx+=4;

            //If transaction version is 1 (only using in genesis block)
            if ( txVersion == 1 ) {

                //Input
                unsigned int input_cnt = calcVarInt(&dat[byteIdx],&byteIdx);//input count

//                printf("[5. Tx version ] %d \n",txVersion);
//                printf("[6. Input count ] %d \n",input_cnt);
                for(unsigned int i = 0; i < input_cnt; i++) {
                    byteIdx+=32;    //txid
                    byteIdx+=4;     //vout
                    unsigned int scriptsig_size = calcVarInt(&dat[byteIdx],&byteIdx);
                    byteIdx+=scriptsig_size;    //scriptsig
//                    printf("[7. Input script byte size %d ] %d \n",i,scriptsig_size);
                }

                byteIdx+=4;     //sequence

                //output
                unsigned int output_cnt=calcVarInt(&dat[byteIdx],&byteIdx);//output count
//                printf("[8. Output  count] %d \n",output_cnt);
                for(unsigned int i=0;i<output_cnt;i++){
                    byteIdx+=8;    //value
                    unsigned int scriptsig_size = calcVarInt(&dat[byteIdx],&byteIdx);
                    byteIdx+=scriptsig_size;    //scriptsig
//                    printf("[9. Output Script size %d] %d \n",i,scriptsig_size);
                }

                byteIdx+=4;     //locktime

            }
                //if transaction version is 2 (using Segwit)
            else if (txVersion == 2) {
                byteIdx+=1; //makrer
                byteIdx+=1; //flag

                //Input
                unsigned int input_cnt= calcVarInt(&dat[byteIdx],&byteIdx);//input count
//                printf("[6. Input count ] %d \n",input_cnt);
                for(unsigned int i=0;i<input_cnt;i++){
                    byteIdx+=32;    //txid
                    byteIdx+=4;     //vout
                    unsigned int scriptsig_size = calcVarInt(&dat[byteIdx],&byteIdx);
                    byteIdx+=scriptsig_size;    //scriptsig
//                    printf("[7. Input script byte size %d ] %d \n",i,scriptsig_size);

                }
                byteIdx+=4;     //sequence

                //output
                unsigned int output_cnt = calcVarInt(&dat[byteIdx],&byteIdx);//output count
//                printf("[8. Output  count] %d \n",output_cnt);
                for(unsigned int i = 0 ;i<output_cnt;i++){
                    byteIdx+=8; // value
                    unsigned int scriptsig_size = calcVarInt(&dat[byteIdx],&byteIdx);
                    byteIdx+=scriptsig_size;    //scriptsig
//                    printf("[9. Output Script size %d] %d \n",i,scriptsig_size);
                }
                //output script witness & locktime
                byteIdx+=34;    //script_witness
                byteIdx+=4;     //locktime
            }
        }
        byteIdx += 4;
        // calcuate size from SIZE: 4 Byte
        for(int i=3; i>-1; i--) {
            blockSetSize = (blockSetSize << 8) + dat[byteIdx+i];
        }
        byteIdx += 4;
        blockNum +=1;
//        printf("\n");
    }
}
void AddDataToHashTable(int fileno, char* path, char * merkleroothash, int nodeid) {
    AddHashData(FileInfotbl, fileno, path, merkleroothash);
    AddNodeHashData(NodeInfotbl, nodeid, fileno, path);
}

//hj add for storage hashtable
#define MAX_NODE_CNT 2000
#define MAX_DATAFILE_CNT 2000

void createHashTables() {
    FileInfotbl = (HashTable*)malloc(sizeof(HashTable));
    FileInfotbl->ents = (HashTblEntry*)malloc(sizeof(HashTblEntry)*MAX_DATAFILE_CNT);
    // initialize hash table entries
    for(int i=0; i<MAX_DATAFILE_CNT; i++) {
        FileInfotbl->ents[i].listcnt = 0;
        FileInfotbl->ents[i].list = NULL;
    }

    NodeInfotbl = (HashNodeTable *)malloc(sizeof(HashNodeTable));
    NodeInfotbl->ents = (HashNodeTblEntry*)malloc(sizeof(HashNodeTblEntry)*MAX_NODE_CNT);
    // initialize hash node table entries
    for(int i=0; i<MAX_NODE_CNT; i++) {
        NodeInfotbl->ents[i].lastFileNo = 0;
        NodeInfotbl->ents[i].list = NULL;
    }
}

// AddHashData : [key]에 data 추가 -
void AddHashData(HashTable *hashtable, int fileno, char* actual_path, char* lastBlockHash){

    // list entry 생성
    Hashlist* elem = (Hashlist*)malloc(sizeof(Hashlist));
    elem->fileno = fileno;
    elem->actual_path = actual_path;
    elem->lastBlockHashMerkleRoot = (char*)malloc(sizeof(char)*32);
    memcpy(elem->lastBlockHashMerkleRoot, lastBlockHash, 32);
    elem->refCnt=0;

    // put elem to list header
    Hashlist* cursor = hashtable->ents[fileno].list;
    hashtable->ents[fileno].list = elem;
    elem->next = cursor;
    elem->prev = NULL;
    if (cursor)
        cursor->prev = elem;

    hashtable->ents[fileno].listcnt++;
}

void AddNodeHashData(HashNodeTable *hashNodeTable,unsigned int nodeid, int fileno,char* actual_path) {
    // list entry 생성
    HashNodelist* elem = (HashNodelist*)malloc(sizeof(HashNodelist));
    elem->fileno = fileno;
    elem->actual_path = actual_path;
    elem->nodeID = nodeid;

    // put elem to list header
    HashNodelist* cursor = hashNodeTable->ents[nodeid].list;
    hashNodeTable->ents[nodeid].list = elem;
    elem->next = cursor;
    elem->prev = NULL;
    if (cursor)
        cursor->prev = elem;

    hashNodeTable->ents[nodeid].lastFileNo = fileno;
}


char* getLastBlockHash(HashTable *hashtable, int key){
    char* res = FileInfotbl->ents[key].list->lastBlockHashMerkleRoot;
    return res;
}

void DeleteHashData(HashTable *hashtable, int key, char* actual_path){

    if(hashtable->ents[key].list == NULL) {
        return;
    }

    Hashlist* delNode = NULL;
    if(hashtable->ents[key].list->actual_path == actual_path){
        delNode = FileInfotbl->ents[key].list;
        hashtable->ents[key].list = hashtable->ents->list->next;
    }
    else {
        Hashlist *node = hashtable->ents[key].list;
        Hashlist *next = node->next;

        while (next) {
            if (strcmp(next->actual_path ,actual_path) == 0) {
                node->next = next->next;
                delNode = next;
                break;
            }
            node = next;
            next = node->next;
        }
    }
    free(delNode->lastBlockHashMerkleRoot);
    free(delNode);
}

void DeleteNodeHashData(HashNodeTable *hashNodeTable,int key){
    if(hashNodeTable->ents[key].list == NULL) {
        return;
    }
    hashNodeTable->ents[key].lastFileNo=0;
    free(hashNodeTable->ents[key].list);
}

void printHashTable(HashNodeTable *hashtable,int key){
    printf("[hash table : %d]\n",key);
    HashNodelist * node = hashtable->ents[key].list;
    printf("print hash table : %s \n",node->actual_path);
    while (node->next) {
        node = node->next;
        printf("print hash table : %s \n",node->actual_path);
    }
}


extern "C"
{

char* shadow_bitcoin_get_dat_file_path(char* processName,int fileno) {
    char *path;
    path=(char*)malloc(sizeof(char)*100);
    sprintf(path,"cp_data/dat_%d_%s.dat",fileno,processName);
    return path;
}

char* shadow_bitcoin_get_tmp_file_path(char* processName){
    char *path;
    path=(char*)malloc(sizeof(char)*100);
    sprintf(path,"cp_data/cp_dat_%s.dat",processName);
    return path;
}

char* shadow_bitcoin_get_actual_path(int processID,int fileno){

    HashNodelist *node = NodeInfotbl->ents[processID].list;
    while (node) {
        if(node->fileno==fileno) {
            return node->actual_path;
        }
        node = node->next;
    }
    return NULL;
}

int shadow_bitcoin_copy_dat_files(char* procName, int fileno) {
    char *read_path=shadow_bitcoin_get_tmp_file_path(procName);
    FILE *rfp = fopen(read_path, "rb");
    if(!rfp) {
        printf("ERROR: file is not open %s file.\n",read_path);
        return 0;
    }

    char path[100];
    sprintf(path,"cp_data/dat_%d_%s.dat",fileno,procName);
    FILE *wfp = fopen(path, "wb+");
    if(!wfp) {
        printf("ERROR: file is not open  %s file.\n",path);
        return 0;
    }
    char buf[1024];
    int readcnt;
    while(!feof(rfp)) {
        readcnt = fread(buf, sizeof(char), 1024, rfp);
        fwrite(buf, sizeof(char), readcnt, wfp);
    }
    fclose(rfp);
    fclose(wfp);

    return 1;
}

int shadow_bitcoin_compare_dat_files(char* processName, int processID, int fileno) {
//    printf("process_emu_compare_dat_file test fileno is %d_%s \n",fileno,proc->processName->str);

    //cp_data file open
    char* path2 = shadow_bitcoin_get_tmp_file_path(processName);
    FILE* file2 = fopen(path2, "rb");

    fseek(file2, 0, SEEK_END);
    unsigned int size2 = ftell(file2);
    fseek(file2,0,SEEK_SET);

    char merkleroothash[32];
    char buf2[size2];
    fread(&buf2, sizeof(char), size2, file2);
    datParser(buf2, size2, merkleroothash);
    fclose(file2);

    //if file is not exist, return 0;
    if(FileInfotbl == NULL) {
        createHashTables();
    }
    if(FileInfotbl->ents[fileno].list == NULL) {
//        printf("COMPARE Result = file %d  is not exist! make the new file!!\n",fileno);

        //data를 hash table에 추가
        AddDataToHashTable(fileno,shadow_bitcoin_get_dat_file_path(processName,fileno),merkleroothash,processID);
        return 0;// file is not exist, so make the file!
    }

    Hashlist * node = FileInfotbl->ents[fileno].list;
    while (node) {
        char* uniqueid = node->lastBlockHashMerkleRoot;
        if(memcmp(uniqueid, merkleroothash, 32) == 0) {
//            printf("COMPARE Result = %s and %s file is same!!!\n",path2,node->actual_path);
            AddNodeHashData(NodeInfotbl,processID,fileno,node->actual_path);
            return 1;
        }
        node = node->next;
    }
//        printf("COMPARE Result = %s and %s file is NOT same!!!\n",path2,FileInfotbl->ents[fileno].list->actual_path);
    AddDataToHashTable(fileno,shadow_bitcoin_get_dat_file_path(processName,fileno),merkleroothash,processID);
    return 0;

}

}