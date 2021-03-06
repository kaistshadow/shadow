#include "bleep_addon.h"

#include <unordered_set>
#include <mutex>
#include <fstream>
#include <iostream>

/* bitcoin coinflip validation */
std::unordered_set<std::string> _bitcoin_coinflip_validation_table;
std::mutex _bitcoin_coinflip_validation_table_m;

extern "C"
{

void shadow_bitcoin_register_hash(const char hash[], int reindex) {
    _bitcoin_coinflip_validation_table_m.lock();
    _bitcoin_coinflip_validation_table.insert(hash);
    _bitcoin_coinflip_validation_table_m.unlock();
    if(reindex == 0){
      std::ofstream file("./data/coinflip_hash.txt",std::ios_base::out | std::ios_base::app);
      file<<hash<<std::endl;
      file.close();
    }
    return;
}
int shadow_bitcoin_check_hash(const char hash[]) {
  _bitcoin_coinflip_validation_table_m.lock();
    std::unordered_set<std::string>::const_iterator got = _bitcoin_coinflip_validation_table.find(hash);
    int res = (got != _bitcoin_coinflip_validation_table.end());
  _bitcoin_coinflip_validation_table_m.unlock();
    return res;
}

void shadow_bitcoin_load_hash(){
  std::ifstream file ("./data/coinflip_hash.txt");
  if(file.is_open()){
    while(!file.eof()){
      std::string hash;
      getline(file, hash);
      _bitcoin_coinflip_validation_table_m.lock();
      _bitcoin_coinflip_validation_table.insert(hash);
      _bitcoin_coinflip_validation_table_m.unlock();
    }
  }
  file.close();
}

}