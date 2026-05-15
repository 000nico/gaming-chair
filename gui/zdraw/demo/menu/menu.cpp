#include "../global.hpp"
#include "../../../modules/modules.hpp"
#include "../../../config_manager/config_manager.hpp"

namespace menu {

// ── State ─────────────────────────────────────────────────────────────────
static const char* tracer_styles[]   = { "arrows", "lines" };
static const char* tracer_centered[] = { "down", "centered" };
static const char* bone_selection[]  = { "head", "neck", "chest", "pelvis" };

static std::vector<std::string> config_list;
static int         selected_config{ 0 };
static std::string config_name{ "config name" };

static int aimbot_bone{ 0 };

// ── Theme ──────────────────────────────────────────────────────────────────
static void apply_theme( )
{
    auto& s = zui::get_style( );

    zdraw::rgba bg     = { 10,  10,  10,  255 };
    zdraw::rgba bg2    = { 14,  14,  14,  255 };
    zdraw::rgba bg3    = { 20,  20,  20,  255 };
    zdraw::rgba border = { 28,  28,  28,  255 };
    zdraw::rgba text   = { 230, 230, 230, 255 };
    zdraw::rgba pink   = { 255, 182, 213, 255 };

    s.window_bg        = bg;
    s.window_border    = border;
    s.nested_bg        = bg2;
    s.nested_border    = border;
    s.group_box_bg     = bg2;
    s.group_box_border = border;

    s.group_box_title_text = pink;
    s.text   = text;
    s.accent = pink;

    s.checkbox_bg     = bg2;
    s.checkbox_border = border;
    s.checkbox_check  = pink;

    s.button_bg      = bg2;
    s.button_border  = border;
    s.button_hovered = bg3;
    s.button_active  = { 28, 28, 28, 255 };

    s.combo_bg            = bg2;
    s.combo_border        = border;
    s.combo_arrow         = pink;
    s.combo_hovered       = bg3;
    s.combo_popup_bg      = bg;
    s.combo_popup_border  = border;
    s.combo_item_hovered  = bg3;
    s.combo_item_selected = { 45, 45, 45, 255 };

    s.color_picker_bg     = bg2;
    s.color_picker_border = border;

    s.text_input_bg     = bg2;
    s.text_input_border = border;

    s.item_spacing_y   = 7.f;
    s.window_padding_x = 12.f;
    s.window_padding_y = 12.f;
}

// ── Tabs ───────────────────────────────────────────────────────────────────
enum class tab { visuals, combat, config, settings };
static tab current_tab{ tab::visuals };

static constexpr int         k_n        = 4;
static constexpr const char* k_labels[] = { "visuals", "combat", "config", "settings" };
static constexpr tab         k_vals[]   = { tab::visuals, tab::combat, tab::config, tab::settings };

static constexpr float k_header_h = 28.f;
static constexpr float k_tabbar_h = 24.f;
static constexpr float k_top      = k_header_h + k_tabbar_h;

// ── Header + tabs (dibujado encima, no afecta el layout de zui) ────────────
static void draw_header_tabs( float wx, float wy, float ww )
{
    auto& dl = zdraw::get_draw_list( zdraw::draw_layer::window );
    auto* f  = zdraw::get_default_font( );

    zdraw::rgba white    = { 235, 235, 235, 255 };
    zdraw::rgba pink     = { 255, 182, 213, 255 };
    zdraw::rgba dim      = { 120, 120, 120, 255 };

    // ── brand ──
    float x = wx + 10.f;
    float y = wy +  8.f;

    auto [w1, h1] = zdraw::measure_text( "gaming", f );
    dl.add_text( x, y, "gaming", white, f );
    x += w1;

    auto [w2, h2] = zdraw::measure_text( "chair", f );
    dl.add_text( x, y, "chair", pink, f );
    x += w2;

    dl.add_text( x, y, ".cc", white, f );

    // ── tab bar ──
    float tw = ww / k_n;
    float mx = zui::detail::mouse_x( );
    float my = zui::detail::mouse_y( );
    bool  click = zui::detail::mouse_clicked( );

    for ( int i = 0; i < k_n; i++ )
    {
        float tx = wx + i * tw;
        float ty = wy + k_header_h;

        bool active  = ( current_tab == k_vals[ i ] );
        bool hovered = ( mx >= tx && mx <= tx + tw && my >= ty && my <= ty + k_tabbar_h );

        if ( hovered && click )
            current_tab = k_vals[ i ];

        zdraw::rgba bg =
            active  ? zdraw::rgba{ 30, 30, 30, 200 } :
            hovered ? zdraw::rgba{ 22, 22, 22, 180 } :
                      zdraw::rgba{ 14, 14, 14, 160 };

        dl.add_rect_filled( tx, ty, tw, k_tabbar_h, bg );

        auto [lw, lh] = zdraw::measure_text( k_labels[ i ], f );

        zdraw::rgba col = active ? pink : hovered ? white : dim;

        dl.add_text(
            tx + ( tw - lw ) * 0.5f,
            ty + ( k_tabbar_h - lh ) * 0.5f,
            k_labels[ i ], col, f
        );

        if ( active )
            dl.add_line(
                tx + 8.f,      ty + k_tabbar_h - 2.f,
                tx + tw - 8.f, ty + k_tabbar_h - 2.f,
                pink, 2.f
            );
    }

    dl.add_line( wx, wy + k_top, wx + ww, wy + k_top,
                 zdraw::rgba{ 30, 30, 30, 255 }, 1.f );
}

// ── Tab: visuals ───────────────────────────────────────────────────────────
static void draw_visuals( float col1, float col2, float gap )
{
    // ── ESP ──
    if ( zui::begin_group_box( "esp", col1 ) )
    {
        zui::checkbox( "enable", esp::enabled );
        zui::separator( );

        zui::checkbox( "names", esp::enableNameESP );
        zui::same_line( );
        zui::color_picker( "##names", esp::nameColor, 80.f );

        zui::new_line( );
        zui::separator( );

        zui::checkbox( "boxes", esp::enableBoxes );
        zui::same_line( );
        zui::color_picker( "##boxes", esp::color, 80.f );

        zui::new_line( );
        zui::separator( );

        zui::checkbox( "health", esp::enableHealthESP );
        zui::same_line( );
        zui::color_picker( "##health", esp::healthColor, 80.f );

        zui::new_line( );
        zui::separator( );

        zui::checkbox( "teams", esp::teams );

        zui::end_group_box( );
    }

    zui::same_line( gap );

    // ── Tracers ──
    if ( zui::begin_group_box( "tracers", col2 ) )
    {
        zui::checkbox( "enable", tracers::enabled );
        zui::same_line( );
        zui::color_picker( "##tracers", tracers::color, 80.f );

        zui::new_line( );
        zui::checkbox( "teams", tracers::teams );
        zui::separator( );

        zui::combo( "style", tracers::style, tracer_styles, 2, col2 );

        zui::separator( );

        if ( !tracers::style )
            zui::slider_float( "offset", tracers::offset, 10.f, 80.f );
        else
            zui::combo( "origin", tracers::centered, tracer_centered, 2, col2 );

        zui::end_group_box( );
    }

    zui::new_line( );

    // ── Bones ──
    if ( zui::begin_group_box( "bones esp", col1 ) )
    {
        zui::checkbox( "enable", bones::enabled );
        zui::same_line( );
        zui::color_picker( "##bones", bones::color, 80.f );

        zui::new_line( );
        zui::separator( );

        zui::checkbox( "teams", bones::teams );
        zui::separator( );

        zui::checkbox( "head", bones::head );
        zui::same_line( );
        zui::color_picker( "##headcolor", bones::headColor, 80.f );

        if ( bones::head )
        {
            zui::new_line( );
            zui::checkbox( "filled", bones::filled );
            zui::same_line( );
            zui::color_picker( "##headfilled", bones::filledColor, 80.f );
        }

        zui::separator( );
        zui::end_group_box( );
    }
}

// ── Tab: combat ────────────────────────────────────────────────────────────
static void draw_combat( float col1, float col2, float gap )
{
    // ── Aimbot ──
    if ( zui::begin_group_box( "aimbot", col1 ) )
    {
        zui::checkbox( "enable", aimassist::enabled );
        zui::separator( );

        zui::slider_float( "smoothing", aimassist::smoothing, 0.f, 20.f );
        zui::separator( );

        zui::slider_float( "fov", aimassist::radio, 10.f, 150.f );
        zui::checkbox( "render fov", aimassist::renderRadio );
        zui::separator( );

        zui::checkbox("dead zone enabled", aimassist::deadZoneEnabled);
        zui::slider_float( "dead zone", aimassist::deadZone, 10.f, 100.f );
        zui::checkbox( "render dead zone", aimassist::renderDeadZone );
        zui::separator( );

        zui::combo( "bone", aimbot_bone, bone_selection, 4, col1 );

        zui::end_group_box( );
    }

    zui::same_line( gap );

    // ── Triggerbot ──
    if ( zui::begin_group_box( "triggerbot", col2 ) )
    {
        zui::checkbox( "enable", triggerbot::enabled );
        zui::separator( );

        zui::slider_float( "delay (ms)", triggerbot::delay, 0.f, 200.f );
        zui::separator( );

        zui::checkbox( "randomization", triggerbot::randomization );

        zui::end_group_box( );
    }

    zui::new_line( );

    /*
    if ( zui::begin_group_box( "rcs", col1 ) )
    {
        zui::checkbox( "enable", rcs::enabled );
        zui::separator( );
        zui::slider_float( "smoothing", rcs::smoothing, 0.f, 200.f );
        zui::end_group_box( );
    }        */ 
}

static void draw_settings( float col1, float col2, float gap )
{
    if ( zui::begin_group_box( "information", col1 ) )
    {
        zui::text( "build: " __DATE__ " " __TIME__ );
        zui::text( "dev: github.com/000nico" );
        zui::separator( );

        zui::end_group_box( );
    }

    zui::same_line( gap );

    if ( zui::begin_group_box( "unload cheat", col2 ) )
    {
;
        if ( zui::button( "destruct", col2 - 24.f, 35.f ) )
        {
            exit();
            destroyed=true;
        }

        zui::end_group_box( );
    }
}

// ── Tab: config ────────────────────────────────────────────────────────────
static void draw_config( float col1, float col2 )
{
    config_list = config::getConfigs( );

    std::vector<const char*> config_names;
    config_names.reserve( config_list.size( ) );
    for ( auto& s : config_list )
        config_names.push_back( s.c_str( ) );

    if ( zui::begin_group_box( "config", col1 ) )
    {
        zui::text( "config name" );
        zui::text_input( "##saveName", config_name, 64, "config file name..." );

        if ( zui::button( "save", 100.f, 24.f ) )
            config::save( config_name + ".json" );

        zui::separator( );

        if ( !config_names.empty( ) )
            zui::combo( "configs", selected_config, config_names.data( ), (int)config_names.size( ), col1 );
        else
            zui::text( "no configs found" );

        zui::spacing( 4.f );

        if ( zui::button( "load", 100.f, 24.f ) && !config_list.empty( ) )
            config::load( config_list[ selected_config ] );

        zui::same_line( 4.f );

        if ( zui::button( "delete", 100.f, 24.f ) && !config_list.empty( ) )
        {
            config::del( config_list[ selected_config ] );
            config_list  = config::getConfigs( );
            selected_config = 0;
        }

        zui::end_group_box( );
    }
    
}

// ── Public ─────────────────────────────────────────────────────────────────
void initialize( ID3D11Device*, ID3D11DeviceContext* ) {}
void update( ) {}

void draw( )
{
    apply_theme( );
    zui::begin( );

    static float wx = 100.f, wy = 130.f;
    static float ww = 720.f, wh = 520.f;

    zui::push_style_var( zui::style_var::window_padding_y, k_top + 10.f );

    if ( zui::begin_window( "##gc", wx, wy, ww, wh, false, 480.f, 210.f ) )
    {
        draw_header_tabs( wx, wy, ww );

        zui::pop_style_var( );
        zui::push_style_var( zui::style_var::window_padding_y, 12.f );

        auto [aw, ah] = zui::get_content_region_avail( );
        constexpr float gap  = 5.f;
        float           col1 = aw * 0.55f - gap * 0.5f;
        float           col2 = aw * 0.45f - gap * 0.5f;

        switch ( current_tab )
        {
            case tab::visuals:  draw_visuals( col1, col2, gap ); break;
            case tab::combat:   draw_combat ( col1, col2, gap ); break;
            case tab::config:   draw_config ( col1, col2 );      break;
            case tab::settings: draw_settings( col1, col2, gap ); break;
        }

        zui::end_window( );
    }

    zui::pop_style_var( );
    zui::end( );
}

} // namespace menu