//
// Created by csrc on 21. 4. 15..
//

#ifndef UNTITLED15_MEMORY_SHARING_H
#define UNTITLED15_MEMORY_SHARING_H

#include <typeinfo>
#include <unordered_set>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <mutex>


/* sharing target object requires:
 * 1. bool operator==(const A& other)
 * 2. std::size_t hash()
 */

namespace memshare {
    class memory_sharing_base {
    public:
        virtual ~memory_sharing_base() {}
    };

    template <typename SPTR_ELEM>
    class memory_sharing : public memory_sharing_base {
    private:
        struct Hash {
            template <typename T, template <typename ELEM> typename SPTR_TYPE>
            std::size_t operator() (SPTR_TYPE<T> const &p) const {
                return p->hash();
            }
        };
        struct Compare {
            template <typename T, typename U, template <typename ELEM> typename SPTR_TYPE>
            size_t operator() (SPTR_TYPE<T> const &a, SPTR_TYPE<U> const &b) const {
                return typeid(*a) == typeid(*b) && *a == *b;
            }
        };
        std::unordered_set<SPTR_ELEM, Hash, Compare> tbl;
    public:
        memory_sharing() {}
        virtual ~memory_sharing() {}
        void try_share(SPTR_ELEM sptr) {
            tbl.insert(sptr);
        }
        SPTR_ELEM lookup(SPTR_ELEM sptr) {
            auto it = tbl.find(sptr);
            if (it != tbl.end())
                return *it;
            return sptr;
        }
    };

    extern std::unordered_map<std::type_index, memory_sharing_base*> global_mtbl_map;
    extern std::mutex global_mtbl_lock;

    template <typename SPTR_TYPE>
    memory_sharing<SPTR_TYPE>* get_memory_sharing() {
        global_mtbl_lock.lock();
        auto it = global_mtbl_map.find(std::type_index(typeid(SPTR_TYPE)));
        if (it != global_mtbl_map.end()) {
            global_mtbl_lock.unlock();
            memory_sharing<SPTR_TYPE>* mtbl = (memory_sharing<SPTR_TYPE>*)(it->second);
            return mtbl;
        } else {
            memory_sharing<SPTR_TYPE>* mtbl = new memory_sharing<SPTR_TYPE>();
            global_mtbl_map.insert({std::type_index(typeid(SPTR_TYPE)), mtbl});
            global_mtbl_lock.unlock();
            return mtbl;
        }
    }

    template <typename SPTR_TYPE>
    void try_share(SPTR_TYPE sptr) {
        auto it = get_memory_sharing<SPTR_TYPE>();
        it->try_share(sptr);
    }
    template <typename SPTR_TYPE>
    SPTR_TYPE lookup(SPTR_TYPE sptr) {
        auto it = get_memory_sharing<SPTR_TYPE>();
        return it->lookup(sptr);
    }

}


#endif //UNTITLED15_MEMORY_SHARING_H
