#include "glguts.h"
#include "gfx_1.3.h"
#include "parallel_imp.h"
#include "wgl_ext.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <commctrl.h>

static HWND statusbar = NULL;
static HDC dc;
static HGLRC glrc;
static HGLRC glrc_core;
static bool fullscreen;

// framebuffer texture states
int32_t window_width;
int32_t window_height;
int32_t window_fullscreen;
bool window_integerscale;
bool window_vsync;
bool window_widescreen;

#include "gl_core_3_3.c"
#define SHADER_HEADER "#version 330 core\n"
#define TEX_FORMAT GL_RGBA
#define TEX_TYPE GL_UNSIGNED_BYTE
#define TEX_NUM 3

static GLuint program;
static GLuint vao;
static GLuint buffer;
static GLuint texture[TEX_NUM];
static uint8_t *buffer_data;
static uint32_t buffer_size = (640*8) * (480*8) * sizeof(uint32_t);

int32_t tex_width[TEX_NUM];
int32_t tex_height[TEX_NUM];

static bool m_fullscreen;
static int rotate_buffer;

#define MSG_BUFFER_LEN 256

void msg_error(const char * err, ...)
{
    va_list arg;
    va_start(arg, err);
    char buf[MSG_BUFFER_LEN];
    vsprintf_s(buf, sizeof(buf), err, arg);
    MessageBoxA(0, buf, "parallel : fatal error", MB_OK);
    va_end(arg);
    exit(0);
}

void msg_warning(const char* err, ...)
{
    va_list arg;
    va_start(arg, err);
    char buf[MSG_BUFFER_LEN];
    vsprintf_s(buf, sizeof(buf), err, arg);
    MessageBox(0, buf, "parallel" ": warning", MB_OK);
    va_end(arg);
}

void msg_debug(const char* err, ...)
{
    va_list arg;
    va_start(arg, err);
    char buf[MSG_BUFFER_LEN];
    vsprintf_s(buf, sizeof(buf), err, arg);
    strcat_s(buf, sizeof(buf), "\n");
    OutputDebugStringA(buf);
    va_end(arg);
}

void win32_client_resize(HWND hWnd, HWND hStatus, int32_t nWidth, int32_t nHeight)
{
    RECT rclient;
    GetClientRect(hWnd, &rclient);

    RECT rwin;
    GetWindowRect(hWnd, &rwin);

    if (hStatus) {
        RECT rstatus;
        GetClientRect(hStatus, &rstatus);

        rclient.bottom -= rstatus.bottom;
    }

    POINT pdiff;
    pdiff.x = (rwin.right - rwin.left) - rclient.right;
    pdiff.y = (rwin.bottom - rwin.top) - rclient.bottom;

    MoveWindow(hWnd, rwin.left, rwin.top, nWidth + pdiff.x, nHeight + pdiff.y, TRUE);
}

static int TestPointer(const PROC pTest)
{
    if (!pTest) {
        return 0;
    }

    ptrdiff_t iTest = (ptrdiff_t)pTest;

    return iTest != 1 && iTest != 2 && iTest != 3 && iTest != -1;
}

void* IntGetProcAddress(const char *name)
{
    HMODULE glMod = NULL;
    PROC pFunc = wglGetProcAddress((LPCSTR)name);
    if (TestPointer(pFunc)) {
        return pFunc;
    }
    glMod = GetModuleHandleA("OpenGL32.dll");
    return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}

#ifdef _DEBUG
static void gl_check_errors(void)
{
    GLenum err;
    static int32_t invalid_op_count = 0;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // if gl_check_errors is called from a thread with no valid
        // GL context, it would be stuck in an infinite loop here, since
        // glGetError itself causes GL_INVALID_OPERATION, so check for a few
        // cycles and abort if there are too many errors of that kind
        if (err == GL_INVALID_OPERATION)
        {
            if (++invalid_op_count >= 100)
            {
                printf("gl_check_errors: invalid OpenGL context!");
            }
        }
        else
        {
            invalid_op_count = 0;
        }

        char *err_str;
        switch (err)
        {
        case GL_INVALID_OPERATION:
            err_str = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            err_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            err_str = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            err_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            err_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            err_str = "unknown";
        }
        printf("gl_check_errors: %d (%s)", err, err_str);
    }
}
#else
#define gl_check_errors(...)
#endif

static GLuint gl_shader_compile(GLenum type, const GLchar *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint param;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);

    if (!param)
    {
        GLchar log[4096];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        printf("%s shader error: %s\n", type == GL_FRAGMENT_SHADER ? "Frag" : "Vert", log);
    }

    return shader;
}

static GLuint gl_shader_link(GLuint vert, GLuint frag)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint param;
    glGetProgramiv(program, GL_LINK_STATUS, &param);

    if (!param)
    {
        GLchar log[4096];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        printf("Shader link error: %s\n", log);
    }

    glDeleteShader(frag);
    glDeleteShader(vert);

    return program;
}

void screen_write(struct frame_buffer *fb)
{
    bool buffer_size_changed = tex_width[rotate_buffer] != fb->width || tex_height[rotate_buffer] != fb->height;
    char* offset = rotate_buffer * buffer_size;

    // check if the framebuffer size has changed
    if (buffer_size_changed)
    {
        tex_width[rotate_buffer] = fb->width;
        tex_height[rotate_buffer] = fb->height;
        // set pitch for all unpacking operations
        glPixelStorei(GL_UNPACK_ROW_LENGTH, fb->pitch);
        // reallocate texture buffer on GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_width[rotate_buffer],
                     tex_height[rotate_buffer], 0, TEX_FORMAT, TEX_TYPE, offset);
    }
    else
    {
        // copy local buffer to GPU texture buffer
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width[rotate_buffer], tex_height[rotate_buffer],
                        TEX_FORMAT, TEX_TYPE, offset);
    }

    rotate_buffer = (rotate_buffer + 1) % TEX_NUM;
}

void screen_read(struct frame_buffer *fb, bool alpha)
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    fb->width = vp[2] & ~3;
    fb->height = vp[3];
    fb->pitch = fb->width;

    if (fb->pixels)
    {
        glReadPixels(vp[0], vp[1], fb->width, fb->height, alpha ? GL_RGBA : GL_RGB, TEX_TYPE, fb->pixels);
    }
}

void gl_screen_render()
{
    RECT rect;
    GetClientRect(gfx.hWnd, &rect);


 // status bar covers the client area, so exclude it from calculation
    RECT statusrect;
    SetRectEmpty(&statusrect);

    if (statusbar && GetClientRect(statusbar, &statusrect)) {
        rect.bottom -= statusrect.bottom;
    }

    int32_t win_width = rect.right - rect.left;
    int32_t win_height = rect.bottom - rect.top;

    if(window_integerscale)
    {
        unsigned width = win_width;
	    unsigned height = win_height;
	    int pad_x = 0, pad_y = 0;
	    unsigned base_h = 480;
	    unsigned base_w = 640;
	    if (width >= base_w && height >= base_h)
	    {
	    	unsigned scale = min(width / base_w, height / base_h);
	    	pad_x = width - base_w * scale;
	    	pad_y = height - base_h * scale;
	    }
	    width -= pad_x;
	    height -= pad_y;
	    glViewport(pad_x / 2, pad_y / 2, width, height);
    }
    else
    {
        int32_t vp_x = 0;
        int32_t vp_y = statusrect.bottom;
        int display_width = 640 * vk_rescaling;
        int display_height = 480 * vk_rescaling;
        if(window_widescreen)
        display_height = (480 * vk_rescaling)* 3 / 4;
        int32_t hw =  display_height * win_width;
        int32_t wh = display_width * win_height;
    
        // add letterboxes or pillarboxes if the window has a different aspect ratio
        // than the current display mode
        if (hw > wh) {
            int32_t w_max = wh / display_height;
            vp_x += (win_width - w_max) / 2;
            win_width = w_max;
        } else if (hw < wh) {
            int32_t h_max = hw / display_width;
            vp_y += (win_height - h_max) / 2;
            win_height = h_max;
        }
        // configure viewport
        glViewport(vp_x, vp_y, win_width, win_height);
    }

    
    // draw fullscreen triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void gl_screen_clear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_screen_close(void)
{
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glDeleteTextures(TEX_NUM, &texture[0]);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &buffer);
    glDeleteProgram(program);
}

uint8_t* screen_get_texture_data()
{
    return buffer_data + (rotate_buffer * buffer_size);
}

void screen_init()
{
    memset(tex_width, 0, sizeof(int32_t) * TEX_NUM);
    memset(tex_height, 0, sizeof(int32_t) * TEX_NUM);
    rotate_buffer = 0;

    if (gfx.hStatusBar)statusbar = gfx.hStatusBar;
    if(!statusbar)
    {
        statusbar = FindWindowExA(gfx.hWnd, NULL, STATUSCLASSNAME, NULL);
        if (statusbar == NULL)
        {
            statusbar = FindWindowExA(gfx.hWnd, NULL, "msctls_statusbar32", NULL);
        }

    }
    /* Get the core Video Extension function pointers from the library handle */
     if (!m_fullscreen) {
        LONG style = GetWindowLong(gfx.hWnd, GWL_STYLE);

        if ((style & (WS_SIZEBOX | WS_MAXIMIZEBOX)) == 0) {
            style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
            SetWindowLong(gfx.hWnd, GWL_STYLE, style);

            // Fix client size after changing the window style, otherwise the PJ64
            // menu will be displayed incorrectly.
            // For some reason, this needs to be called twice, probably because the
            // style set above isn't applied immediately.
            for (int i = 0; i < 2; i++) {
                win32_client_resize(gfx.hWnd, statusbar, window_width, window_height);
            }
        }
    }

    PIXELFORMATDESCRIPTOR win_pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
        PFD_TYPE_RGBA, // The kind of framebuffer. RGBA or palette.
        32,            // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, // Number of bits for the depthbuffer
        8,  // Number of bits for the stencilbuffer
        0,  // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    dc = GetDC(gfx.hWnd);
    if (!dc) {
        msg_error("Can't get device context.");

    }

    int32_t win_pf = ChoosePixelFormat(dc, &win_pfd);
    if (!win_pf) {
        msg_error("Can't choose pixel format.");
    }
    SetPixelFormat(dc, win_pf, &win_pfd);

    // create legacy context, required for wglGetProcAddress to work properly
    glrc = wglCreateContext(dc);
    if (!glrc || !wglMakeCurrent(dc, glrc)) {
        msg_error("Can't create OpenGL context.");
    }

    // load wgl extension
    wgl_LoadFunctions(dc);

    // attributes for a 3.3 core profile without all the legacy stuff
    GLint attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    // create the actual context
    glrc_core = wglCreateContextAttribsARB(dc, glrc, attribs);
    if (!glrc_core || !wglMakeCurrent(dc, glrc_core)) {
        // rendering probably still works with the legacy context, so just send
        // a warning
        msg_warning("Can't create OpenGL 3.3 core context.");
    }

    // enable vsync
    wglSwapIntervalEXT(window_vsync);

    // load OpenGL function pointers
    ogl_LoadFunctions();

    // shader sources for drawing a clipped full-screen triangle. the geometry
    // is defined by the vertex ID, so a VBO is not required.
    const GLchar *vert_shader =
        SHADER_HEADER
        "out vec2 uv;\n"
        "void main(void) {\n"
        "    uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n"
        "    gl_Position = vec4(uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);\n"
        "}\n";

    const GLchar *frag_shader =
        SHADER_HEADER
        "in vec2 uv;\n"
        "layout(location = 0) out vec4 color;\n"
        "uniform sampler2D tex0;\n"
        "void main(void) {\n"
        "color = texture(tex0, uv);\n"
        "}\n";

    // compile and link OpenGL program
    GLuint vert = gl_shader_compile(GL_VERTEX_SHADER, vert_shader);
    GLuint frag = gl_shader_compile(GL_FRAGMENT_SHADER, frag_shader);
    program = gl_shader_link(vert, frag);
    glUseProgram(program);

    // prepare dummy VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // prepare texture
    glGenTextures(TEX_NUM, &texture[0]);
    for (int i = 0; i < TEX_NUM; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferStorage(GL_PIXEL_UNPACK_BUFFER, buffer_size * TEX_NUM, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    buffer_data = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, buffer_size * TEX_NUM, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    // check if there was an error when using any of the commands above
    gl_check_errors();
}

void screen_swap(bool blank)
{
    if (IsIconic(gfx.hWnd)) {
        return;
    }

    // clear current buffer, indicating the start of a new frame
    gl_screen_clear();

    if (!blank)
    {
        gl_screen_render();
    }

   SwapBuffers(dc);

}

void screen_toggle_fullscreen()
{
    static HMENU old_menu;
    static LONG old_style;
    static WINDOWPLACEMENT old_pos;

    m_fullscreen = !m_fullscreen;

    if (m_fullscreen) {
        // hide curser
        ShowCursor(FALSE);

        // hide status bar
        if (statusbar) {
            ShowWindow(statusbar, SW_HIDE);
        }

        // disable menu and save it to restore it later
        old_menu = GetMenu(gfx.hWnd);
        if (old_menu) {
            SetMenu(gfx.hWnd, NULL);
        }

        // save old window position and size
        GetWindowPlacement(gfx.hWnd, &old_pos);

        // use virtual screen dimensions for fullscreen mode
        int32_t vs_width = GetSystemMetrics(SM_CXSCREEN);
        int32_t vs_height = GetSystemMetrics(SM_CYSCREEN);

        // disable all styles to get a borderless window and save it to restore
        // it later
        old_style = GetWindowLong(gfx.hWnd, GWL_STYLE);
        LONG style = WS_VISIBLE;
        SetWindowLong(gfx.hWnd, GWL_STYLE, style);

        // resize window so it covers the entire virtual screen
        SetWindowPos(gfx.hWnd, HWND_TOP, 0, 0, vs_width, vs_height, SWP_SHOWWINDOW);
    }
    else {
        // restore cursor
        ShowCursor(TRUE);

        // restore status bar
        if (statusbar) {
            ShowWindow(statusbar, SW_SHOW);
        }

        // restore menu
        if (old_menu) {
            SetMenu(gfx.hWnd, old_menu);
            old_menu = NULL;
        }

        // restore style
        SetWindowLong(gfx.hWnd, GWL_STYLE, old_style);

        // restore window size and position
        SetWindowPlacement(gfx.hWnd, &old_pos);
    }
}

void screen_close(void)
{
    gl_screen_close();

     if (glrc_core) {
        wglDeleteContext(glrc_core);
    }

    wglDeleteContext(glrc);

}
