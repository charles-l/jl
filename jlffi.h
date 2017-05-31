#ifndef JLFFI_H
#define JLFFI_H

#include <dlfcn.h>
#include <stdio.h>

static void *loaded = NIL;

void *load_clib(char *name) {
    void *i;
    for(i = loaded; i != NIL && car_(i) != name; i = cdr(i));
    if(i == NIL) {
        void *l = dlopen(name, RTLD_LAZY | RTLD_GLOBAL); // TODO: does global affect anything important?
        loaded = cons(cons(name, l), loaded);
        return l;
    } else {
        return cdr(car(i));
    }
}

void *unload_clib(void *l) {
    // TODO: search through loaded and unload it
    dlclose(l);
}

#endif // JLFFI_H
