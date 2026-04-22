
static bool _wcs_contains( const wchar_t* haystack, const wchar_t* needle ) {
    if ( !haystack || reinterpret_cast< std::uintptr_t >( haystack ) < 0x1000 )
        return false;
    if ( !needle || !needle[ 0 ] )
        return true;

    for ( ; *haystack; ++haystack ) {
        if ( *haystack != *needle ) continue;
        const wchar_t* h = haystack;
        const wchar_t* n = needle;
        while ( *h && *n && *h == *n ) { ++h; ++n; }
        if ( !*n ) return true;
    }
    return false;
}