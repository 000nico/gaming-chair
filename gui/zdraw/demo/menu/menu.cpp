#include "../global.hpp"
#include "../../../modules/modules.hpp"

namespace menu {

// ── State ─────────────────────────────────────────────────────────────────
static bool esp_enabled      { true };
static bool esp_names        { true };
static bool esp_boxes        { true };
static bool esp_health       { true };

static zdraw::rgba esp_col_player { 255, 140, 0, 255 };
static zdraw::rgba esp_col_names  { 255, 255, 255, 255 };
static zdraw::rgba esp_col_health { 80, 220, 80, 255 };

static bool esp_tracers      { false };
static int  esp_tracer_style { 0 };

static const char* tracer_styles[] = { "arrows", "lines" };
static const char* tracer_centered[] = { "down", "centered" };
static const char* bone_selection[] = {"head", "neck", "chest", "pelvis"};
// ── Theme ─────────────────────────────────────────────────────────────────
static void apply_theme()
{
    auto& s = zui::get_style();

    // ── pastel palette ──
    zdraw::rgba bg     = { 10, 10, 10, 255 };
    zdraw::rgba bg2    = { 14, 14, 14, 255 };
    zdraw::rgba bg3    = { 20, 20, 20, 255 };
    zdraw::rgba border = { 28, 28, 28, 255 };

    zdraw::rgba text   = { 230, 230, 230, 255 };

    // pastel pink (lavado)
    zdraw::rgba pink   = { 248, 210, 228, 255 };

    // hover/soft pink variant
    zdraw::rgba pink_soft = { 255, 230, 242, 255 };

    s.window_bg        = bg;
    s.window_border    = border;

    s.nested_bg        = bg2;
    s.nested_border    = border;

    s.group_box_bg     = bg2;
    s.group_box_border = border;

    // headers / titles
    s.group_box_title_text = pink;
    s.text   = text;
    s.accent = pink;

    // checkbox
    s.checkbox_bg     = bg2;
    s.checkbox_border = border;
    s.checkbox_check  = pink;

    // buttons
    s.button_bg       = bg2;
    s.button_border   = border;
    s.button_hovered  = bg3;
    s.button_active   = { 28, 28, 28, 255 };

    // combo
    s.combo_bg            = bg2;
    s.combo_border        = border;
    s.combo_arrow         = pink;
    s.combo_hovered       = bg3;
    s.combo_popup_bg      = bg;
    s.combo_popup_border  = border;
    s.combo_item_hovered  = bg3;
    s.combo_item_selected = { 45, 45, 45, 255 };

    // color picker
    s.color_picker_bg     = bg2;
    s.color_picker_border = border;

    // spacing
    s.item_spacing_y   = 7.f;
    s.window_padding_x = 12.f;
    s.window_padding_y = 12.f;
}

// ── Tabs ──────────────────────────────────────────────────────────────────
enum class tab { visuals, combat, config, settings };
static tab current_tab{ tab::visuals };

static constexpr int k_n = 4;
static constexpr const char* k_labels[k_n] = { "visuals", "combat", "config", "settings" };
static constexpr tab k_vals[k_n] = { tab::visuals, tab::combat, tab::config, tab::settings };

static constexpr float k_header_h = 28.f;
static constexpr float k_tabbar_h = 24.f;
static constexpr float k_top = k_header_h + k_tabbar_h;

// ── HEADER ───────────────────────────────────────────────────────────────
static void draw_header_tabs(float wx, float wy, float ww)
{
    auto& dl = zdraw::get_draw_list(zdraw::draw_layer::window);
    auto* f  = zdraw::get_default_font();

    zdraw::rgba white = { 235, 235, 235, 255 };
    zdraw::rgba pink  = { 255, 182, 213, 255 };
    zdraw::rgba dim   = { 120, 120, 120, 255 };
    zdraw::rgba pink_soft = { 255, 182, 213, 255 };

    float x = wx + 10.f;
    float y = wy + 8.f;

    auto [w1, h1] = zdraw::measure_text("gaming", f);
    dl.add_text(x, y, "gaming", white, f);
    x += w1;

    auto [w2, h2] = zdraw::measure_text("chair", f);
    dl.add_text(x, y, "chair", pink_soft, f);
    x += w2;

    dl.add_text(x, y, ".cc", white, f);

    float tw = ww / k_n;

    float mx = zui::detail::mouse_x();
    float my = zui::detail::mouse_y();
    bool click = zui::detail::mouse_clicked();

    for (int i = 0; i < k_n; i++)
    {
        float tx = wx + i * tw;
        float ty = wy + k_header_h;

        bool active  = (current_tab == k_vals[i]);
        bool hovered = (mx >= tx && mx <= tx + tw && my >= ty && my <= ty + k_tabbar_h);

        if (hovered && click)
            current_tab = k_vals[i];

        zdraw::rgba bg =
            active  ? zdraw::rgba{ 30, 30, 30, 200 } :
            hovered ? zdraw::rgba{ 22, 22, 22, 180 } :
                      zdraw::rgba{ 14, 14, 14, 160 };

        dl.add_rect_filled(tx, ty, tw, k_tabbar_h, bg);

        auto [lw, lh] = zdraw::measure_text(k_labels[i], f);

        zdraw::rgba col =
            active ? pink :
            hovered ? white :
                      dim;

        dl.add_text(
            tx + (tw - lw) * 0.5f,
            ty + (k_tabbar_h - lh) * 0.5f,
            k_labels[i],
            col,
            f
        );

        if (active)
            dl.add_line(tx + 8.f, ty + k_tabbar_h - 2.f,
                        tx + tw - 8.f, ty + k_tabbar_h - 2.f,
                        pink, 2.f);
    }

    dl.add_line(wx, wy + k_top, wx + ww, wy + k_top,
                zdraw::rgba{ 30, 30, 30, 255 }, 1.f);
}

// ── MAIN ──────────────────────────────────────────────────────────────────
void initialize(ID3D11Device*, ID3D11DeviceContext*) {}
void update() {}

void draw()
{
    apply_theme();
    zui::begin();

    const auto [sw, sh] = zdraw::get_display_size();
    zdraw::get_draw_list(zdraw::draw_layer::background)
        .add_rect_filled(0.f, 0.f, (float)sw, (float)sh,
                         zdraw::rgba{ 8, 8, 8, 255 });

    static float wx = 100.f, wy = 130.f;
    static float ww = 720.f, wh = 520.f;

    zui::push_style_var(zui::style_var::window_padding_y, k_top + 10.f);

    if (zui::begin_window("##gc", wx, wy, ww, wh, false, 480.f, 210.f))
    {
        draw_header_tabs(wx, wy, ww);

        if (current_tab == tab::visuals)
        {
            auto [aw, ah] = zui::get_content_region_avail();

            float gap = 5.f;
            float col1 = aw * 0.55f - gap * 0.5f;
            float col2 = aw * 0.45f - gap * 0.5f;

            // ESP
			if (zui::begin_group_box("esp", col1))
			{
				zui::checkbox("enable", esp::enabled);

				zui::separator();

				zui::checkbox("names", esp::enableNameESP);
				zui::same_line();
				zui::color_picker("##names", esp::nameColor, 80.f);

				zui::new_line();
				zui::separator();

				
				zui::checkbox("boxes", esp::enableBoxes);
				zui::same_line();
				zui::color_picker("##boxes", esp::color, 80.f);

				zui::new_line();
				zui::separator();
				
				
				zui::checkbox("health", esp::enableHealthESP);
				zui::same_line();
				zui::color_picker("##health", esp::healthColor, 80.f);

                zui::new_line();
				zui::separator();

                zui::checkbox("teams", esp::teams);
                zui::separator();

				zui::end_group_box();
			}

            zui::same_line(gap);

            // TRACERS
            if (zui::begin_group_box("tracers", col2))
            {
                zui::checkbox("enable", tracers::enabled);
				zui::same_line();
				zui::color_picker("##tracers", tracers::color, 80.f);

                zui::new_line();
                zui::checkbox("teams", tracers::teams);

				zui::separator();

				zui::new_line();
                zui::combo("style", tracers::style, tracer_styles, 2, col2);
				
				if(!tracers::style){
					zui::separator();
					zui::new_line();
					zui::slider_float("offset", tracers::offset, 10.0f, 80.0f);
				}
                else{
                    zui::separator();
                    zui::new_line();
                    zui::combo("origin", tracers::centered, tracer_centered, 2, col2);
                }

                zui::end_group_box();
            }

            zui::new_line();

            // BONES
            if (zui::begin_group_box("bones esp", col1))
            {
                zui::checkbox("enable", bones::enabled);
				zui::same_line();
				zui::color_picker("##bones", bones::color, 80.f);

                zui::new_line();
                zui::separator();

                zui::checkbox("teams", bones::teams);

                zui::separator();
                zui::new_line();

                zui::checkbox("head", bones::head);
                zui::same_line();
                zui::color_picker("##headcolor", bones::headColor, 80.f);

                if(bones::head){
                    zui::new_line();
                    zui::checkbox("filled", bones::filled);
                    zui::same_line();
                    zui::color_picker("##headfilled", bones::filledColor, 80.f);
                }
                
				zui::separator();
				
                zui::end_group_box();
            }
        }

        if(current_tab == tab::combat){
            auto [aw, ah] = zui::get_content_region_avail();

            float gap = 5.f;
            float col1 = aw * 0.55f - gap * 0.5f;
            float col2 = aw * 0.45f - gap * 0.5f;

            // aim assist
            if (zui::begin_group_box("aimbot", col1))
            {
                zui::checkbox("enable", aimassist::enabled);

                zui::new_line();
                zui::separator();

                zui::slider_float("smoothing", aimassist::smoothing, 0.0f, 50.0f);

                zui::new_line();
                zui::separator();

                zui::slider_float("radio", aimassist::radio, 10.0f, 100.0f);
                zui::checkbox("render radio", aimassist::renderRadio);
                
                zui::new_line();
                zui::separator();

                zui::slider_float("dead zone", aimassist::deadZone, 10.0f, 100.0f);
                zui::checkbox("render dead zone", aimassist::renderDeadZone);

                zui::new_line();
                zui::separator();

                zui::combo("bone selection", tracers::centered, bone_selection, 4, col2);

                zui::end_group_box();
            }

            zui::same_line(gap);

            if (zui::begin_group_box("triggerbot", col2))
            {
                zui::checkbox("enable", triggerbot::enabled);

                zui::new_line();
                zui::separator();

                zui::slider_float("delay", triggerbot::delay, 0.0f, 200.0f);

                zui::new_line();
                zui::separator();

                zui::checkbox("randomization", triggerbot::randomization);

                zui::end_group_box();
            }

            zui::new_line();

            if (zui::begin_group_box("RCS", col1))
            {
                zui::checkbox("enable", triggerbot::enabled);

                zui::new_line();
                zui::separator();

                zui::slider_float("smoothing", triggerbot::delay, 0.0f, 200.0f);

                zui::new_line();
                zui::separator();

                zui::checkbox("delay", triggerbot::randomization);

                zui::end_group_box();
            }
        }
        zui::end_window();
    }

    zui::pop_style_var();
    zui::end();
}

} // namespace menu