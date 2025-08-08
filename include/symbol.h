#ifndef SYMBOL_H
#define SYMBOL_H

void symbol_intern_init(void);
const char *symbol_intern(const char *name);
void symbol_intern_free_all(void);

#endif
