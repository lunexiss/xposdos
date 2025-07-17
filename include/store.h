#ifndef STORE_H
#define STORE_H

void save_to_store(const char* key, const char* value);
const char* load_from_store(const char* key);
void load_store_from_disk(void);

#endif
