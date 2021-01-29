#include "file.h"

#include "../util/utils.h"
#include "../data_unit/data_segment.h"

#include <queue>

void file::_debug_print() {
    std::map<long int, data_segment*>::const_iterator it;
    unsigned char* buffer = nullptr;
    int idx = 0;
    for (it = segments.cbegin(); it != segments.cend(); it++) {
        buffer = (unsigned char*)realloc(buffer, sizeof(unsigned char) * (it->second->get_size() + 1));
        buffer[it->second->get_size()] = 0;
        it->second->read(buffer, it->second->get_size(), 0);
        printf("segment %d - size:%ld, content:%s\n", idx, it->second->get_size(), buffer);
        idx++;
    }
    free(buffer);
}

file::file(const char* filename, datatype* dtype) {
    this->filename = std::string(filename);
    size = 0;
    this->dtype = dtype;
}

std::map<long int, data_segment*>::iterator file::get_segment(long int offset) {
    if (offset >= size) {
        return segments.end();
    }
    auto it = segments.upper_bound(offset);
    return --it;
}
std::map<long int, data_segment*>::iterator file::get_end() {
    return segments.end();
}

void file::cleanse() {
    auto current_it = segments.begin();
    while (current_it != segments.end()) {
        auto next_it = std::next(current_it, 1);
        data_segment* ds = current_it->second;
        segments.erase(current_it);
        if (!(ds->get_flags() & _DATASEGMENT_SHARED) || ((sharing_unit*)ds)->unreference() == 0) {
            delete ds;
        }
        current_it = next_it;
    }
    size = 0;
}

std::map<long int, data_segment*>::iterator file::create_new_segment(size_t initial_size) {
	data_segment* segment = new memory_unit(initial_size, dtype);
	auto it = segments.insert({ size, segment });
	return it.first;
}

void file::update_size() {
	if (segments.empty()) {
		size = 0;
		return;
	}
	auto it = (--segments.end());
	size = it->first + it->second->get_size();
}

void file::fix_covered_segment(std::map<long int, data_segment*>::iterator it, size_t bytesErased) {
    // assumption: it->second is non-sharable
    size_t bytesLeft = bytesErased;

    auto current = std::next(it, 1);
    std::map<long int, data_segment*>::iterator next;
    while (current != segments.end() && bytesLeft > 0) {
        next = std::next(current, 1);
        // check byteleft covers the segment
        if (current->second->get_size() > bytesLeft) {
            // copy current's [bytesLeft, size) to it
            unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * current->second->get_size() - bytesLeft);
            current->second->read(buffer, current->second->get_size() - bytesLeft, bytesLeft);
            it->second->write(buffer, current->second->get_size() - bytesLeft, it->second->get_size());
            free(buffer);
            // drop it
            data_segment* ds = current->second;
            segments.erase(current);
            if (!(ds->get_flags() & _DATASEGMENT_SHARED) || ((sharing_unit*)ds)->unreference() == 0) {
                delete ds;
            }
            break;
        } else {
            // drop fully covered segment
            data_segment* ds = current->second;
            segments.erase(current);
            if (!(ds->get_flags() & _DATASEGMENT_SHARED) || ((sharing_unit*)ds)->unreference() == 0) {
                delete ds;
            }
        }
        current = next;
    }
}

// CAUTION: remains safe when the file control system is accessed by only one thread at the same time.
void file::divide_local_segment(std::map<long int, data_segment*>::iterator it) {
    long int offset = it->first;
    auto* m = dynamic_cast<memory_unit*>(it->second);

    unsigned char* left = m->get_data();
    size_t leftsize = m->get_size();
    unsigned char* right = nullptr;
    size_t rightsize = 0;

    data_segment* prev_dataseg = it->second;
    bool first = true;
    while (left) {
        int subtype = dtype->get_pivot(left, &leftsize, &right, &rightsize);
        data_segment* current_dataseg = new memory_unit(left, leftsize, dtype);
        if (subtype) {
            current_dataseg = make_shared(dynamic_cast<memory_unit*>(current_dataseg));
        }
        if (first) {
            it->second = current_dataseg;
            first = false;
        } else {
            segments.insert({offset, current_dataseg});
        }
        offset += leftsize;

        left = right;
        leftsize = rightsize;
        right = nullptr;
        rightsize = 0;
    }
    delete prev_dataseg;
    _debug_print();
}