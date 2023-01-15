#include <SDK/foobar2000.h>

#include <string>
#include <thread>

#include <shellscalingapi.h>

typedef cfg_int_t<t_int32> cfg_int32;

// {8E502CCB-BCFB-4f05-9595-0314CD2E5CFA}
static const GUID cfg_timebomb_duration_guid = { 0x8e502ccb,
                                                 0xbcfb,
                                                 0x4f05,
                                                 { 0x95, 0x95, 0x3, 0x14, 0xcd, 0x2e, 0x5c, 0xfa } };
static cfg_int32 cfg_timebomb_duration(cfg_timebomb_duration_guid, 5);

void
g_show_window();
void
g_hide_window();

class tb_commands : public mainmenu_commands
{
    t_uint32 get_command_count();
    GUID get_command(t_uint32 p_index);
    void get_name(t_uint32 p_index, pfc::string_base& p_out);
    bool get_description(t_uint32 p_index, pfc::string_base& p_out);
    GUID get_parent();
    void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback);

    static const GUID cmd_guid;

    double ask_for_duration();
};

template<typename callback_type>
void
cb();

// {2608F059-0164-495a-9435-EA0DB4078246}
const GUID tb_commands::cmd_guid = { 0x2608f059, 0x164, 0x495a, { 0x94, 0x35, 0xea, 0xd, 0xb4, 0x7, 0x82, 0x46 } };

t_uint32
tb_commands::get_command_count()
{
    return 1;
}

GUID
tb_commands::get_command(t_uint32 p_index)
{
    if (p_index == 0)
        return cmd_guid;
    return pfc::guid_null;
}

void
tb_commands::get_name(t_uint32 p_index, pfc::string_base& p_out)
{
    if (p_index == 0)
        p_out = "Stop in...";
}

bool
tb_commands::get_description(t_uint32 p_index, pfc::string_base& p_out)
{
    if (p_index == 0) {
        p_out = "Stop after a specified amount of time.";
        return true;
    }
    return false;
}

GUID
tb_commands::get_parent()
{
    return mainmenu_groups::playback;
}

class tb_stop_cb : public main_thread_callback
{
    void callback_run()
    {
        static_api_ptr_t<playback_control> pbc;
        pbc->stop();
    }
};

class tb_exit_cb : public main_thread_callback
{
    void callback_run()
    {
        GUID g;
        mainmenu_commands::g_find_by_name("Exit", g);
        mainmenu_commands::g_execute(g);
    }
};

class window_tutorial1
{
  private:
    static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static ATOM g_class_atom;
    static LONG g_refcount;

    HWND m_hWnd;
    HFONT m_hFont;

    LRESULT process_message(UINT msg, WPARAM wParam, LPARAM lParam);

  public:
    window_tutorial1();
    ~window_tutorial1();

    HWND create(HWND p_hWndParent);
    void destroy();

    inline operator HWND() { return m_hWnd; }
    BOOL is_window() { return ::IsWindow(m_hWnd); }
};

static window_tutorial1 g_wnd;

ATOM window_tutorial1::g_class_atom = 0;
LONG window_tutorial1::g_refcount = 0;

enum Controls
{
    BTN_STOP = 200,
    BTN_EXIT = 300,
    EDT_DURATION = 400
};

window_tutorial1::window_tutorial1()
{
    m_hWnd = NULL;
    m_hFont = NULL;
}

window_tutorial1::~window_tutorial1() {}

HWND
window_tutorial1::create(HWND p_hWndParent)
{
    assert(m_hWnd == NULL);

    if (g_refcount == 0) {
        // set up a unique class name to avoid clashes with other tutorial steps
        static int dummy = 0;
        pfc::string8 class_name;
        class_name << "CLS" << pfc::format_hex((t_uint64)&dummy, sizeof(&dummy) * 2);
        pfc::stringcvt::string_os_from_utf8 os_class_name(class_name);

        if (g_class_atom == NULL) {
            WNDCLASS wc;
            memset(&wc, 0, sizeof(wc));
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
            wc.lpfnWndProc = &WindowProc;
            wc.hInstance = core_api::get_my_instance();
            wc.hCursor = LoadCursor(0, IDC_ARROW);
            wc.hbrBackground = 0;
            wc.lpszClassName = os_class_name;
            wc.cbWndExtra = sizeof(window_tutorial1*);
            g_class_atom = RegisterClass(&wc);
        }
    }

    if (g_class_atom != NULL) {
        g_refcount++;

        RECT r = { 0, 0, 256, 64 };
        AdjustWindowRectEx(
          &r, WS_POPUP | WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, WS_EX_TOOLWINDOW);
        return uCreateWindowEx(WS_EX_TOOLWINDOW,
                               (const char*)g_class_atom,
                               "Enter minutes",
                               WS_POPUP | WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               r.right - r.left,
                               r.bottom - r.top,
                               core_api::get_main_window(),
                               /* no menu */ 0,
                               core_api::get_my_instance(),
                               this);
    }

    return NULL;
}

void
window_tutorial1::destroy()
{
    // Destroy the window.
    if (m_hWnd) {
        DestroyWindow(m_hWnd);

        g_refcount--;

        if (g_refcount == 0) {
            UnregisterClass((LPCTSTR)g_class_atom, core_api::get_my_instance());
            g_class_atom = 0;
        }
    }
}

LRESULT WINAPI
window_tutorial1::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    window_tutorial1* _this = reinterpret_cast<window_tutorial1*>(GetWindowLongPtr(hWnd, 0));

    if (msg == WM_NCCREATE) {
        LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        _this = reinterpret_cast<window_tutorial1*>(lpCreateStruct->lpCreateParams);
        SetWindowLongPtr(hWnd, 0, (LONG_PTR)_this);
        _this->m_hWnd = hWnd;
    }

    LRESULT lResult = 0;
    if (_this != 0) {
        try {
            lResult = _this->process_message(msg, wParam, lParam);
        } catch (const std::exception&) {
        }
    } else {
        lResult = DefWindowProc(hWnd, msg, wParam, lParam);
    }

    if (msg == WM_NCDESTROY) {
        _this->m_hWnd = 0;
    }

    return lResult;
}

LRESULT
window_tutorial1::process_message(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

        case WM_CREATE: {
            RECT clientRect{};
            GetClientRect(*this, &clientRect);
            int const xMid = clientRect.right / 2, xMax = clientRect.right, yMid = clientRect.bottom / 2,
                      yMax = clientRect.bottom;
            uCreateWindowEx(WS_EX_NOPARENTNOTIFY,
                            "Edit",
                            "",
                            WS_CHILD | WS_VISIBLE | ES_NUMBER,
                            0,
                            0,
                            xMax,
                            yMid,
                            *this,
                            (HMENU)EDT_DURATION,
                            0,
                            0);
            uCreateWindowEx(WS_EX_NOPARENTNOTIFY,
                            "Button",
                            "Stop",
                            WS_CHILD | WS_VISIBLE,
                            0,
                            yMid,
                            xMid,
                            yMax - yMid,
                            *this,
                            (HMENU)BTN_STOP,
                            0,
                            0);
            uCreateWindowEx(WS_EX_NOPARENTNOTIFY,
                            "Button",
                            "Exit",
                            WS_CHILD | WS_VISIBLE,
                            xMid,
                            yMid,
                            xMax - xMid,
                            yMax - yMid,
                            *this,
                            (HMENU)BTN_EXIT,
                            0,
                            0);
            SetDlgItemInt(*this, EDT_DURATION, cfg_timebomb_duration, FALSE);
        } break;

        case WM_DESTROY: {
            // Notify the window placement variable that our window
            // was destroyed. This will also update the variables value
            // with the current window position and size.
        } break;

        case WM_CLOSE: {
            // Hide and disable the window.
            g_hide_window();
        } break;

        case WM_KEYDOWN:
            switch (wParam) {

                case VK_ESCAPE: {
                    // Hide and disable the window.
                    g_hide_window();
                } break;
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            if (BeginPaint(m_hWnd, &ps) != NULL) {

                // We will only paint our client area in the default window color for now.
                FillRect(ps.hdc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));

                EndPaint(m_hWnd, &ps);
            }
        } break;

        case WM_COMMAND: {
            if (wParam == BTN_STOP || wParam == BTN_EXIT) {
                BOOL translated;
                cfg_timebomb_duration = GetDlgItemInt(*this, EDT_DURATION, &translated, FALSE);
                if (!translated)
                    return 0;
            }
            switch (wParam) {
                case BTN_STOP: {
                    std::thread t(&cb<tb_stop_cb>);
                    t.detach();
                    g_hide_window();
                } break;
                case BTN_EXIT: {
                    std::thread t(&cb<tb_exit_cb>);
                    t.detach();
                    g_hide_window();
                } break;
            }
        } break;

        default:
            return DefWindowProc(m_hWnd, msg, wParam, lParam);
    }

    return 0;
}

static void
g_show_window()
{
    if (!g_wnd.is_window()) {
        g_wnd.create(core_api::get_main_window());
    }
}

static void
g_hide_window()
{
    // Destroy the window.
    g_wnd.destroy();
}

template<typename callback_type>
void
cb()
{
    Sleep(cfg_timebomb_duration * 60 * 1000);
    static_api_ptr_t<main_thread_callback_manager> mtcm;
    service_ptr_t<callback_type> cb = new service_impl_t<callback_type>();
    mtcm->add_callback(cb);
}

void
tb_commands::execute(t_uint32 p_index, service_ptr_t<service_base> foo)
{
    if (p_index == 0)
        g_show_window();
}

static mainmenu_commands_factory_t<tb_commands> tbcmds;

DECLARE_COMPONENT_VERSION("Timebomb", "0.0.3", "Zao");
