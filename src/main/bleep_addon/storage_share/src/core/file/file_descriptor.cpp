//
// Created by csrc on 20. 12. 24..
//

#include "file_descriptor.h"

#include "../util/utils.h"

/* file_descriptor */
// assign a file descriptor to file with corresponding flags(modes) and preassigned fd number
// increase reference count of file
file_descriptor::file_descriptor(file* f, int fd, int flags) {
    this->f = f;
    file_info._fileno = fd;
    this->flags = flags;

    // cursor move
    if (flags & _FILEMODE_APPEND) {
        random_access(f->get_size());
    } else {
        random_access(0);
    }
}
int file_descriptor::check_eof() {
	return file_info._offset == f->get_size();
}

void file_descriptor::sequential_access(long int offset) {
    size_t target_offset;
    if (offset > 0) {
        target_offset = file_info._offset + offset;
    } else {
        target_offset = file_info._offset - offset;
    }

    if (target_offset < 0) {
        target_offset = 0;
    } else if (target_offset >= f->get_size()) {
        cursor = f->get_end();
        local_offset = target_offset - f->get_size();
        file_info._offset = target_offset;
        return; // iterator-end case
    }
    // there's no iterator-end case, because it is already cut by previous condition
    size_t current_offset = file_info._offset;
    while (true) {
        if (current_offset <= target_offset) {
            if (target_offset - current_offset < cursor->second->get_size() - local_offset) {
                local_offset += target_offset - current_offset;
                file_info._offset = target_offset;
                return;
            } else {
                cursor++;
                current_offset = cursor->first;
                local_offset = 0;
            }
        } else {
            cursor--;
            current_offset = cursor->first;
            local_offset = 0;
        }
    }
}
// move cursor to offset.
// current segment and local offset, and offset value is changed.
// if offset is larger than file size, move to end of the file.
void file_descriptor::random_access(size_t offset) {
    cursor = f->get_segment(offset);
    if (cursor == f->get_end()) {
        local_offset = offset - f->get_size();
    } else {
        local_offset = offset - cursor->first;
    }
    file_info._offset = offset;
}

size_t file_descriptor::read(void * ptr, size_t size, size_t count) {
	// TODO: assert for size*count overflow should be added
	size_t read_left = size * count;

	size_t readCnt = 0;

	while (true) {
		if (cursor == f->get_end() || read_left < 0) {
			return (readCnt / size);	// Q: if some last byte is not divided into size, is it 1, or 0?
		}
		size_t segment_read_left = read_left;
		if (segment_read_left > cursor->second->get_size() - local_offset) {
            segment_read_left = cursor->second->get_size() - local_offset;
		}
		size_t readByte = cursor->second->read(&((unsigned char*)ptr)[readCnt], segment_read_left, local_offset);
		readCnt += readByte;
		read_left -= readByte;
		// read stopped for some reason
		bool fStop = readByte < (cursor->second->get_size() - local_offset);
		sequential_access(readByte);
		if (fStop) {
			return (readCnt / size);	// Q: if some last byte is not divided into size, is it 1, or 0?
		}
	}
}
size_t file_descriptor::write(const void * ptr, size_t size, size_t count) {
	// TODO: assert for size*count overflow should be added
	size_t write_amount = size * count;

	if (flags & _FILEMODE_APPEND) {
	    random_access(f->get_size());
	}

	// IMPORTANT: when the mode includes append, it directly goes to end of the file and write on there.
	//            all offset is also changed, so that when ftell is called, it returns end offset.
	// default action is just overwrite when there's data in there.
    size_t writtenBytes;
	if (cursor == f->get_end()) {
	    if (f->get_size() == 0) {
	        // if there's no file segment, create.
            // new segment should be added
            cursor = f->create_new_segment(local_offset);
            writtenBytes = cursor->second->write(ptr, write_amount, local_offset);
	    } else {
	        // fix cursor to last segment with local offset + the segment's size
	        cursor--;
	        local_offset = local_offset + cursor->second->get_size();

            if (cursor->second->get_flags() & _DATASEGMENT_SHARED) {
                cursor->second = new memory_unit(dynamic_cast<sharing_unit*>(cursor->second));
            }
            writtenBytes = cursor->second->write(ptr, write_amount, local_offset);
	    }
	} else {
        if (cursor->second->get_flags() & _DATASEGMENT_SHARED) {
            cursor->second = new memory_unit(dynamic_cast<sharing_unit*>(cursor->second));
        }
        size_t segmentLeftSpace = cursor->second->get_size() - local_offset;
        writtenBytes = cursor->second->write(ptr, write_amount, local_offset);
        size_t erasedByteLeft = writtenBytes - segmentLeftSpace;

        f->fix_covered_segment(cursor, erasedByteLeft);
	}
    // do split
    // TEMPORARY
    // CAUTION: remains safe when the file control system is accessed by only one thread at the same time.
    f->divide_local_segment(cursor);

	f->update_size();
	random_access(file_info._offset + writtenBytes);

	return (writtenBytes / size);	// Q: if some last byte is not divided into size, is it 1, or 0?
}
int file_descriptor::padding(size_t offset, size_t size) {
    // since there's no interface for users to get size of the file, just return 0.
    // if some application uses append mode, this should be implemented.
    return 0;
}

/* file_descriptors */
file_descriptor* file_descriptors::assign_file_descriptor(file *f, int flags) {
    int fd = try_assign_unused_fd();
    if (fd == -1) {
        // error case
        return nullptr;
    }
    if (fd >= fdlist.size()) {
        fdlist.resize(fd + 10, nullptr);
    }
    fdlist[fd] = new file_descriptor(f, fd, flags);
    return fdlist[fd];
}
int file_descriptors::release_file_descriptor(int fd) {
    if (fd >= fdlist.size() || fdlist[fd] == nullptr) {
        return -1;
    }
    delete fdlist[fd];
    fdlist[fd] = nullptr;
    return free_fd(fd);
}
file_descriptor * file_descriptors::get_file_descriptor(int fd) {
    if (fd >= fdlist.size()) {
        return nullptr;
    }
    return fdlist[fd];
}