#include <sstream>

#include <assert.h>

#include <apt-pkg/pkgcache.h>
#include <apt-pkg/prettyprinters.h>
#include <apt-pkg/cachefile.h>

struct PCache {
    // Owned by us.
    pkgCacheFile *cache_file;

    // Borrowed from cache_file.
    pkgCache *cache;
};

struct PPkgIterator {
    // Owned by us.
    pkgCache::PkgIterator iterator;

    // Borrowed from PCache.
    pkgCache *cache;
};

extern "C" {
    void init_config_system();

    PCache *pkg_cache_create();

    PPkgIterator *pkg_cache_pkg_iter(PCache *cache);
    PPkgIterator *pkg_cache_find_name(PCache *cache, const char *name);
    PPkgIterator *pkg_cache_find_name_arch(PCache *cache, const char *name, const char *arch);
    void pkg_iter_release(PPkgIterator *iterator);

    void pkg_iter_next(PPkgIterator *iterator);
    bool pkg_iter_end(PPkgIterator *iterator);

    const char *pkg_iter_name(PPkgIterator *iterator);
    const char *pkg_iter_arch(PPkgIterator *iterator);
    const char *pkg_iter_current_version(PPkgIterator *iterator);

    // freed by caller
    char *pkg_iter_pretty(PCache *cache, PPkgIterator *iterator);

}

void init_config_system() {
    pkgInitConfig(*_config);
    pkgInitSystem(*_config, _system);
}

PCache *pkg_cache_create() {
    pkgCacheFile *cache_file = new pkgCacheFile();
    pkgCache *cache = cache_file->GetPkgCache();

    PCache *ret = new PCache();
    ret->cache_file = cache_file;
    ret->cache = cache;

    return ret;
}

void pkg_cache_release(PCache *cache) {
    // TODO: is cache->cache cleaned up with cache->cache_file?
    delete cache->cache_file;
    delete cache;
}

PPkgIterator *pkg_cache_pkg_iter(PCache *cache) {
    PPkgIterator *wrapper = new PPkgIterator();
    wrapper->iterator = cache->cache->PkgBegin();
    wrapper->cache = cache->cache;
    return wrapper;
}

PPkgIterator *pkg_cache_find_name(PCache *cache, const char *name) {
    PPkgIterator *wrapper = new PPkgIterator();
    wrapper->iterator = cache->cache->FindPkg(name);
    wrapper->cache = cache->cache;
    return wrapper;
}

PPkgIterator *pkg_cache_find_name_arch(PCache *cache, const char *name, const char *arch) {
    PPkgIterator *wrapper = new PPkgIterator();
    wrapper->iterator = cache->cache->FindPkg(name, arch);
    wrapper->cache = cache->cache;
    return wrapper;
}

// TODO: we don't expose this so we always leak the wrapper.
void pkg_iter_release(PPkgIterator *wrapper) {
    delete wrapper;
}

void pkg_iter_next(PPkgIterator *wrapper) {
    ++wrapper->iterator;
}

bool pkg_iter_end(PPkgIterator *wrapper) {
    return wrapper->cache->PkgEnd() == wrapper->iterator;
}

const char *pkg_iter_name(PPkgIterator *wrapper) {
    return wrapper->iterator.Name();
}

const char *pkg_iter_arch(PPkgIterator *wrapper) {
    return wrapper->iterator.Arch();
}

const char *pkg_iter_current_version(PPkgIterator *wrapper) {
    return wrapper->iterator.CurVersion();
}

char *pkg_iter_pretty(PCache *cache, PPkgIterator *wrapper) {
    assert(cache);
    assert(wrapper);
    std::stringstream ss;
    ss << APT::PrettyPkg(cache->cache_file->GetDepCache(), wrapper->iterator);
    return strdup(ss.str().c_str());
}

