#pragma once

#include <dwrite.h>
#include <cstring>
#include <cstdint>
#include <new>

#ifndef PEACH_MAX_CUSTOM_FONTS
#   define PEACH_MAX_CUSTOM_FONTS 16
#endif

namespace peach {
    class context_t;

    struct font_blob_t {
        const void* data = nullptr;
        size_t      size = 0;
        bool        used = false;
    };

    class mem_font_file_loader_t : public IDWriteFontFileLoader {
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID iid, void** obj ) override {
            if ( iid == __uuidof( IUnknown ) || iid == __uuidof( IDWriteFontFileLoader ) ) {
                *obj = this; AddRef( ); return S_OK;
            }
            *obj = nullptr; return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef( )  override { return ++m_ref; }
        ULONG STDMETHODCALLTYPE Release( ) override {
            auto r = --m_ref; if ( !r ) delete this; return r;
        }

        HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
            const void* key, UINT32 key_size,
            IDWriteFontFileStream** stream ) override;

        static mem_font_file_loader_t* create( ) { return new mem_font_file_loader_t( ); }

    private:
        mem_font_file_loader_t( ) = default;
        LONG m_ref = 1;
    };

    class mem_font_stream_t : public IDWriteFontFileStream {
    public:
        mem_font_stream_t( const void* data, size_t size )
            : m_data( data ), m_size( size ) {
        }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID iid, void** obj ) override {
            if ( iid == __uuidof( IUnknown ) || iid == __uuidof( IDWriteFontFileStream ) ) {
                *obj = this; AddRef( ); return S_OK;
            }
            *obj = nullptr; return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef( )  override { return ++m_ref; }
        ULONG STDMETHODCALLTYPE Release( ) override {
            auto r = --m_ref; if ( !r ) delete this; return r;
        }

        HRESULT STDMETHODCALLTYPE ReadFileFragment(
            const void** fragment_start, UINT64 offset, UINT64 size,
            void** fragment_ctx ) override {
            if ( offset + size > m_size ) return E_INVALIDARG;
            *fragment_start = static_cast< const uint8_t* >( m_data ) + offset;
            *fragment_ctx = nullptr;
            return S_OK;
        }
        void STDMETHODCALLTYPE ReleaseFileFragment( void* ) override { }

        HRESULT STDMETHODCALLTYPE GetFileSize( UINT64* size ) override {
            *size = m_size; return S_OK;
        }
        HRESULT STDMETHODCALLTYPE GetLastWriteTime( UINT64* t ) override {
            *t = 0; return S_OK;
        }

    private:
        const void* m_data;
        size_t      m_size;
        LONG        m_ref = 1;
    };

    class mem_font_enumerator_t : public IDWriteFontFileEnumerator {
    public:
        mem_font_enumerator_t( IDWriteFactory* factory,
            mem_font_file_loader_t* loader,
            const uint32_t* indices,
            int                      count )
            : m_factory( factory ), m_loader( loader ),
            m_count( count ), m_current( -1 ) {
            for ( int i = 0; i < count && i < PEACH_MAX_CUSTOM_FONTS; ++i )
                m_indices[ i ] = indices[ i ];
            m_factory->AddRef( );
            m_loader->AddRef( );
        }
        ~mem_font_enumerator_t( ) {
            if ( m_current_file ) { m_current_file->Release( ); m_current_file = nullptr; }
            m_factory->Release( );
            m_loader->Release( );
        }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID iid, void** obj ) override {
            if ( iid == __uuidof( IUnknown ) || iid == __uuidof( IDWriteFontFileEnumerator ) ) {
                *obj = this; AddRef( ); return S_OK;
            }
            *obj = nullptr; return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef( )  override { return ++m_ref; }
        ULONG STDMETHODCALLTYPE Release( ) override {
            auto r = --m_ref; if ( !r ) delete this; return r;
        }

        HRESULT STDMETHODCALLTYPE MoveNext( BOOL* has_current ) override {
            if ( m_current_file ) { m_current_file->Release( ); m_current_file = nullptr; }
            ++m_current;
            if ( m_current >= m_count ) { *has_current = FALSE; return S_OK; }

            uint32_t idx = m_indices[ m_current ];
            HRESULT hr = m_factory->CreateCustomFontFileReference(
                &idx, sizeof( idx ), m_loader, &m_current_file );
            *has_current = SUCCEEDED( hr ) ? TRUE : FALSE;
            return SUCCEEDED( hr ) ? S_OK : hr;
        }

        HRESULT STDMETHODCALLTYPE GetCurrentFontFile( IDWriteFontFile** file ) override {
            if ( !m_current_file ) return E_FAIL;
            m_current_file->AddRef( );
            *file = m_current_file;
            return S_OK;
        }

    private:
        IDWriteFactory* m_factory;
        mem_font_file_loader_t* m_loader;
        uint32_t                m_indices[ PEACH_MAX_CUSTOM_FONTS ];
        int                     m_count;
        int                     m_current;
        IDWriteFontFile* m_current_file = nullptr;
        LONG                    m_ref = 1;
    };

    class mem_font_collection_loader_t : public IDWriteFontCollectionLoader {
    public:
        mem_font_collection_loader_t( mem_font_file_loader_t* file_loader,
            const uint32_t* indices,
            int                     count )
            : m_file_loader( file_loader ), m_count( count ) {
            for ( int i = 0; i < count && i < PEACH_MAX_CUSTOM_FONTS; ++i )
                m_indices[ i ] = indices[ i ];
            m_file_loader->AddRef( );
        }
        ~mem_font_collection_loader_t( ) { m_file_loader->Release( ); }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID iid, void** obj ) override {
            if ( iid == __uuidof( IUnknown ) || iid == __uuidof( IDWriteFontCollectionLoader ) ) {
                *obj = this; AddRef( ); return S_OK;
            }
            *obj = nullptr; return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef( )  override { return ++m_ref; }
        ULONG STDMETHODCALLTYPE Release( ) override {
            auto r = --m_ref; if ( !r ) delete this; return r;
        }

        HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
            IDWriteFactory* factory,
            const void*                  /* key */,
            UINT32                       /* key_size */,
            IDWriteFontFileEnumerator** enumerator ) override {
            auto* e = new ( std::nothrow ) mem_font_enumerator_t(
                factory, m_file_loader, m_indices, m_count );
            if ( !e ) return E_OUTOFMEMORY;
            *enumerator = e;
            return S_OK;
        }

    private:
        mem_font_file_loader_t* m_file_loader;
        uint32_t                m_indices[ PEACH_MAX_CUSTOM_FONTS ];
        int                     m_count;
        LONG                    m_ref = 1;
    };

    class font_registry;
    static font_registry* s_registry_instance = nullptr;

    class font_registry {
    public:
        // Singleton
        static font_registry& get( ) {
            static font_registry inst;
            return inst;
        }

        // ── Register a font blob ─────────────────────────────────────────────────
        // data must remain valid for the entire app lifetime (no copy is made).
        // Returns the index assigned to this blob, or -1 on failure.
        int add( const void* data, size_t size ) {
            if ( m_blob_count >= PEACH_MAX_CUSTOM_FONTS ) return -1;
            int idx = m_blob_count++;
            m_blobs[ idx ] = { data, size, true };
            return idx;
        }

        // Convenience: register from a typed array
        template<size_t N>
        int add( const uint8_t( &arr )[ N ] ) { return add( arr, N ); }

        // ── Install into a raw IDWriteFactory  ───────────────────────────────────
        // Call once after DWriteCreateFactory and before any draw_text calls.
        // Keeps ownership of all COM objects internally; call uninstall() or
        // destroy the registry to clean up.
        bool install( IDWriteFactory* factory ) {
            if ( m_installed || m_blob_count == 0 ) return m_installed;
            m_factory = factory;
            m_factory->AddRef( );

            // 1. File loader
            m_file_loader = mem_font_file_loader_t::create( );
            if ( FAILED( factory->RegisterFontFileLoader( m_file_loader ) ) ) {
                m_file_loader->Release( ); m_file_loader = nullptr;
                m_factory->Release( ); m_factory = nullptr;
                return false;
            }

            // Build index list
            uint32_t indices[ PEACH_MAX_CUSTOM_FONTS ];
            for ( int i = 0; i < m_blob_count; ++i ) indices[ i ] = ( uint32_t )i;

            // 2. Collection loader
            m_coll_loader = new ( std::nothrow ) mem_font_collection_loader_t(
                m_file_loader, indices, m_blob_count );
            if ( !m_coll_loader ) { cleanup( ); return false; }

            if ( FAILED( factory->RegisterFontCollectionLoader( m_coll_loader ) ) ) {
                cleanup( ); return false;
            }

            // 3. Build the collection (key = pointer to loader itself, unique per instance)
            const void* key = m_coll_loader;
            HRESULT hr = factory->CreateCustomFontCollection(
                m_coll_loader,
                &key, sizeof( key ),
                &m_collection );
            if ( FAILED( hr ) ) { cleanup( ); return false; }

            // 4. Register the raw GDI handle as well so DWrite's fallback path
            //    also finds the font.  This is belt-and-suspenders.
            for ( int i = 0; i < m_blob_count; ++i ) {
                if ( !m_blobs[ i ].used ) continue;
                DWORD num = 0;
                AddFontMemResourceEx(
                    const_cast< void* >( m_blobs[ i ].data ),
                    static_cast< DWORD >( m_blobs[ i ].size ),
                    nullptr, &num );
                // We don't store the HANDLE — if you need removal, store it yourself.
            }

            m_installed = true;
            s_registry_instance = this;
            return true;
        }

        // ── Install via context_t (recommended) ──────────────────────────────────
        // Extracts the IDWriteFactory* from a live peach context without needing
        // to keep your own reference.
        bool install_into_context( context_t& ctx );
        // ── Get the built collection ─────────────────────────────────────────────
        // Returns nullptr before install() succeeds.
        IDWriteFontCollection* collection( ) const { return m_collection; }

        bool has_family( const wchar_t* name ) const {
            if ( !m_collection ) return false;
            UINT32 idx; BOOL exists = FALSE;
            m_collection->FindFamilyName( name, &idx, &exists );
            return exists == TRUE;
        }

        IDWriteFontFace* find_face( const wchar_t* family_name,
            DWRITE_FONT_WEIGHT  weight = DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL,
            DWRITE_FONT_STYLE   style = DWRITE_FONT_STYLE_NORMAL ) const {
            if ( !m_collection ) return nullptr;
            UINT32 idx; BOOL exists = FALSE;
            if ( FAILED( m_collection->FindFamilyName( family_name, &idx, &exists ) ) || !exists )
                return nullptr;

            IDWriteFontFamily* family = nullptr;
            if ( FAILED( m_collection->GetFontFamily( idx, &family ) ) ) return nullptr;

            IDWriteFont* font = nullptr;
            family->GetFirstMatchingFont( weight, stretch, style, &font );
            family->Release( );
            if ( !font ) return nullptr;

            IDWriteFontFace* face = nullptr;
            font->CreateFontFace( &face );
            font->Release( );
            return face;
        }

        void uninstall( ) { cleanup( ); }

        ~font_registry( ) { cleanup( ); }

    public:
        font_registry( ) = default;
        font_registry( const font_registry& ) = delete;
        font_registry& operator=( const font_registry& ) = delete;

        void cleanup( ) {
            if ( m_collection ) { m_collection->Release( ); m_collection = nullptr; }
            if ( m_factory && m_coll_loader ) {
                m_factory->UnregisterFontCollectionLoader( m_coll_loader );
            }
            if ( m_coll_loader ) { m_coll_loader->Release( ); m_coll_loader = nullptr; }
            if ( m_factory && m_file_loader ) {
                m_factory->UnregisterFontFileLoader( m_file_loader );
            }
            if ( m_file_loader ) { m_file_loader->Release( ); m_file_loader = nullptr; }
            if ( m_factory ) { m_factory->Release( ); m_factory = nullptr; }
            m_installed = false;
        }

        font_blob_t                    m_blobs[ PEACH_MAX_CUSTOM_FONTS ]{};
        int                            m_blob_count = 0;
        IDWriteFactory* m_factory = nullptr;
        mem_font_file_loader_t* m_file_loader = nullptr;
        mem_font_collection_loader_t* m_coll_loader = nullptr;
        IDWriteFontCollection* m_collection = nullptr;
        bool                           m_installed = false;
    };

    inline HRESULT STDMETHODCALLTYPE mem_font_file_loader_t::CreateStreamFromKey(
        const void* key, UINT32 key_size,
        IDWriteFontFileStream** stream ) {
        if ( key_size != sizeof( uint32_t ) ) return E_INVALIDARG;
        uint32_t idx = *static_cast< const uint32_t* >( key );
        auto& reg = font_registry::get( );
        if ( idx >= static_cast< uint32_t >( reg.m_blob_count ) || !reg.m_blobs[ idx ].used )
            return E_INVALIDARG;

        auto* s = new ( std::nothrow ) mem_font_stream_t(
            reg.m_blobs[ idx ].data, reg.m_blobs[ idx ].size );
        if ( !s ) return E_OUTOFMEMORY;
        *stream = s;
        return S_OK;
    }

    inline uint32_t first_codepoint( const char* s ) {
        if ( !s ) return 0;
        const auto* p = ( const uint8_t* )s;
        if ( ( *p & 0x80 ) == 0 )   return *p;
        if ( ( *p & 0xE0 ) == 0xC0 ) return ( ( *p & 0x1F ) << 6 ) | ( p[ 1 ] & 0x3F );
        if ( ( *p & 0xF0 ) == 0xE0 ) return ( ( *p & 0x0F ) << 12 ) | ( ( p[ 1 ] & 0x3F ) << 6 ) | ( p[ 2 ] & 0x3F );
        if ( ( *p & 0xF8 ) == 0xF0 ) return ( ( *p & 0x07 ) << 18 ) | ( ( p[ 1 ] & 0x3F ) << 12 ) | ( ( p[ 2 ] & 0x3F ) << 6 ) | ( p[ 3 ] & 0x3F );
        return 0;
    }

    inline bool query_font_family_name( const void* ttf_data, size_t ttf_size,
        wchar_t* out_name, int out_len ) {
        auto& reg = font_registry::get( );
        if ( !reg.m_collection || !reg.m_factory ) return false;

        UINT32 count = reg.m_collection->GetFontFamilyCount( );
        for ( UINT32 i = 0; i < count; ++i ) {
            IDWriteFontFamily* family = nullptr;
            if ( FAILED( reg.m_collection->GetFontFamily( i, &family ) ) ) continue;
            IDWriteLocalizedStrings* names = nullptr;
            family->GetFamilyNames( &names );
            if ( names ) {
                names->GetString( 0, out_name, out_len );
                names->Release( );
                family->Release( );
                return true;
            }
            family->Release( );
        }
        return false;
    }

}