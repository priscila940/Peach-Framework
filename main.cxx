#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include <dwmapi.h>
#include <impl/includes.h>

static std::string to_hex( int glyph ) {
    char buf[ 8 ];
    buf[ 0 ] = ( char )( 0xE0 | ( glyph >> 12 ) );
    buf[ 1 ] = ( char )( 0x80 | ( ( glyph >> 6 ) & 0x3F ) );
    buf[ 2 ] = ( char )( 0x80 | ( glyph & 0x3F ) );
    buf[ 3 ] = 0;
    return std::string( buf );
}
namespace text {
    constexpr const char* font = "icomoon";
}

namespace af {
    constexpr const char* font = "Font Awesome 7 Free";

    static std::string aimbot_str = to_hex( 0xf8cc );
    static std::string visuals_str = to_hex( 0xf06e );
    static std::string exploit_str = to_hex( 0xf1e2 );  // pick a different codepoint
    static std::string misc_str = to_hex( 0xf085 );     // pick a different codepoint

    const char* aimbot = aimbot_str.c_str( );
    const char* visuals = visuals_str.c_str( );
    const char* exploit = exploit_str.c_str( );
    const char* misc = misc_str.c_str( );
}

namespace ico {
    constexpr const char* font = "icomoon";

    const char* aimbot = "\ue9ce";
}

namespace settings {
    namespace aimbot {
        inline bool  enabled = false;
        inline bool  silent_aim = false;
        inline bool  visible_only = true;
        inline bool  wall_penetration = false;
        inline bool  aim_at_downed = false;
        inline bool  aim_at_bots = true;
        inline float fov = 15.f;
        inline float smooth = 6.f;
        inline float speed = 8.f;
        inline float smooth_factor = 0.35f;
        inline bool  bone_head = true;
        inline bool  bone_neck = false;
        inline bool  bone_chest = false;
        inline bool  bone_pelvis = false;
        inline float crosshair_weight = 0.65f;
        inline float distance_weight = 0.35f;
        inline bool  prefer_visible = true;
    }
    namespace close_aim {
        inline bool  enabled = false;
        inline bool  auto_trigger = true;
        inline float max_range = 80.f;
        inline float fov = 5.f;
        inline bool  players_only = true;
        inline bool  ignore_teammates = false;
    }
    namespace triggerbot {
        inline bool  enabled = false;
        inline bool  burst_mode = false;
        inline float shoot_delay = 0.08f;
        inline float burst_interval = 0.12f;
        inline bool  on_crosshair = true;
        inline bool  on_hitbox_only = false;
    }
    namespace visuals {
        inline bool  bbox_enabled = true;
        inline bool  bbox_filled = false;
        inline float bbox_thickness = 1.5f;
        inline bool  skeleton = true;
        inline float skeleton_thickness = 2.0f;
        inline bool  show_name = true;
        inline bool  show_distance = true;
        inline bool  show_weapon = true;
        inline bool  show_platform = true;
        inline bool  show_level = true;
        inline bool  fov_circle = true;
        inline float fov_circle_radius = 70.f;
        inline float fov_circle_thickness = 1.5f;
        inline bool  snaplines = false;
        inline bool  off_screen_arrows = true;
        inline bool  color_by_visibility = true;
        inline bool  color_teammates = true;
        inline bool  health_bar = false;
        inline bool  health_bar_vertical = true;
        inline float max_esp_distance = 500.f;
    }
    namespace chams {
        inline bool  enabled = false;
        inline bool  wireframe = true;
        inline bool  ignore_depth = true;
        inline bool  two_sided = true;
        inline bool  additive = true;
        inline bool  dynamic_area_light = true;
        inline float color_r = 0.0f;
        inline float color_g = 2.0f;
        inline float color_b = 5.0f;
    }
    namespace tracers {
        inline bool  enabled = true;
        inline float max_age = 2.0f;
        inline float line_thickness = 2.0f;
        inline float impact_dot_radius = 4.0f;
        inline bool  deduplicate = true;
        inline float min_spawn_interval = 0.05f;
    }
    namespace trails {
        inline bool  enabled = true;
        inline float duration = 1.5;
        inline float push_interval = 0.025;
        inline float line_thickness = 1.5f;
        inline int   max_trail_length = 128;
    }
    namespace exploits {
        inline bool  rapid_fire = false;
        inline bool  no_recoil = false;
        inline bool  no_spread = false;
        inline bool  instant_reload = false;
        inline bool  super_glide = false;
        inline bool  extended_slide = false;
        inline bool  anti_aim = false;
        inline bool  speed_hack = false;
    }
    namespace misc {
        inline bool  item_esp = false;
        inline float item_max_distance = 200.f;
        inline bool  show_chest = true;
        inline bool  show_ammobox = true;
        inline bool  show_ground_loot = true;
        inline bool  particles_enabled = true;
        inline float particle_spawn_rate = 0.02f;
        inline bool  fancy_particles = true;
        inline bool  watermark = true;
        inline bool  watermark_shadow = true;
        inline bool  panic_key = true;
        inline float menu_x = 200.f;
        inline float menu_y = 130.f;
        inline bool  show_fps = true;
        inline bool  menu_shadow = true;
        inline float menu_opacity = 1.0f;
    }
}

// ── d3d / window globals ──────────────────────────────────────────────────────
static HWND                    g_hwnd = nullptr;
static IDXGISwapChain* g_swapchain = nullptr;
static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static bool                    g_running = true;
static bool                    g_prev_ldown = false;
static int                     g_sw = 0, g_sh = 0;
static ID3D11RenderTargetView* g_render_rtv = nullptr;

struct menu_state_t {
    bool  open = true;
    float x = 200.f, y = 130.f;
    float sidebar_w = 170.f;
    float height = 520.f;
    float content_w = 620.f;
    float right_w = 180.f;
    int   active_nav = 0, active_tab = 0;
    bool  dragging = false;
    float drag_off_x = 0.f, drag_off_y = 0.f;
} g_menu;

// ── search state ──────────────────────────────────────────────────────────────
struct search_state_t {
    char  buf[ 64 ]{};
    int   len = 0;
    bool  focused = false;
    float cursor_blink = 0.f;
} g_search;

struct search_item_t { const char* label; const char* section; int nav; int tab; };
static const search_item_t k_search_items[ ] = {
    { "Enable Aimbot",      "Aimbot",      0, 0 }, { "Silent Aim",          "Aimbot",      0, 0 },
    { "Visible Only",       "Aimbot",      0, 0 }, { "Wall Penetration",    "Aimbot",      0, 0 },
    { "Aim at Downed",      "Aimbot",      0, 0 }, { "Aim at Bots",         "Aimbot",      0, 0 },
    { "FOV",                "Aimbot",      0, 0 }, { "Smooth",              "Aimbot",      0, 0 },
    { "Speed",              "Aimbot",      0, 0 }, { "Prefer Visible",      "Aimbot",      0, 0 },
    { "Enable Close Aim",   "Close Aim",   0, 1 }, { "Auto Trigger",        "Close Aim",   0, 1 },
    { "Max Range",          "Close Aim",   0, 1 }, { "Players Only",        "Close Aim",   0, 1 },
    { "Ignore Teammates",   "Close Aim",   0, 1 },
    { "Enable Triggerbot",  "Triggerbot",  0, 2 }, { "Burst Mode",          "Triggerbot",  0, 2 },
    { "Shoot Delay",        "Triggerbot",  0, 2 }, { "Burst Interval",      "Triggerbot",  0, 2 },
    { "On Crosshair",       "Triggerbot",  0, 2 }, { "On Hitbox Only",      "Triggerbot",  0, 2 },
    { "Bounding Box",       "Player ESP",  1, 0 }, { "Filled Box",          "Player ESP",  1, 0 },
    { "Skeleton",           "Player ESP",  1, 0 }, { "Off-screen Arrows",   "Player ESP",  1, 0 },
    { "Snap Lines",         "Player ESP",  1, 0 }, { "Health Bar",          "Player ESP",  1, 0 },
    { "Show Name",          "Labels",      1, 0 }, { "Show Distance",       "Labels",      1, 0 },
    { "Show Weapon",        "Labels",      1, 0 }, { "Show Platform",       "Labels",      1, 0 },
    { "Show Level",         "Labels",      1, 0 }, { "Enable Chams",        "Chams",       1, 0 },
    { "Wireframe",          "Chams",       1, 0 }, { "Ignore Depth",        "Chams",       1, 0 },
    { "Two Sided",          "Chams",       1, 0 }, { "Additive Blend",      "Chams",       1, 0 },
    { "Enable Tracers",     "Tracers",     1, 1 }, { "Max Age",             "Tracers",     1, 1 },
    { "Impact Dot Radius",  "Tracers",     1, 1 }, { "Enable Trails",       "Trails",      1, 1 },
    { "Trail Duration",     "Trails",      1, 1 }, { "Enable Loot ESP",     "Loot ESP",    1, 1 },
    { "Show Chests",        "Loot ESP",    1, 1 }, { "Show Ammoboxes",      "Loot ESP",    1, 1 },
    { "Ground Loot",        "Loot ESP",    1, 1 }, { "Loot Distance",       "Loot ESP",    1, 1 },
    { "Color by Visibility","Visibility",  1, 2 }, { "Color Teammates",     "Visibility",  1, 2 },
    { "Show FOV Circle",    "FOV Circle",  1, 2 }, { "FOV Radius",          "FOV Circle",  1, 2 },
    { "Max ESP Distance",   "Distance",    1, 2 },
    { "Super Glide",        "Movement",    2, 0 }, { "Extended Slide",      "Movement",    2, 0 },
    { "Anti-Aim",           "Combat",      2, 0 }, { "Speed Hack",          "Combat",      2, 0 },
    { "Rapid Fire",         "Weapon",      2, 1 }, { "No Recoil",           "Weapon",      2, 1 },
    { "No Spread",          "Weapon",      2, 1 }, { "Instant Reload",      "Weapon",      2, 1 },
    { "Panic Key",          "Safety",      2, 2 },
    { "Particles",          "Visual FX",   3, 0 }, { "Fancy Particles",     "Visual FX",   3, 0 },
    { "Watermark",          "Visual FX",   3, 0 }, { "Particle Spawn Rate", "Visual FX",   3, 0 },
    { "Show FPS",           "Interface",   3, 1 }, { "Menu Shadow",         "Interface",   3, 1 },
    { "Watermark Shadow",   "Interface",   3, 1 }, { "Menu Opacity",        "Interface",   3, 1 },
};
static constexpr int k_search_item_count = ( int )( sizeof( k_search_items ) / sizeof( k_search_items[ 0 ] ) );

// ── tab history ───────────────────────────────────────────────────────────────
struct tab_state_t { int nav, tab; };
static tab_state_t s_history[ 32 ]{};
static int s_history_len = 0, s_history_pos = -1;

static void history_push( int nav, int tab ) {
    s_history_len = s_history_pos + 1;
    if ( s_history_pos >= 0 &&
        s_history[ s_history_pos ].nav == nav &&
        s_history[ s_history_pos ].tab == tab ) return;
    if ( s_history_len >= 32 ) {
        for ( int i = 0; i < 31; i++ ) s_history[ i ] = s_history[ i + 1 ];
        s_history_len = 31; s_history_pos = 30;
    }
    s_history[ s_history_len ] = { nav, tab };
    s_history_pos = s_history_len++;
}
static void history_back( ) {
    if ( s_history_pos <= 0 ) return;
    --s_history_pos;
    g_menu.active_nav = s_history[ s_history_pos ].nav;
    g_menu.active_tab = s_history[ s_history_pos ].tab;
}
static void history_forward( ) {
    if ( s_history_pos >= s_history_len - 1 ) return;
    ++s_history_pos;
    g_menu.active_nav = s_history[ s_history_pos ].nav;
    g_menu.active_tab = s_history[ s_history_pos ].tab;
}
static bool history_can_back( ) { return s_history_pos > 0; }
static bool history_can_forward( ) { return s_history_pos < s_history_len - 1; }

// ── per-nav tab definitions ───────────────────────────────────────────────────
static int get_nav_tabs( int nav, const char* ( &out )[ 4 ] ) {
    switch ( nav ) {
        case 0: out[ 0 ] = "Aimbot";   out[ 1 ] = "Close Aim"; out[ 2 ] = "Triggerbot"; return 3;
        case 1: out[ 0 ] = "Player";   out[ 1 ] = "World";      out[ 2 ] = "Global";     return 3;
        case 2: out[ 0 ] = "Player";   out[ 1 ] = "Weapon";     out[ 2 ] = "Global";     return 3;
        case 3: out[ 0 ] = "General";  out[ 1 ] = "Interface";                       return 2;
        default: return 0;
    }
}

// ── ui namespace ──────────────────────────────────────────────────────────────
namespace ui {
    constexpr const char* k_font = "Segoe UI";
    constexpr const char* k_font_bold = "Segoe UI Semibold";
    constexpr float k_font_sz = 13.f;
    constexpr float k_pad = 10.f;
    constexpr float k_item_h = 26.f;
    constexpr float k_inner_pad = 12.f;

    constexpr peach::color_t k_accent{ 75, 87, 219, 255 };
    constexpr peach::color_t k_accent2{ 94, 105, 238, 255 };
    constexpr peach::color_t k_accent3{ 179,136,235,255 };
    constexpr peach::color_t k_accent4{ 179,136,235,255 };
    constexpr peach::color_t k_text{ 232, 232, 232, 255 };
    constexpr peach::color_t k_text_dim{ 136, 136, 136, 180 };
    constexpr peach::color_t k_hot{ 255, 255, 255, 200 };
    constexpr peach::color_t k_border{ 46,  46,  46, 255 };

    struct cursor_t { float x = 0, y = 0; bool ldown = false, lclick = false; } g_cursor;

    bool hovered( float x, float y, float w, float h ) {
        return g_cursor.x >= x && g_cursor.x <= x + w &&
            g_cursor.y >= y && g_cursor.y <= y + h;
    }
}

// ── window / d3d ─────────────────────────────────────────────────────────────
static LRESULT WINAPI wnd_proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {
    switch ( msg ) {
        case WM_DESTROY:       g_running = false; PostQuitMessage( 0 ); return 0;
        case WM_NCHITTEST:     return g_menu.open ? HTCLIENT : HTTRANSPARENT;
        case WM_MOUSEACTIVATE: return MA_NOACTIVATE;
    }
    return DefWindowProcA( hwnd, msg, wp, lp );
}
static bool create_overlay( ) {
    WNDCLASSEXA wc{ sizeof( wc ) };
    wc.lpfnWndProc = wnd_proc; wc.lpszClassName = "peach_overlay";
    wc.hCursor = LoadCursor( nullptr, IDC_ARROW );
    if ( !RegisterClassExA( &wc ) ) return false;
    g_sw = GetSystemMetrics( SM_CXSCREEN ); g_sh = GetSystemMetrics( SM_CYSCREEN );
    g_hwnd = CreateWindowExA( WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
        wc.lpszClassName, "peach", WS_POPUP,
        0, 0, g_sw, g_sh, nullptr, nullptr, nullptr, nullptr );
    if ( !g_hwnd ) return false;
    ShowWindow( g_hwnd, SW_SHOW ); return true;
}
static bool create_d3d( ) {
    const D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
    if ( FAILED( D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, &fl, 1, D3D11_SDK_VERSION,
        &g_device, nullptr, &g_context ) ) ) return false;
    IDXGIDevice* dd = nullptr; IDXGIAdapter* da = nullptr; IDXGIFactory2* df = nullptr;
    g_device->QueryInterface( __uuidof( IDXGIDevice ), ( void** )&dd );
    dd->GetAdapter( &da ); da->GetParent( __uuidof( IDXGIFactory2 ), ( void** )&df );
    DXGI_SWAP_CHAIN_DESC1 scd{};
    scd.Width = g_sw; scd.Height = g_sh; scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferCount = 2; scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SampleDesc.Count = 1; scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scd.Flags = 0;
    IDXGISwapChain1* sc1 = nullptr;
    df->CreateSwapChainForHwnd( g_device, g_hwnd, &scd, nullptr, nullptr, &sc1 );
    sc1->QueryInterface( __uuidof( IDXGISwapChain ), ( void** )&g_swapchain ); sc1->Release( );
    df->MakeWindowAssociation( g_hwnd, DXGI_MWA_NO_ALT_ENTER );
    df->Release( ); da->Release( ); dd->Release( );
    if ( !g_swapchain ) return false;
    ID3D11Texture2D* bb = nullptr;
    g_swapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( void** )&bb );
    g_device->CreateRenderTargetView( bb, nullptr, &g_render_rtv ); bb->Release( );
    return g_render_rtv != nullptr;
}

// ── nav item ──────────────────────────────────────────────────────────────────
static void draw_nav_item( float x, float y, float w,
    const char* label, int index, int& active ) {
    auto& c = *g_interface; auto& cr = ui::g_cursor;
    const float row_h = 30.f;
     float icon_x = x + 18.f;
    const float icon_y = y + row_h * 0.5f - 8.f; // vertically center the glyph
    bool sel = ( index == active ), hot = ui::hovered( x, y, w, row_h );
    if ( sel )      c.draw_rounded_rect( x + 6.f, y + 3.f, w - 12.f, row_h - 6.f, 5.f, { ui::k_accent2.r, ui::k_accent2.g, ui::k_accent2.b, 80 } );
    else if ( hot ) c.draw_rounded_rect( x + 6.f, y + 3.f, w - 12.f, row_h - 6.f, 5.f, { ui::k_accent2.r, ui::k_accent2.g, ui::k_accent2.b, 18 } );

    peach::color_t icon_col = sel ? ui::k_accent2 : ui::k_text_dim;

    static const char* k_icons[ ] = {
        af::aimbot,
        af::visuals,
        af::exploit,
        af::misc,
    };
    if ( index >= 0 && index < 4 ) {
        icon_x += 1;
        c.draw_text( k_icons[ index ], icon_x, icon_y, af::font, 16.f, icon_col,
            peach::color_t( 0, 0, 0, 0 ), 0.f, peach::text_align::center );
    }

    c.draw_text( label, x + 36.f, icon_y, ui::k_font, 12.5f, sel ? ui::k_hot : ui::k_text_dim );

    if ( hot && cr.lclick && index != active ) {
        active = index;
        g_menu.active_tab = 0;
        history_push( index, 0 );
    }
}

static void draw_sub_item( float x, float y, float w,
    const char* label, int index, int& active,
    peach::color_t dot_col = { 179, 136, 235, 255 } ) {
    auto& c = *g_interface; auto& cr = ui::g_cursor;
    const float row_h = 28.f;
    const float indent = 14.f;
    const float item_x = x + indent;
    const float item_w = w - indent;
    bool sel = ( index == active ), hot = ui::hovered( item_x, y, item_w, row_h );
    const float icon_y = y + row_h * 0.5f;

    if ( hot || sel ) c.draw_rect( x + indent - 2.f, y + 3.f, 2.f, row_h - 6.f,
        { dot_col.r, dot_col.g, dot_col.b, sel ? ( uint8_t )200 : ( uint8_t )60 } );

    if ( sel )      c.draw_rounded_rect( item_x, y + 2.f, item_w - 8.f, row_h - 4.f, 4.f, { 179,136,235,30 } );
    else if ( hot ) c.draw_rounded_rect( item_x, y + 2.f, item_w - 8.f, row_h - 4.f, 4.f, { 179,136,235,14 } );

    c.draw_rounded_rect( item_x + 6.f, icon_y - 5.f, 10.f, 10.f, 3.f,
        { dot_col.r, dot_col.g, dot_col.b, sel ? ( uint8_t )255 : ( uint8_t )140 } );

    c.draw_text( label, item_x + 23.f, icon_y - ui::k_font_sz + 4.5f,
        ui::k_font, 12.f, sel ? ui::k_hot : ui::k_text_dim );

    if ( hot && cr.lclick && index != active ) {
        active = index;
        g_menu.active_tab = 0;
        history_push( index, 0 );
    }
}

// ── top bar ───────────────────────────────────────────────────────────────────
static void draw_top_bar( float x, float y, float w ) {
    auto& c = *g_interface; auto& cr = ui::g_cursor;
    const float h = 40.f;
    c.draw_rounded_rect( x, y, w, h, 10.f, { 22,22,22,255 } );
    c.draw_rounded_rect_outline( x, y, w, h, 10.f, 1.f, { 60,60,60,255 } );

    const float av_cx = x + 10.f + 13.f;
    const float av_cy = y + h * 0.5f;
    c.draw_circle( av_cx, av_cy, 13.f, { 36,36,36,255 } );
    c.draw_text( "LY", av_cx - 6.f, av_cy - ui::k_font_sz * 0.5f + 1.f, ui::k_font_bold, 10.f, ui::k_hot );
    const float name_x = av_cx + 13.f + 6.f;
    c.draw_text( "Luo Yie", name_x, av_cy - ui::k_font_sz * 0.5f - 1.5f, ui::k_font, 11.f, ui::k_text_dim );

    const char* tabs[ 4 ] = {};
    const int tab_count = get_nav_tabs( g_menu.active_nav, tabs );

    if ( tab_count > 0 ) {
        float tab_widths[ 4 ]{};
        float total_tabs_w = 0.f;
        for ( int i = 0; i < tab_count; i++ ) {
            tab_widths[ i ] = g_interface->measure_text( tabs[ i ], ui::k_font_bold, 12.f ) + 20.f;
            total_tabs_w += tab_widths[ i ];
        }
        total_tabs_w += 4.f * ( tab_count - 1 );

        float tab_x = x + ( w - total_tabs_w ) * 0.5f;
        for ( int i = 0; i < tab_count; i++ ) {
            float pw = tab_widths[ i ], tw = pw - 20.f;
            bool sel = ( i == g_menu.active_tab ), hot = ui::hovered( tab_x, y + 7.f, pw, 26.f );
            if ( sel )      c.draw_rounded_rect( tab_x, y + 7.f, pw, 26.f, 13.f, { ui::k_accent.r, ui::k_accent.g, ui::k_accent.b, 150 } );
            else if ( hot ) c.draw_rounded_rect( tab_x, y + 7.f, pw, 26.f, 13.f, { ui::k_accent2.r,  ui::k_accent2.g,  ui::k_accent2.b, 30 } );
            c.draw_text( tabs[ i ], tab_x + ( pw - tw ) * 0.5f, y + 20.f - 12.f * 0.5f,
                ui::k_font_bold, 12.f, sel ? ui::k_hot : ( hot ? ui::k_text : ui::k_text_dim ) );
            if ( hot && cr.lclick && i != g_menu.active_tab ) {
                g_menu.active_tab = i;
                history_push( g_menu.active_nav, i );
            }
            tab_x += pw + 4.f;
        }
    }

    SYSTEMTIME st{}; GetLocalTime( &st );
    int hour12 = st.wHour % 12; if ( !hour12 ) hour12 = 12;
    const char* ampm = st.wHour < 12 ? "AM" : "PM";
    char time_buf[ 16 ]; sprintf_s( time_buf, "%d:%02d %s", hour12, st.wMinute, ampm );
    float time_w = g_interface->measure_text( time_buf, ui::k_font_bold, 12.f );
    float center_y = y + h * 0.5f;
    float time_x = x + w - time_w - 18.f;

    bool can_fwd = history_can_forward( );
    float fwd_x = time_x - 30.f;
    bool fwd_hot = can_fwd && ui::hovered( fwd_x, y + 8.f, 18.f, 24.f );
    peach::color_t fwd_col = fwd_hot ? ui::k_hot : can_fwd ? ui::k_text_dim : peach::color_t{ 60,60,60,180 };
    c.draw_line( fwd_x + 4.f, center_y - 6.f, fwd_x + 10.f, center_y, 1.5f, fwd_col );
    c.draw_line( fwd_x + 10.f, center_y, fwd_x + 4.f, center_y + 6.f, 1.5f, fwd_col );
    if ( fwd_hot && cr.lclick ) history_forward( );

    bool can_back = history_can_back( );
    float back_x = fwd_x - 18.f;
    bool back_hot = can_back && ui::hovered( back_x, y + 8.f, 18.f, 24.f );
    peach::color_t back_col = back_hot ? ui::k_hot : can_back ? ui::k_text_dim : peach::color_t{ 60,60,60,180 };
    c.draw_line( back_x + 10.f, center_y - 6.f, back_x + 4.f, center_y, 1.5f, back_col );
    c.draw_line( back_x + 4.f, center_y, back_x + 10.f, center_y + 6.f, 1.5f, back_col );
    if ( back_hot && cr.lclick ) history_back( );

    c.draw_text( time_buf, time_x, center_y - 12.f * 0.5f, ui::k_font_bold, 12.f, ui::k_text );
}

// ── widget primitives ─────────────────────────────────────────────────────────
static float draw_section_label( float x, float y, const char* title ) {
    g_interface->draw_text( title, x, y, ui::k_font, 11.f, ui::k_text_dim );
    return y + 18.f;
}
static void draw_child( float x, float y, float w, float h ) {
    g_interface->draw_rounded_rect( x, y, w, h, 10.f, { 25,25,25,150 } );
    g_interface->draw_rounded_rect_outline( x, y, w, h, 10.f, 1.f, { 48,48,48,150 } );
}

static void draw_check( float x, float& y, float w, const char* lbl, bool& value ) {
    y += ui::k_item_h;

    auto& c = *g_interface; auto& cr = ui::g_cursor;
    const float bs = 16.f;
    const float bx = x + ui::k_inner_pad;
    const float by = y + ( ui::k_item_h - bs ) * 0.5f;
    bool hot = ui::hovered( x, y, w, ui::k_item_h );
    if ( value ) {
        c.draw_rounded_rect( bx, by, bs, bs, 4.f, ui::k_accent );
        c.draw_line( bx + 2.5f, by + 8.f, bx + 5.5f, by + 11.f, 1.7f, { 0, 0, 0, 255 } );
        c.draw_line( bx + 5.5f, by + 11.f, bx + 13.5f, by + 4.5f, 1.7f, { 0, 0, 0, 255 } );
        c.draw_circle( bx + 5.5f, by + 11.f, 1.75f, { 0, 0, 0, 255 } );
    }
    else {
        c.draw_rounded_rect( bx, by, bs, bs, 4.f, { 32,32,32,255 } );
    }
    const float text_center_y = by + bs;
    const float ty = text_center_y - ui::k_font_sz - 4.f;
    peach::color_t tc = hot ? ui::k_hot : value ? ui::k_text : ui::k_text_dim;
    c.draw_text( lbl, bx + bs + 8.f, ty, ui::k_font, ui::k_font_sz, tc );
    if ( hot && cr.lclick ) value = !value;
}

static void draw_slider( float x, float& y, float w,
    const char* lbl, float& value, float mn, float mx ) {
    y += ui::k_item_h;
    auto& c = *g_interface;
    const float tx = x + ui::k_inner_pad;
    const float tw = w - ui::k_inner_pad * 2.f;
    char buf[ 32 ]; sprintf_s( buf, "%.2f", value );
    float vw = g_interface->measure_text( buf, ui::k_font, ui::k_font_sz );

    const float label_y = y + 2.f;
    c.draw_text( lbl, tx, label_y, ui::k_font, ui::k_font_sz, ui::k_text );
    c.draw_text( buf, tx + tw - vw, label_y, ui::k_font, ui::k_font_sz, ui::k_accent2 );

    const float ty2 = label_y + ui::k_font_sz + 8.f;
    const float th = 3.f;
    c.draw_rounded_rect( tx, ty2, tw, th, 1.5f, { 36,36,36,255 } );
    float t = ( value - mn ) / ( mx - mn ), fw = t * tw;
    if ( fw > 0 ) c.draw_rounded_rect( tx, ty2, fw, th, 1.5f, ui::k_accent );
    bool hot = ui::hovered( tx, ty2 - 6.f, tw, th + 12.f );
    c.draw_circle( tx + fw, ty2 + th * 0.5f, hot ? 6.f : 5.f, hot ? ui::k_accent2 : ui::k_accent );
    if ( hot && ui::g_cursor.ldown && !g_menu.dragging ) {
        float nt = ( ui::g_cursor.x - tx ) / tw;
        nt = nt < 0 ? 0 : nt > 1 ? 1 : nt; value = mn + nt * ( mx - mn );
    }

    y += 6.f;
}

static constexpr int   k_max_results = 32;
static constexpr float k_child_top_pad = -18.f;

// ─────────────────────────────────────────────────────────────────────────────
// CONTENT
// ─────────────────────────────────────────────────────────────────────────────
static void draw_content( float cx, float cy, float cw, float ch,
    float rx, float ry, float rw, float rh ) {
    auto& c = *g_interface;
    auto& m = g_menu;
    auto& s = g_search;

    c.draw_rounded_rect( cx, cy, cw, ch, 12.f, { 18,18,18,255 } );
    c.draw_rounded_rect_outline( cx, cy, cw, ch, 12.f, 1.f, { 38,38,38,255 } );
    c.draw_rounded_rect( rx, ry, rw, rh, 12.f, { 18,18,18,255 } );
    c.draw_rounded_rect_outline( rx, ry, rw, rh, 12.f, 1.f, { 38,38,38,255 } );

    const float children_h = ch;

    {
        c.push_clip_rect( cx + 2.f, cy + 2.f, cw - 4.f, children_h - 4.f );

        const float pad = 14.f, gap = 10.f;
        const float col_w = ( cw - pad * 2.f - gap ) * 0.5f;
        const float lx = cx + pad, rx2 = lx + col_w + gap;
        const float kc = ui::k_item_h, ks = ui::k_item_h + 12.f, ko = 14.f;

        switch ( m.active_nav ) {

            case 0: {
                    switch ( m.active_tab ) {
                        case 0: {
                                constexpr float lbl_h = 18.f;
                                constexpr float r_gap = 10.f;
                                const float left_top = cy + 14.f;
                                const float left_lbl_bot = left_top + lbl_h;
                                const float left_child_h = children_h - ( left_lbl_bot - cy ) - 10.f;
                                const float right_top = cy + 14.f;
                                const float total_right_h = left_child_h + lbl_h;
                                const float right_children_pool = total_right_h - 2.f * lbl_h - r_gap;
                                const float bp_h = right_children_pool * 0.45f;
                                const float sc_h = right_children_pool - bp_h;

                                float ly = left_top;
                                ly = draw_section_label( lx, ly, "General" );
                                draw_child( lx, ly, col_w, left_child_h );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Enable Aimbot", settings::aimbot::enabled );
                                draw_check( lx, iy, col_w, "Silent Aim", settings::aimbot::silent_aim );
                                draw_check( lx, iy, col_w, "Visible Only", settings::aimbot::visible_only );
                                draw_check( lx, iy, col_w, "Wall Penetration", settings::aimbot::wall_penetration );
                                draw_check( lx, iy, col_w, "Aim at Downed", settings::aimbot::aim_at_downed );
                                draw_check( lx, iy, col_w, "Aim at Bots", settings::aimbot::aim_at_bots );
                                draw_slider( lx, iy, col_w, "FOV", settings::aimbot::fov, 1.f, 360.f );
                                draw_slider( lx, iy, col_w, "Smooth", settings::aimbot::smooth, 1.f, 20.f );
                                draw_slider( lx, iy, col_w, "Speed", settings::aimbot::speed, 1.f, 20.f );

                                const float right_child_h = children_h - ( right_top - cy ) - 10.f;
                                const float pool = right_child_h - lbl_h * 2.f - r_gap;
                                const float r_bp_h = pool * 0.45f;
                                const float r_sc_h = pool - r_bp_h;

                                float ry2 = right_top;
                                ry2 = draw_section_label( rx2, ry2, "Bone Priority" );
                                draw_child( rx2, ry2, col_w, r_bp_h );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Head", settings::aimbot::bone_head );
                                draw_check( rx2, iy, col_w, "Neck", settings::aimbot::bone_neck );
                                draw_check( rx2, iy, col_w, "Chest", settings::aimbot::bone_chest );
                                draw_check( rx2, iy, col_w, "Pelvis", settings::aimbot::bone_pelvis );

                                ry2 += r_bp_h + r_gap;
                                ry2 = draw_section_label( rx2, ry2, "Scoring" );
                                draw_child( rx2, ry2, col_w, r_sc_h );
                                iy = ry2 + k_child_top_pad;
                                draw_slider( rx2, iy, col_w, "Crosshair Wt", settings::aimbot::crosshair_weight, 0.f, 1.f );
                                draw_slider( rx2, iy, col_w, "Distance Wt", settings::aimbot::distance_weight, 0.f, 1.f );
                                draw_check( rx2, iy, col_w, "Prefer Visible", settings::aimbot::prefer_visible );
                            } break;
                        case 1: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "General" );
                                draw_child( lx, ly, col_w, kc * 2 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Enable Close Aim", settings::close_aim::enabled );
                                draw_check( lx, iy, col_w, "Auto Trigger", settings::close_aim::auto_trigger );
                                ly += ( kc * 2 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Range" );
                                draw_child( lx, ly, col_w, ks * 2 + ko );
                                iy = ly + k_child_top_pad;
                                draw_slider( lx, iy, col_w, "Max Range", settings::close_aim::max_range, 10.f, 500.f );
                                draw_slider( lx, iy, col_w, "FOV", settings::close_aim::fov, 1.f, 30.f );
                                ry2 = draw_section_label( rx2, ry2, "Filter" );
                                draw_child( rx2, ry2, col_w, kc * 2 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Players Only", settings::close_aim::players_only );
                                draw_check( rx2, iy, col_w, "Ignore Teammates", settings::close_aim::ignore_teammates );
                            } break;
                        case 2: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "General" );
                                draw_child( lx, ly, col_w, kc * 2 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Enable Triggerbot", settings::triggerbot::enabled );
                                draw_check( lx, iy, col_w, "Burst Mode", settings::triggerbot::burst_mode );
                                ly += ( kc * 2 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Timing" );
                                draw_child( lx, ly, col_w, ks * 2 + ko );
                                iy = ly + k_child_top_pad;
                                draw_slider( lx, iy, col_w, "Shoot Delay", settings::triggerbot::shoot_delay, 0.f, 1.f );
                                draw_slider( lx, iy, col_w, "Burst Interval", settings::triggerbot::burst_interval, 0.f, 1.f );
                                ry2 = draw_section_label( rx2, ry2, "Condition" );
                                draw_child( rx2, ry2, col_w, kc * 2 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "On Crosshair", settings::triggerbot::on_crosshair );
                                draw_check( rx2, iy, col_w, "On Hitbox Only", settings::triggerbot::on_hitbox_only );
                            } break;
                    }
                } break;

            case 1: {
                    switch ( m.active_tab ) {
                        case 0: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "ESP" );
                                draw_child( lx, ly, col_w, kc * 6 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Bounding Box", settings::visuals::bbox_enabled );
                                draw_check( lx, iy, col_w, "Filled Box", settings::visuals::bbox_filled );
                                draw_check( lx, iy, col_w, "Skeleton", settings::visuals::skeleton );
                                draw_check( lx, iy, col_w, "Off-screen Arrows", settings::visuals::off_screen_arrows );
                                draw_check( lx, iy, col_w, "Snap Lines", settings::visuals::snaplines );
                                draw_check( lx, iy, col_w, "Health Bar", settings::visuals::health_bar );
                                ly += ( kc * 6 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Labels" );
                                draw_child( lx, ly, col_w, kc * 5 + ko );
                                iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Name", settings::visuals::show_name );
                                draw_check( lx, iy, col_w, "Distance", settings::visuals::show_distance );
                                draw_check( lx, iy, col_w, "Weapon", settings::visuals::show_weapon );
                                draw_check( lx, iy, col_w, "Platform", settings::visuals::show_platform );
                                draw_check( lx, iy, col_w, "Level", settings::visuals::show_level );
                                ry2 = draw_section_label( rx2, ry2, "Chams" );
                                draw_child( rx2, ry2, col_w, kc * 5 + ks * 3 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Enable Chams", settings::chams::enabled );
                                draw_check( rx2, iy, col_w, "Wireframe", settings::chams::wireframe );
                                draw_check( rx2, iy, col_w, "Ignore Depth", settings::chams::ignore_depth );
                                draw_check( rx2, iy, col_w, "Two Sided", settings::chams::two_sided );
                                draw_check( rx2, iy, col_w, "Additive Blend", settings::chams::additive );
                                draw_slider( rx2, iy, col_w, "R", settings::chams::color_r, 0.f, 10.f );
                                draw_slider( rx2, iy, col_w, "G", settings::chams::color_g, 0.f, 10.f );
                                draw_slider( rx2, iy, col_w, "B", settings::chams::color_b, 0.f, 10.f );
                            } break;
                        case 1: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Tracers" );
                                draw_child( lx, ly, col_w, kc + ks * 3 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Enable Tracers", settings::tracers::enabled );
                                draw_slider( lx, iy, col_w, "Max Age (s)", settings::tracers::max_age, 0.5f, 5.f );
                                draw_slider( lx, iy, col_w, "Line Thickness", settings::tracers::line_thickness, 1.f, 4.f );
                                draw_slider( lx, iy, col_w, "Impact Dot Radius", settings::tracers::impact_dot_radius, 1.f, 10.f );
                                ly += ( kc + ks * 3 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Trails" );
                                draw_child( lx, ly, col_w, kc + ks * 2 + ko );
                                iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Enable Trails", settings::trails::enabled );
                                draw_slider( lx, iy, col_w, "Duration (s)", settings::trails::duration, 0.5f, 5.f );
                                draw_slider( lx, iy, col_w, "Line Thickness", settings::trails::line_thickness, 1.f, 4.f );
                                ry2 = draw_section_label( rx2, ry2, "Loot ESP" );
                                draw_child( rx2, ry2, col_w, kc * 4 + ks + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Enable Loot ESP", settings::misc::item_esp );
                                draw_check( rx2, iy, col_w, "Show Chests", settings::misc::show_chest );
                                draw_check( rx2, iy, col_w, "Show Ammoboxes", settings::misc::show_ammobox );
                                draw_check( rx2, iy, col_w, "Ground Loot", settings::misc::show_ground_loot );
                                draw_slider( rx2, iy, col_w, "Max Distance", settings::misc::item_max_distance, 50.f, 500.f );
                            } break;
                        case 2: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Visibility" );
                                draw_child( lx, ly, col_w, kc * 2 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Color by Visibility", settings::visuals::color_by_visibility );
                                draw_check( lx, iy, col_w, "Color Teammates", settings::visuals::color_teammates );
                                ly += ( kc * 2 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "FOV Circle" );
                                draw_child( lx, ly, col_w, kc + ks + ko );
                                iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Show FOV Circle", settings::visuals::fov_circle );
                                draw_slider( lx, iy, col_w, "FOV Radius", settings::visuals::fov_circle_radius, 10.f, 300.f );
                                ry2 = draw_section_label( rx2, ry2, "Distance" );
                                draw_child( rx2, ry2, col_w, ks + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_slider( rx2, iy, col_w, "Max ESP Distance", settings::visuals::max_esp_distance, 50.f, 2000.f );
                            } break;
                    }
                } break;

            case 2: {
                    switch ( m.active_tab ) {
                        case 0: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Movement" );
                                draw_child( lx, ly, col_w, kc * 2 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Super Glide", settings::exploits::super_glide );
                                draw_check( lx, iy, col_w, "Extended Slide", settings::exploits::extended_slide );
                                ry2 = draw_section_label( rx2, ry2, "Combat" );
                                draw_child( rx2, ry2, col_w, kc * 2 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Anti-Aim", settings::exploits::anti_aim );
                                draw_check( rx2, iy, col_w, "Speed Hack", settings::exploits::speed_hack );
                            } break;
                        case 1: {
                                float ly = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Modifications" );
                                draw_child( lx, ly, col_w, kc * 4 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Rapid Fire", settings::exploits::rapid_fire );
                                draw_check( lx, iy, col_w, "No Recoil", settings::exploits::no_recoil );
                                draw_check( lx, iy, col_w, "No Spread", settings::exploits::no_spread );
                                draw_check( lx, iy, col_w, "Instant Reload", settings::exploits::instant_reload );
                            } break;
                        case 2: {
                                float ly = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Safety" );
                                draw_child( lx, ly, col_w, kc * 2 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Panic Key", settings::misc::panic_key );
                                draw_check( lx, iy, col_w, "Anti-Aim", settings::exploits::anti_aim );
                            } break;
                    }
                } break;

            case 3: {
                    switch ( m.active_tab ) {
                        case 0: {
                                float ly = cy + 14.f, ry2 = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Visual FX" );
                                draw_child( lx, ly, col_w, kc * 3 + ks + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Particles", settings::misc::particles_enabled );
                                draw_check( lx, iy, col_w, "Fancy Particles", settings::misc::fancy_particles );
                                draw_check( lx, iy, col_w, "Watermark", settings::misc::watermark );
                                draw_slider( lx, iy, col_w, "Spawn Rate", settings::misc::particle_spawn_rate, 0.01f, 0.2f );
                                ly += ( kc * 3 + ks + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Input" );
                                draw_child( lx, ly, col_w, kc + ko );
                                iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Panic Key", settings::misc::panic_key );
                                ry2 = draw_section_label( rx2, ry2, "Tracers" );
                                draw_child( rx2, ry2, col_w, kc + ks * 2 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Enable Tracers", settings::tracers::enabled );
                                draw_slider( rx2, iy, col_w, "Max Age (s)", settings::tracers::max_age, 0.5f, 5.f );
                                draw_slider( rx2, iy, col_w, "Line Thickness", settings::tracers::line_thickness, 1.f, 4.f );
                                ry2 += ( kc + ks * 2 + ko ) + 14.f;
                                ry2 = draw_section_label( rx2, ry2, "Trails" );
                                draw_child( rx2, ry2, col_w, kc + ks * 2 + ko );
                                iy = ry2 + k_child_top_pad;
                                draw_check( rx2, iy, col_w, "Enable Trails", settings::trails::enabled );
                                draw_slider( rx2, iy, col_w, "Duration (s)", settings::trails::duration, 0.5f, 5.f );
                                draw_slider( rx2, iy, col_w, "Line Thickness", settings::trails::line_thickness, 1.f, 4.f );
                            } break;
                        case 1: {
                                float ly = cy + 14.f;
                                ly = draw_section_label( lx, ly, "Display" );
                                draw_child( lx, ly, col_w, kc * 3 + ko );
                                float iy = ly + k_child_top_pad;
                                draw_check( lx, iy, col_w, "Show FPS", settings::misc::show_fps );
                                draw_check( lx, iy, col_w, "Menu Shadow", settings::misc::menu_shadow );
                                draw_check( lx, iy, col_w, "Watermark Shadow", settings::misc::watermark_shadow );
                                ly += ( kc * 3 + ko ) + 14.f;
                                ly = draw_section_label( lx, ly, "Opacity" );
                                draw_child( lx, ly, col_w, ks + ko );
                                iy = ly + k_child_top_pad;
                                draw_slider( lx, iy, col_w, "Menu Opacity", settings::misc::menu_opacity, 0.1f, 1.f );
                            } break;
                    }
                } break;

            case 5: case 6: case 7: break;
        }

        c.pop_clip_rect( );
    }

    // ── right panel — search ──────────────────────────────────────────────────
right_panel:
    c.push_clip_rect( rx + 2.f, ry + 2.f, rw - 4.f, rh - 4.f );
    {
        constexpr float bar_h = 34.f;
        constexpr float bar_r = 8.f;
        constexpr float pad = 10.f;

        const float bx = rx + pad;
        const float by = ry + pad;
        const float bw = rw - pad * 2.f;

        peach::color_t bar_bg = s.focused ? peach::color_t{ 24,22,30,255 } : peach::color_t{ 22,22,22,255 };
        c.draw_rounded_rect( bx, by, bw, bar_h, bar_r, bar_bg );
        if ( s.focused )
            c.draw_rounded_rect_outline( bx, by, bw, bar_h, bar_r, 1.f,
                { ui::k_accent2.r, ui::k_accent2.g, ui::k_accent2.b, 140 } );
        else
            c.draw_rounded_rect_outline( bx, by, bw, bar_h, bar_r, 1.f,
                { 44,44,44,255 } );

        const float ic_x = bx + 13.f, ic_y = by + bar_h * 0.5f;
        const float ic_r = 5.f;
        peach::color_t ic_col = s.focused ? ui::k_accent2 : ui::k_text_dim;
        c.draw_circle( ic_x, ic_y - 1.f, ic_r, { ic_col.r, ic_col.g, ic_col.b, 25 } );
        c.draw_circle( ic_x, ic_y - 1.f, ic_r * 0.5f, ic_col );
        c.draw_line( ic_x + ic_r * 0.65f, ic_y + ic_r * 0.65f - 1.f,
            ic_x + ic_r * 1.55f, ic_y + ic_r * 1.55f - 1.f, 1.5f, ic_col );

        const float tx = bx + 30.f;
        const float ty = by + ( bar_h - ui::k_font_sz ) * 0.5f;
        if ( s.len == 0 && !s.focused ) {
            c.draw_text( "Search", tx, ty - 2, ui::k_font, ui::k_font_sz, { 52,52,52,255 } );
        }
        else {
            c.draw_text( s.buf, tx, ty - 3, ui::k_font, ui::k_font_sz, ui::k_text );
            s.cursor_blink += 0.025f;
            if ( s.cursor_blink > 1.2f ) s.cursor_blink = 0.f;
            if ( s.cursor_blink < 0.6f ) {
                float cw2 = g_interface->measure_text( s.buf, ui::k_font, ui::k_font_sz );
                c.draw_rect( tx + cw2 + 1.f, ty + 1.f, 1.5f, ui::k_font_sz - 2.f,
                    ui::k_accent2 );
            }
        }

        if ( s.len > 0 ) {
            const float cx2 = bx + bw - 12.f, cy2 = by + bar_h * 0.5f;
            bool x_hot = ui::hovered( cx2 - 8.f, by + 4.f, 16.f, bar_h - 8.f );
            peach::color_t xc = x_hot ? ui::k_hot : ui::k_text_dim;
            c.draw_line( cx2 - 4.f, cy2 - 4.f, cx2 + 4.f, cy2 + 4.f, 1.5f, xc );
            c.draw_line( cx2 + 4.f, cy2 - 4.f, cx2 - 4.f, cy2 + 4.f, 1.5f, xc );
            if ( x_hot && ui::g_cursor.lclick ) { s.len = 0; s.buf[ 0 ] = '\0'; }
        }

        if ( ui::hovered( bx, by, bw, bar_h ) && ui::g_cursor.lclick )
            s.focused = true;
        else if ( ui::g_cursor.lclick && !ui::hovered( rx, ry, rw, rh ) )
            s.focused = false;

        const float res_y = by + bar_h + pad;
        const float res_h = rh - bar_h - pad * 3.f;

        if ( s.len == 0 ) {
            const char* hint = "Type to search";
            float hw = g_interface->measure_text( hint, ui::k_font, 11.f );
            c.draw_text( hint, rx + ( rw - hw ) * 0.5f, ry + rh * 0.5f - 11.f * 0.5f,
                ui::k_font, 11.f, { 44,44,44,255 } );
        }
        else {
            int matches[ k_max_results ];
            int match_count = 0;
            char q[ 64 ]{};
            for ( int i = 0; i < s.len && i < 63; i++ )
                q[ i ] = ( char )tolower( ( unsigned char )s.buf[ i ] );
            for ( int i = 0; i < k_search_item_count && match_count < k_max_results; i++ ) {
                char lbl[ 64 ]{};
                for ( int j = 0; k_search_items[ i ].label[ j ] && j < 63; j++ )
                    lbl[ j ] = ( char )tolower( ( unsigned char )k_search_items[ i ].label[ j ] );
                if ( strstr( lbl, q ) ) matches[ match_count++ ] = i;
            }

            if ( match_count == 0 ) {
                const char* msg = "No results";
                float mw = g_interface->measure_text( msg, ui::k_font, 11.f );
                c.draw_text( msg, rx + ( rw - mw ) * 0.5f, res_y + res_h * 0.5f - 11.f * 0.5f,
                    ui::k_font, 11.f, { 55,55,55,255 } );
            }
            else {
                char hdr[ 32 ]; sprintf_s( hdr, "%d result%s", match_count, match_count == 1 ? "" : "s" );
                c.draw_text( hdr, bx, res_y, ui::k_font, 10.f, { 70,70,70,255 } );

                constexpr float row_h = 34.f;
                float row_y = res_y + 16.f;

                for ( int i = 0; i < match_count; i++ ) {
                    if ( row_y + row_h > ry + rh - pad ) break;
                    const auto& item = k_search_items[ matches[ i ] ];
                    const float rw2 = rw - pad * 2.f;
                    bool row_hot = ui::hovered( bx, row_y, rw2, row_h );

                    if ( row_hot )
                        c.draw_rounded_rect( bx, row_y, rw2, row_h, 5.f,
                            { ui::k_accent2.r, ui::k_accent2.g, ui::k_accent2.b, 18 } );
                    else if ( i % 2 == 0 )
                        c.draw_rounded_rect( bx, row_y, rw2, row_h, 5.f, { 255,255,255,3 } );

                    c.draw_circle( bx + 8.f, row_y + row_h * 0.5f, 3.f,
                        row_hot ? ui::k_accent2 : peach::color_t{ ui::k_accent2.r, ui::k_accent2.g, ui::k_accent2.b, 120 } );
                    c.draw_text( item.label, bx + 18.f, row_y + ( row_h - ui::k_font_sz ) * 0.5f - 2,
                        ui::k_font, ui::k_font_sz, row_hot ? ui::k_hot : ui::k_text );

                    if ( i < match_count - 1 )
                        c.draw_rect( bx + 10.f, row_y + row_h - 1.f, rw2 - 12.f, 1.f, { 34,34,34,180 } );

                    if ( row_hot && ui::g_cursor.lclick ) {
                        g_menu.active_nav = item.nav;
                        g_menu.active_tab = item.tab;
                        history_push( item.nav, item.tab );
                        s.len = 0; s.buf[ 0 ] = '\0'; s.focused = false;
                    }

                    row_y += row_h;
                }
            }
        }
    }
    c.pop_clip_rect( );
}

// ── render menu ───────────────────────────────────────────────────────────────
static void render_menu( ) {
    if ( !g_menu.open ) return;

    struct ACCENT_POLICY { int State; int Flags; int Color; int AnimationId; };
    struct WINCOMPATTRDATA { int Attr; PVOID Data; ULONG Size; };
    typedef BOOL( WINAPI* pSetWindowCompositionAttribute )( HWND, WINCOMPATTRDATA* );

    ACCENT_POLICY accent = { 4, 2, 0x00FFFFFF, 0 }; // ACCENT_ENABLE_ACRYLICBLURBEHIND
    WINCOMPATTRDATA data = { 19, &accent, sizeof( accent ) };
    auto fn = ( pSetWindowCompositionAttribute )GetProcAddress(
        GetModuleHandleA( "user32.dll" ), "SetWindowCompositionAttribute" );
    if ( fn ) fn( g_hwnd, &data );
    static bool s_seeded = false;
    if ( !s_seeded ) { history_push( g_menu.active_nav, g_menu.active_tab ); s_seeded = true; }

    auto& m = g_menu; auto& c = *g_interface; auto& cr = ui::g_cursor;

    c.set_text_antialiasing( true );
    c.set_shape_antialiasing( true );
    c.set_text_pixel_snapping( true );

    const float gap = 8.f, bar_h = 40.f, right_w = 188.f, content_w = 580.f;
    const float total_w = m.sidebar_w + gap + content_w + gap + right_w;
    const float total_h = m.height;
    const float sx = m.x, sy = m.y, sh = m.height, sw = m.sidebar_w;

    if ( ui::hovered( sx, sy, total_w, bar_h ) && cr.lclick ) {
        m.dragging = true; m.drag_off_x = cr.x - sx; m.drag_off_y = cr.y - sy;
    }
    if ( !cr.ldown ) m.dragging = false;
    if ( m.dragging ) { m.x = cr.x - m.drag_off_x; m.y = cr.y - m.drag_off_y; }

    const float bar_x = sx + sw + gap, bar_y = sy, bar_w = content_w + gap + right_w;
    const float content_x = bar_x, content_y = sy + bar_h + gap;
    const float content_h = sh - bar_h - gap;
    const float right_x = content_x + content_w + gap, right_y = content_y, right_h = content_h;

    // sidebar
    c.draw_rounded_rect( sx, sy, sw, sh, 12.f, { 16,16,16,255 } );
    c.draw_rect_gradient_h( sx + 12.f, sy + 1.f, sw - 24.f, 1.f, { 255,255,255,0 }, { 255,255,255,18 } );
    c.draw_rect_gradient_h( sx + 12.f, sy + 1.f, ( sw - 24.f ) * 0.5f, 1.f, { 255,255,255,18 }, { 255,255,255,0 } );
    c.draw_rounded_rect_outline( sx, sy, sw, sh, 12.f, 1.f, { 38,38,38,255 } );
    c.push_clip_rect( sx + 2.f, sy + 2.f, sw - 4.f, sh - 4.f );

    c.draw_text( "SkyCheats", sx + 14.f, sy + 16.f, ui::k_font_bold, 15.f, ui::k_hot );
    c.draw_text( "Get better", sx + 14.f, sy + 32.f, ui::k_font, 11.f, ui::k_text_dim );

    const float btn_x = sx + sw - 28.f, btn_y = sy + 16.f;
    bool btn_hot = ui::hovered( btn_x - 2.f, btn_y - 4.f, 20.f, 16.f );
    peach::color_t dot_c = btn_hot ? ui::k_hot : ui::k_text_dim;
    for ( int i = 0; i < 3; i++ ) c.draw_circle( btn_x + i * 5.f, btn_y + 4.f, 1.5f, dot_c );

    c.draw_rect( sx + 10.f, sy + 52.f, sw - 20.f, 1.f, { 38,38,38,255 } );

    float ny = sy + 58.f;
    draw_nav_item( sx, ny, sw, "Aimbot", 0, m.active_nav ); ny += 30.f;
    draw_nav_item( sx, ny, sw, "Visuals", 1, m.active_nav ); ny += 30.f;
    draw_nav_item( sx, ny, sw, "Exploits", 2, m.active_nav ); ny += 30.f;
    draw_nav_item( sx, ny, sw, "Miscellaneous", 3, m.active_nav ); ny += 30.f;

    ny += 12.f;
    c.draw_text( "Configuration", sx + 14.f, ny, ui::k_font_bold, 12.5f, ui::k_text );
    const float chev_x = sx + sw - 22.f, chev_y = ny + 4.f;
    c.draw_line( chev_x, chev_y, chev_x + 5.f, chev_y + 5.f, 1.5f, ui::k_text_dim );
    c.draw_line( chev_x + 5.f, chev_y + 5.f, chev_x + 10.f, chev_y, 1.5f, ui::k_text_dim );

    ny += 22.f;

    const float cfg_bar_x = sx + 14.f - 2.f;
    const float cfg_bar_top = ny + 4.f;
    const float cfg_bar_bot = ny + 30.f * 3 + 28.f - 4.f;
    c.draw_rect( cfg_bar_x, cfg_bar_top, 2.f, cfg_bar_bot - cfg_bar_top, { 50,50,50,60 } );

    draw_sub_item( sx, ny, sw, "Rage", 5, m.active_nav, { 179,136,235,255 } ); ny += 30.f;
    draw_sub_item( sx, ny, sw, "Legitimate", 6, m.active_nav, { 194,160,239,255 } ); ny += 30.f;
    draw_sub_item( sx, ny, sw, "Tournament", 7, m.active_nav, { 206,179,242,255 } );

    ny += 29.f;
    const float add_indent = 14.f;
    bool add_hot = ui::hovered( sx + add_indent, ny, sw - add_indent, 28.f );
    if ( add_hot ) c.draw_rounded_rect( sx + add_indent, ny + 3.f, sw - add_indent - 8.f, 22.f, 5.f, { 46,46,46,50 } );
    const float plus_x = sx + add_indent + 11.f, plus_y = ny + 14.f;
    c.draw_line( plus_x - 5.f, plus_y, plus_x + 5.f, plus_y, 1.5f, ui::k_text_dim );
    c.draw_line( plus_x, plus_y - 5.f, plus_x, plus_y + 5.f, 1.5f, ui::k_text_dim );
    c.draw_text( "Add Config", plus_x + 12.f, ny + ( 28.f - ui::k_font_sz ) - 10.f,
        ui::k_font, 12.f, add_hot ? ui::k_text : ui::k_text_dim );

    static DWORD s_last_tick = 0; static int s_fc = 0, s_fps = 0;
    DWORD now = GetTickCount( ); s_fc++;
    if ( now - s_last_tick >= 1000 ) { s_fps = s_fc; s_fc = 0; s_last_tick = now; }
    char fps_buf[ 16 ]; sprintf_s( fps_buf, "%d FPS", s_fps );
    c.draw_rounded_rect( sx + 10.f, sy + sh - 28.f, sw - 20.f, 18.f, 4.f, { 26,26,26,255 } );
    peach::color_t fps_col = s_fps >= 60 ? peach::color_t{ 100,220,120,255 } :
        s_fps >= 30 ? peach::color_t{ 220,180,60,255 } :
        peach::color_t{ 220,80,80,255 };
    c.draw_circle( sx + 20.f, sy + sh - 19.f, 3.f, fps_col );
    c.draw_text( fps_buf, sx + 29.f, sy + sh - 28.f + ( 18.f - 11.f ) * 0.5f - 2.5,
        ui::k_font, 11.f, ui::k_text_dim );

    c.pop_clip_rect( );

    draw_top_bar( bar_x, bar_y, bar_w );
    draw_content( content_x, content_y, content_w, content_h, right_x, right_y, right_w, right_h );
}

// ── input + main ─────────────────────────────────────────────────────────────
static void poll_input( ) {
    POINT pt{}; GetCursorPos( &pt );
    ui::g_cursor.x = ( float )pt.x; ui::g_cursor.y = ( float )pt.y;
    const bool cur = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
    ui::g_cursor.lclick = cur && !g_prev_ldown; ui::g_cursor.ldown = cur;
    g_prev_ldown = cur;
    if ( GetAsyncKeyState( VK_INSERT ) & 1 ) g_menu.open = !g_menu.open;

    if ( g_search.focused && g_menu.open ) {
        if ( GetAsyncKeyState( VK_BACK ) & 1 ) {
            if ( g_search.len > 0 ) g_search.buf[ --g_search.len ] = '\0';
        }
        if ( GetAsyncKeyState( VK_ESCAPE ) & 1 ) {
            g_search.len = 0; g_search.buf[ 0 ] = '\0'; g_search.focused = false;
        }
        for ( int vk = 0x20; vk <= 0x5A; vk++ ) {
            if ( !( GetAsyncKeyState( vk ) & 1 ) ) continue;
            if ( g_search.len >= 63 ) break;
            bool shift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
            char ch = 0;
            if ( vk >= 'A' && vk <= 'Z' )  ch = shift ? ( char )vk : ( char )( vk + 32 );
            else if ( vk == VK_SPACE )      ch = ' ';
            else if ( vk == 0xBD )          ch = shift ? '_' : '-';
            if ( ch ) { g_search.buf[ g_search.len++ ] = ch; g_search.buf[ g_search.len ] = '\0'; }
        }
        for ( int vk = '0'; vk <= '9'; vk++ ) {
            if ( GetAsyncKeyState( vk ) & 1 ) {
                if ( g_search.len < 63 ) {
                    g_search.buf[ g_search.len++ ] = ( char )vk;
                    g_search.buf[ g_search.len ] = '\0';
                }
            }
        }
    }
}

int main( ) {
    if ( !create_overlay( ) ) return 1;
    if ( !create_d3d( ) )     return 1;

    static std::vector<unsigned char> s_icomoon_ttf;
    static std::vector<unsigned char> s_af_ttf;
    static std::vector<unsigned char> s_af_solid_ttf;

    auto load_font = [ ] ( const char* data, size_t size, std::vector<unsigned char>& out_ttf ) {
        const int b85_size = ( size - 1 ) * 4 / 5;
        std::vector<unsigned char> lz4_buf( b85_size );
        Decode85( ( const unsigned char* )data, lz4_buf.data( ) );
        unsigned int raw_sz = stb_decompress_length( ( unsigned char* )lz4_buf.data( ) );
        out_ttf.resize( raw_sz );
        stb_decompress( out_ttf.data( ), ( unsigned char* )lz4_buf.data( ), ( unsigned int )lz4_buf.size( ) );
        return peach::font_registry::get( ).add( out_ttf.data( ), out_ttf.size( ) );
        };

    load_font( icomoon_compressed_data_base85, sizeof( icomoon_compressed_data_base85 ), s_icomoon_ttf );
    load_font( af_solid_compressed_data_base85, sizeof( af_solid_compressed_data_base85 ), s_af_solid_ttf );

    if ( !g_interface->init( g_device, g_context, g_swapchain, true ) ) return 1;
    if ( !peach::font_registry::get( ).install_into_context( *g_interface ) ) {
        printf( "Could not load context\n" );
        return 1;
    }

    MSG msg{};
    while ( g_running ) {
        while ( PeekMessageA( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
            TranslateMessage( &msg ); DispatchMessageA( &msg );
        }
        poll_input( );
        float clear[ 4 ] = { 0.f, 0.f, 0.f, 0.f }; 

        g_context->ClearRenderTargetView( g_render_rtv, clear );
        g_context->OMSetRenderTargets( 1, &g_render_rtv, nullptr );
        D3D11_VIEWPORT vp{ 0,0,( float )g_sw,( float )g_sh,0,1 };
        g_context->RSSetViewports( 1, &vp );
        if ( g_menu.open ) {
            render_menu( ); 
        }
        g_interface->flush( );
        g_swapchain->Present( 0, 0 );
    }

    g_render_rtv->Release( ); g_swapchain->Release( );
    g_interface->shutdown( ); g_context->Release( ); g_device->Release( );
    DestroyWindow( g_hwnd ); return 0;
}