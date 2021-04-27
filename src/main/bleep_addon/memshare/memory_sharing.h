//
// Created by csrc on 21. 4. 15..
//

#ifndef UNTITLED15_MEMORY_SHARING_H
#define UNTITLED15_MEMORY_SHARING_H

#ifdef __cplusplus

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
    class memory_sharing_unspecified {
    private:
        std::mutex tbl_lock;
    public:
        virtual ~memory_sharing_unspecified() {}
        virtual void try_share(void* sptr_ref);
        virtual void* lookup(void* sptr_ref);
        void lock_tbl();
        void unlock_tbl();
    };

    template <typename SPTR_ELEM>
    class memory_sharing : public memory_sharing_unspecified {
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
        void try_share(void* sptr_ref) {
            SPTR_ELEM* sptr_ptr = (SPTR_ELEM*)sptr_ref;
            lock_tbl();
            tbl.insert(*sptr_ptr);
            unlock_tbl();
        }
        void* lookup(void* sptr_ref) {
            SPTR_ELEM* sptr_ptr = (SPTR_ELEM*)sptr_ref;
            lock_tbl();
            auto it = tbl.find(*sptr_ptr);
            unlock_tbl();
            if (it != tbl.end()) {
                SPTR_ELEM* res = new SPTR_ELEM();
                *res = *it;
                return (void *) res;
            }
            return sptr_ref;
        }
    };
}


extern "C"
{
#endif

void try_register_memshare_table(void* type_idx_ref, void* mtbl);
void memshare_try_share(void* type_idx_ref, void* sptr_ref);
void* memshare_lookup(void* type_idx_ref, void* sptr_ref);

#ifdef __cplusplus
}
#endif


#endif //UNTITLED15_MEMORY_SHARING_H
