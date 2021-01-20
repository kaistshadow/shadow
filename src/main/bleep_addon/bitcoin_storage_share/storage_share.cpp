#include "storage_share.h"

#include <unordered_map>
#include <stdio.h>
#include <mutex>
#include <string>

#include <stdlib.h>

extern "C"
{

std::mutex _bitcoin_storage_share_FileInfotbl_m;
std::mutex _bitcoin_storage_share_NodeInfotbl_m;

typedef struct _Hashlist{
    char* actual_path;
    unsigned int refCnt;
}Hashlist;

typedef struct _HashTable{
    int listcnt;
    std::unordered_map<std::string, Hashlist> list;
}HashTable;

std::unordered_map<int, HashTable> FileInfotbl;

typedef struct _HashNodelist {
    char* actual_path;
    unsigned char* lastBlockhash;
}HashNodelist;

typedef struct _HashNodeTable{
    int lastFileNo;
    std::unordered_map<int,HashNodelist> list;
}HashNodeTable;
std::unordered_map<int, HashNodeTable> NodeInfotbl;


//해당 위치부터 사이즈 만큼 읽기(뒤집어 진것 똑바로 읽기)
void PrintHex(unsigned char* data, int start, int size, unsigned char* dest) {
    for (int i = 0; i <size; i++){
        if (dest)
            dest[i]= data[start+size-i-1];
    }
}

//해당 array print
void PrintArray(unsigned char* data,int size) {
    for (int i = 0; i <size; i++){
        printf("%02x ",data[i]);
    }
}

// input: 변환할 hex array, array 길이
// action: hex값을 그대로 decimal로 전환
// ouput: 변환된 unsigned int decimal
unsigned int hexToInt(unsigned char* data, int size, int endianness) {
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
unsigned int calcVarInt(unsigned char * data, int *bytepos) {

    unsigned int varint =0;
    unsigned char _data;
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

void datParser(unsigned char* dat, unsigned int size, unsigned char* lastBlockMerkleRoot) {

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

        unsigned char prevBlockHash[32];
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
void AddDataToHashTable(int fileno, char* path, unsigned char * merkleroothash, unsigned int nodeid) {
    FileInfotbl[fileno].listcnt++;
//    printf("[node %d] AddeHasData FileNo= %d / actual_path = %s\n",nodeid, fileno, path);
    AddHashData(fileno, path, merkleroothash);
    AddNodeHashData(nodeid, fileno, path,merkleroothash);
}


// AddHashData : [key]에 data 추가 -
void AddHashData(int fileno, char* actual_path, unsigned char* lastBlockHash){

    FileInfotbl.reserve(FileInfotbl.size()+1);

    Hashlist hashlist = { actual_path, 0};
    std::string blockhash(reinterpret_cast<char const *>(lastBlockHash),32);
    FileInfotbl[fileno].list[blockhash] = hashlist;

}

void AddNodeHashData(unsigned int nodeid, int fileno,char* actual_path, unsigned char* lastblockhash) {

    HashNodelist hashnodelist = {actual_path, lastblockhash};
    NodeInfotbl[nodeid].list[fileno] = hashnodelist;
    NodeInfotbl[nodeid].lastFileNo = fileno;

    std::string blockhash(reinterpret_cast<char const *>(lastblockhash),32);
    FileInfotbl[fileno].list[blockhash].refCnt++;
//    printf("[node %d] AddNodeHasData FileNo= %d / actual_path = %s\n",nodeid, fileno, actual_path);

}

//std::string getLastBlockHash(int fileno){
//    return FileInfotbl[fileno]
//    char* res = FileInfotbl->ents[key].list->lastBlockHashMerkleRoot;
//    return res;
//}

//void DeleteHashData(int key, char* actual_path){
//
//    if(hashtable->ents[key].list == NULL) {
//        return;
//    }
//
//    Hashlist* delNode = NULL;
//    if(hashtable->ents[key].list->actual_path == actual_path){
//        delNode = FileInfotbl->ents[key].list;
//        hashtable->ents[key].list = hashtable->ents->list->next;
//    }
//    else {
//        Hashlist *node = hashtable->ents[key].list;
//        Hashlist *next = node->next;
//
//        while (next) {
//            if (strcmp(next->actual_path ,actual_path) == 0) {
//                node->next = next->next;
//                delNode = next;
//                break;
//            }
//            node = next;
//            next = node->next;
//        }
//    }
//    free(delNode->lastBlockHashMerkleRoot);
//    free(delNode);
//}
//
//void DeleteNodeHashData(int key){
//    if(hashNodeTable->ents[key].list == NULL) {
//        return;
//    }
//    hashNodeTable->ents[key].lastFileNo=0;
//    free(hashNodeTable->ents[key].list);
//}
//
//void printHashTable(int key){
//    printf("[hash table : %d]\n",key);
//    HashNodelist * node = hashtable->ents[key].list;
//    printf("print hash table : %s \n",node->actual_path);
//    while (node->next) {
//        node = node->next;
//        printf("print hash table : %s \n",node->actual_path);
//    }
//}




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

    return NodeInfotbl[processID].list[fileno].actual_path;

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

int shadow_bitcoin_compare_dat_files(char* processName, unsigned int processID, int fileno) {
//    printf("process_emu_compare_dat_file test fileno is %d_%s \n",fileno,proc->processName->str);

    //cp_data file open
    char* path2 = shadow_bitcoin_get_tmp_file_path(processName);
    FILE* file2 = fopen(path2, "rb");

    fseek(file2, 0, SEEK_END);
    unsigned int size2 = ftell(file2);
    fseek(file2,0,SEEK_SET);

//    unsigned char merkleroothash[32];
    unsigned char* merkleroothash ;
    merkleroothash=(unsigned char*)malloc(sizeof(char)*32);
    unsigned char buf2[size2];
    fread(&buf2, sizeof(char), size2, file2);
    datParser(buf2, size2, merkleroothash);
    fclose(file2);

//    printf("compare_dat files file no : %d  /tmp file path : %s /last block hash: %02x \n ",fileno, path2, *merkleroothash);
    _bitcoin_storage_share_FileInfotbl_m.lock();
    std::unordered_map<int, HashTable>::const_iterator  res = FileInfotbl.find(fileno);
    if(res == FileInfotbl.end()){
        //data를 hash table에 추가
//        printf("COMPARE Result = %s make new file !! %d \n",processName, fileno);
        AddDataToHashTable(fileno,shadow_bitcoin_get_dat_file_path(processName,fileno),merkleroothash,processID);
        if(!shadow_bitcoin_copy_dat_files(processName,fileno)) {
            printf("cannot copy %d Dat file!\n",fileno);
        }

        _bitcoin_storage_share_FileInfotbl_m.unlock();
        return 1;// file is not exist, so make the file!
    }
    std::string blockhash(reinterpret_cast<char const *>(merkleroothash),32);
    std::unordered_map<std::string, Hashlist>::const_iterator listnode = res->second.list.find(blockhash);

    if(listnode == res->second.list.end()) { // 해당하는 해시가 없으면 , 즉 새로운 데이터 파일이 들어 왔을 때,
//        printf("COMPARE Result = %s file is NOT same!!!\n",path2);
        shadow_bitcoin_copy_dat_files(processName,fileno);
        AddDataToHashTable(fileno,shadow_bitcoin_get_dat_file_path(processName,fileno),merkleroothash,processID);
        FileInfotbl[fileno].list[blockhash].refCnt+=1;
        _bitcoin_storage_share_FileInfotbl_m.unlock();
        return 1;
    } else {
//        printf("COMPARE Result = %s and %s file is same!!!\n",path2,FileInfotbl[fileno].list[blockhash].actual_path);
        AddNodeHashData(processID, fileno, listnode->second.actual_path, merkleroothash);
        FileInfotbl[fileno].list[blockhash].refCnt += 1;
        NodeInfotbl[fileno].lastFileNo = fileno;
        _bitcoin_storage_share_FileInfotbl_m.unlock();
        return 1;
    }
}//compare_dat_files

}//extern c