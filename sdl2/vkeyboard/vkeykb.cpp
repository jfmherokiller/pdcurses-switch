/*
 *  Copyright (C) 2002-2017  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <list>
#include <string>

#include "SDL.h"
#include "vkeybd.h"


// Special names in definition file: display and apply/cancel buttons
#define KEY_DISPLAY "DISPLAY"
#define KEY_APPLY "APPLY"
#define KEY_CANCEL "CANCEL"

// A ratio to compute the "speed" of mouse cursor when emulated from joystick
#define JOYSTICK_OFFSET_RATIO 80

// Blinking speed of cursor (in ms)
#define CURSOR_DELAY 250
// Delay between two joystick movements (in ms)
#define JOYSTICK_DELAY 12
// Delay between two keys sent to keybord (in ms)
#define KEYBOARD_FLUSH_DELAY 60

// Begin and end characters for display
const char *OPEN_DEFAULT = "[";
const char *CLOSE_DEFAULT = "]";
const char *OPEN_PRESSED = "\x19";
const char *CLOSE_PRESSED = "\x19";
const char *OPEN_RELEASED = "\x18";
const char *CLOSE_RELEASED = "\x18";

// Table ok keys: name in definition file, name to display, code to send to keyboard
struct KeyboardKeyMap {
    const char *name;
    const char *disp_name;
    SDL_Scancode key;
};

static KeyboardKeyMap vkeybd_map[] = {
        {"esc",         "Esc",      SDL_SCANCODE_ESCAPE},
        {"f1",          "F1",       SDL_SCANCODE_F1},
        {"f2",          "F2",       SDL_SCANCODE_F2},
        {"f3",          "F3",       SDL_SCANCODE_F3},
        {"f4",          "F4",       SDL_SCANCODE_F4},
        {"f5",          "F5",       SDL_SCANCODE_F5},
        {"f6",          "F6",       SDL_SCANCODE_F6},
        {"f7",          "F7",       SDL_SCANCODE_F7},
        {"f8",          "F8",       SDL_SCANCODE_F8},
        {"f9",          "F9",       SDL_SCANCODE_F9},
        {"f10",         "F10",      SDL_SCANCODE_F10},
        {"f11",         "F11",      SDL_SCANCODE_F11},
        {"f12",         "F12",      SDL_SCANCODE_F12},

        {"grave",       "`",        SDL_SCANCODE_GRAVE},
        {"1",           "1",        SDL_SCANCODE_1},
        {"2",           "2",        SDL_SCANCODE_2},
        {"3",           "3",        SDL_SCANCODE_3},
        {"4",           "4",        SDL_SCANCODE_4},
        {"5",           "5",        SDL_SCANCODE_5},
        {"6",           "6",        SDL_SCANCODE_6},
        {"7",           "7",        SDL_SCANCODE_7},
        {"8",           "8",        SDL_SCANCODE_8},
        {"9",           "9",        SDL_SCANCODE_9},
        {"0",           "0",        SDL_SCANCODE_0},
        {"minus",       "-",        SDL_SCANCODE_MINUS},
        {"equals",      "=",        SDL_SCANCODE_EQUALS},
        {"backspace",   "<--",      SDL_SCANCODE_BACKSPACE},

        {"tab",         "Tab",      SDL_SCANCODE_TAB},
        {"q",           "q",        SDL_SCANCODE_Q},
        {"w",           "w",        SDL_SCANCODE_W},
        {"e",           "e",        SDL_SCANCODE_E},
        {"r",           "r",        SDL_SCANCODE_R},
        {"t",           "t",        SDL_SCANCODE_T},
        {"y",           "y",        SDL_SCANCODE_Y},
        {"u",           "u",        SDL_SCANCODE_U},
        {"i",           "i",        SDL_SCANCODE_I},
        {"o",           "o",        SDL_SCANCODE_O},
        {"p",           "p",        SDL_SCANCODE_P},
        {"lbracket",    "[",        SDL_SCANCODE_LEFTBRACKET},
        {"rbracket",    "]",        SDL_SCANCODE_RIGHTBRACKET},
        {"backslash",   "\\",       SDL_SCANCODE_BACKSLASH},

        {"capslock",    "Cps.Lck.", SDL_SCANCODE_CAPSLOCK},
        {"a",           "a",        SDL_SCANCODE_A},
        {"s",           "s",        SDL_SCANCODE_S},
        {"d",           "d",        SDL_SCANCODE_D},
        {"f",           "f",        SDL_SCANCODE_F},
        {"g",           "g",        SDL_SCANCODE_G},
        {"h",           "h",        SDL_SCANCODE_H},
        {"j",           "j",        SDL_SCANCODE_J},
        {"k",           "k",        SDL_SCANCODE_K},
        {"l",           "l",        SDL_SCANCODE_L},
        {"semicolon",   ";",        SDL_SCANCODE_SEMICOLON},
        {"quote",       "'",        SDL_SCANCODE_APOSTROPHE},
        {"enter",       "Enter",    SDL_SCANCODE_RETURN},

        {"lshift",      "LShift",   SDL_SCANCODE_LSHIFT},
        {"z",           "z",        SDL_SCANCODE_Z},
        {"x",           "x",        SDL_SCANCODE_X},
        {"c",           "c",        SDL_SCANCODE_C},
        {"v",           "v",        SDL_SCANCODE_V},
        {"b",           "b",        SDL_SCANCODE_B},
        {"n",           "n",        SDL_SCANCODE_N},
        {"m",           "m",        SDL_SCANCODE_M},
        {"comma",       ",",        SDL_SCANCODE_COMMA},
        {"period",      ".",        SDL_SCANCODE_PERIOD},
        {"slash",       "/",        SDL_SCANCODE_SLASH},
        {"rshift",      "RShift",   SDL_SCANCODE_RSHIFT},

        {"lctrl",       "LCtrl",    SDL_SCANCODE_LCTRL},
        {"lalt",        "LAlt",     SDL_SCANCODE_LALT},
        {"space",       " ",        SDL_SCANCODE_SPACE},
        {"ralt",        "RAlt",     SDL_SCANCODE_RALT},
        {"rctrl",       "RCtrl",    SDL_SCANCODE_RCTRL},

        {"printscreen", "Prt.Scn.", SDL_SCANCODE_PRINTSCREEN},
        {"scrolllock",  "Scr.Lck.", SDL_SCANCODE_SCROLLLOCK},
        {"pause",       "Pause",    SDL_SCANCODE_PAUSE},
        {"insert",      "Ins",      SDL_SCANCODE_INSERT},
        {"home",        "Home",     SDL_SCANCODE_HOME},
        {"pageup",      "Pg.Up.",   SDL_SCANCODE_PAGEUP},
        {"delete",      "Del.",     SDL_SCANCODE_DELETE},
        {"end",         "End",      SDL_SCANCODE_END},
        {"pagedown",    "Pg.Dw.",   SDL_SCANCODE_PAGEDOWN},

        {"up",          "Up",       SDL_SCANCODE_UP},
        {"left",        "Left",     SDL_SCANCODE_LEFT},
        {"down",        "Down",     SDL_SCANCODE_DOWN},
        {"right",       "Right",    SDL_SCANCODE_RIGHT},

        {"numlock",     "Num.Lck.", SDL_SCANCODE_NUMLOCKCLEAR},
        {"kp_divide",   "KP/",      SDL_SCANCODE_KP_DIVIDE},
        {"kp_multiply", "KP*",      SDL_SCANCODE_KP_MULTIPLY},
        {"kp_minus",    "KP-",      SDL_SCANCODE_KP_MINUS},
        {"kp_7",        "KP7",      SDL_SCANCODE_KP_7},
        {"kp_8",        "KP8",      SDL_SCANCODE_KP_8},
        {"kp_9",        "KP9",      SDL_SCANCODE_KP_9},
        {"kp_plus",     "KP+",      SDL_SCANCODE_KP_PLUS},
        {"kp_4",        "KP4",      SDL_SCANCODE_KP_4},
        {"kp_5",        "KP5",      SDL_SCANCODE_KP_5},
        {"kp_6",        "KP6",      SDL_SCANCODE_KP_6},
        {"kp_1",        "KP1",      SDL_SCANCODE_KP_1},
        {"kp_2",        "KP2",      SDL_SCANCODE_KP_2},
        {"kp_3",        "KP3",      SDL_SCANCODE_KP_3},
        {"kp_enter",    "KP.Enter", SDL_SCANCODE_KP_ENTER},
        {"kp_0",        "KP0",      SDL_SCANCODE_KP_0},
        {"kp_period",   "KP.",      SDL_SCANCODE_KP_PERIOD},

        {0,             0,          SDL_SCANCODE_UNKNOWN}};

// Definition of a key on the virtual keyboard
class KeyboardKey {
private:
    // Code of the key
    char *key;
    // Selection rectangle (relative to keyboard position)
    Bit16u x, y, x2, y2;

public:
    KeyboardKey(char *_key, Bit16u _x, Bit16u _y, Bit16u _x2, Bit16u _y2) {
        key = _key;
        x = _x;
        y = _y;
        x2 = _x2;
        y2 = _y2;
    }

    // Check if x,y is in rectangle
    bool IsInside(Bit16s _x, Bit16s _y) {
        return (_x >= x) && (_x <= x2) && (_y >= y) && (_y <= y2);
    }

    // Returns key name
    char *GetKeyName() {
        return key;
    }

    // Returns rectangle delimiting key
    SDL_Rect *GetRect() {
        SDL_Rect *rect = new SDL_Rect();
        rect->x = x;
        rect->y = y;
        rect->w = x2 - x + 1;
        rect->h = y2 - y + 1;
        return rect;
    }
};

// List and iterator of keyboard keys
typedef std::list<KeyboardKey *> KeyboardKeyList;
typedef std::list<KeyboardKey *>::iterator KeyboardKeyList_it;

// The buffer of keys virtually pressed
#define BUFFER_SIZE 250

class VirtualKeyboardBuffer {
private:
    struct KeyBuffer {
        // The key code
        SDL_Scancode key;
        // true=key is pressed, false=key is released
        bool pressed;
    } buffer[BUFFER_SIZE];
    // Current size of the buffer
    int pos;

public:
    // Reset the content of the buffer
    void Clear() {
        pos = 0;
    }

    // Return if buffer is empty or not
    bool IsEmpty() {
        return pos == 0;
    }

    // Check if enough space for 'nb' keys
    bool HasEnoughSpace(int nb = 1) {
        return (pos + nb <= BUFFER_SIZE);
    }

    // Add a key to the buffer
    void AddKey(SDL_Scancode key, bool pressed) {
        if (pos >= BUFFER_SIZE)
            return;
        buffer[pos].key = key;
        buffer[pos].pressed = pressed;
        pos++;
    }

    // Remove the first key from the buffer and send it to the emulated keyboard
    void PurgeNext() {
        if (pos > 0) {
            SDL_Event sdlevent = {};
            if (buffer[0].pressed) {
                sdlevent.type = SDL_KEYDOWN;
            } else {
                sdlevent.type = SDL_KEYUP;
            }
            sdlevent.key.keysym.sym = buffer[0].key;
            SDL_PushEvent(&sdlevent);

            //KEYBOARD_AddKey(buffer[0].key, buffer[0].pressed);
            for (int i = 1; i < pos; i++) {
                buffer[i - 1].key = buffer[i].key;
                buffer[i - 1].pressed = buffer[i].pressed;
            }
            pos--;
        }
    }

    // Check if that key is currently pressed or released
    bool IsKeyPressed(SDL_Scancode key) {
        for (int i = pos - 1; i >= 0; i--)
            if (buffer[i].key == key)
                return buffer[i].pressed;
        return false;
    }
};

// The directory containing the virtual keyboard configuration
static std::string vkeybd_dir;
// The surface of the virtual keyboard image
static SDL_Surface *vkeybd_surface;
// The rectangle delimiting the virtual keyboard display
static SDL_Rect display_rect;
// The list of keys
static KeyboardKeyList keyList;
// The current format of virtual keyboard (320 or 640)
static int vkeybd_format;

// Information on screen
#if SDL_VERSION_ATLEAST(2, 0, 0)
static SDL_Window *screen_window;
#endif
static int screen_width, screen_height;
static float screen_ratio;
static int screen_offset_x, screen_offset_y;
// Information on drawing surface
static bool draw_init;
static Bit8u *draw_pixels;
static Bitu draw_pitch;
static SDL_Surface *draw_surface;
// The rectangle delimiting the virtual keyboard
static SDL_Rect vkeybd_rect;
// The position of the cursor in virtual keyboard display
static Bit16u cursor_x, cursor_y;
// The "speed" of the mouse cursor when emulated from joystick
static int joystick_offset;
// The buffer
static VirtualKeyboardBuffer buffer;
// Needed colors
static Bitu color_black, color_white, color_grey;
// The moment of the last flush to keyboard
static Bit32u keyboardFlushTime;

// Blit pixels directly in memory
void
GFX_Blit(Bit8u *src_pixels, Bitu src_pitch, SDL_Rect *src_rect, Bit8u *dst_pixels, Bitu dst_pitch, SDL_Rect *dst_rect,
         Bitu bpp) {
    const Bit8u *src = src_pixels + src_rect->y * src_pitch + src_rect->x * bpp;
    Bit8u *dst = dst_pixels + dst_rect->y * dst_pitch + dst_rect->x * bpp;
    for (int i = 0; i < src_rect->h; i++) {
        memcpy(dst, src, src_rect->w * bpp);
        src += src_pitch;
        dst += dst_pitch;
    }
}

// Lock the screen and copy content in draw_surface
void GFX_StartUpdate() {
    //bool s = GFX_StartUpdate(draw_pixels, draw_pitch);
    // Only at virtual keyboard open (background will not change because in pause)
    if (!draw_init) {
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = draw_surface->w;
        rect.h = draw_surface->h;
        GFX_Blit(draw_pixels, draw_pitch, &rect, (Bit8u *) draw_surface->pixels, draw_surface->pitch, &rect,
                 draw_surface->format->BytesPerPixel);
        draw_init = true;
    }
}

// Update screen using the content of draw_surface and request a refresh
void GFX_EndUpdate() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = draw_surface->w;
    rect.h = draw_surface->h;
    GFX_Blit((Bit8u *) draw_surface->pixels, draw_surface->pitch, &rect, draw_pixels, draw_pitch, &rect,
             draw_surface->format->BytesPerPixel);
    Bit16u changedLines[] = {0, 0}; // TODO: To improve
    changedLines[1] = draw_surface->h;
    //GFX_EndUpdate(changedLines);
    draw_pixels = NULL;
    draw_pitch = 0;
}

// Remove all spaces/tab/... of a line in the keyboard definition file
static void PackLine(char *str_src, char *str_dest) {
    while (*str_src) {
        if (!isspace(*str_src)) {
            *str_dest = *str_src;
            str_dest++;
        }
        str_src++;
    }
    *str_dest = 0;
}

// Read a line of the keyboard definition file and returns the corresponding KeyboardKey object
static KeyboardKey *DefDecode(char *line) {
    char *p = strchr(line, '=');
    if (p == NULL)
        return NULL;
    *p = 0;
    char *c[4];
    int i = 0;
    c[0] = p + 1;
    while (*++p) {
        if (*p == ',') {
            *p = 0;
            if (++i < 4)
                c[i] = p + 1;
        }
    }
    if (i != 3)
        return NULL;
    char *key = strdup(line);
    Bit16u x = atoi(c[0]);
    Bit16u y = atoi(c[1]);
    Bit16u x2 = atoi(c[2]);
    Bit16u y2 = atoi(c[3]);
    return new KeyboardKey(key, x, y, x2, y2);
}

// Read the keyboard definition file and initialize list of keyboard keys
#define MAX_LENGTH_LINE 100

static void DefRead(const char *def_path) {
    char line[MAX_LENGTH_LINE], work[MAX_LENGTH_LINE];
    FILE *def_file = fopen(def_path, "r");
    if (def_file == NULL) {
       fprintf( stderr,"VKEYBD: Cannot load virtual keyboard, definition file not found: %s", def_path);
        return;
    }
    while (fgets(line, MAX_LENGTH_LINE, def_file) != NULL) {
        PackLine(line, work);
        if (*work)
            if (*work != '#') {
                KeyboardKey *kk = DefDecode(work);
                if (kk == NULL)
                   fprintf( stderr,"VKEYBD: Bad content in virtual keyboard definition file, line ignored %s", line);
                else {
                    if (!strcmp(kk->GetKeyName(), KEY_DISPLAY)) {
                        SDL_Rect *rect = kk->GetRect();
                        display_rect.x = rect->x;
                        display_rect.y = rect->y;
                        display_rect.w = rect->w;
                        display_rect.h = rect->h;
                        delete kk;
                        delete rect;
                    } else
                        keyList.push_back(kk);
                }
            }
    }
    fclose(def_file);
}

// Search key name in the mapping table
static KeyboardKeyMap *SearchKeyName(char *keyName) {
    Bitu i = 0;
    while (vkeybd_map[i].name) {
        if (!strcmp(vkeybd_map[i].name, keyName))
            return &vkeybd_map[i];
        i++;
    }
    return NULL;
}

// Add a dark shadow on key to simulate pressed
static void DrawKeyPressed(KeyboardKey *key) {
    SDL_Rect *rect = key->GetRect();
    rect->x += vkeybd_rect.x;
    rect->y += vkeybd_rect.y;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Bit32u amask = 0x000000ff;
#else
    Bit32u amask = 0xff000000;
#endif
    SDL_Surface *alpha_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, rect->w, rect->h,
                                                      draw_surface->format->BitsPerPixel,
                                                      draw_surface->format->Rmask, draw_surface->format->Gmask,
                                                      draw_surface->format->Bmask,
                                                      amask);
    SDL_FillRect(alpha_surface, NULL, SDL_MapRGBA(alpha_surface->format, 0, 0, 0, 150));
    SDL_BlitSurface(alpha_surface, NULL, draw_surface, rect);
    SDL_FreeSurface(alpha_surface);
    delete rect;
}

// Redraw the initial key to remove the dark shadow and simulate key released
static void DrawKeyReleased(KeyboardKey *key) {
    SDL_Rect *rect_src = key->GetRect();
    SDL_Rect *rect_dest = key->GetRect();
    rect_dest->x += vkeybd_rect.x;
    rect_dest->y += vkeybd_rect.y;
    SDL_BlitSurface(vkeybd_surface, rect_src, draw_surface, rect_dest);
    delete rect_src;
    delete rect_dest;
}

// Draw a text on screen (font size=14)
extern Bit8u int10font14[256 * 14];

static void DrawText(Bit16u x, Bit16u y, const char *text, Bit32u color) {
    Bit32u *draw = (Bit32u *) (((Bit8u *) draw_surface->pixels) + ((y) * draw_surface->pitch)) + x;
    while (*text) {
        Bit8u *font = &int10font14[(*text) * 14];
        Bitu i, j;
        Bit32u *draw_line = draw;
        for (i = 0; i < 14; i++) {
            Bit8u map = *font++;
            for (j = 0; j < 8; j++) {
                if (map & 0x80)
                    *((Bit32u *) (draw_line + j)) = color;
                map <<= 1;
            }
            draw_line += draw_surface->pitch / 4;
        }
        text++;
        draw += 8;
    }
}

// Draw text cursor in specified color (black=display it, white=remove it)
static void DrawCursor(Bit16u x, Bit16u y, Bit32u color) {
    Bit32u *draw = (Bit32u *) (((Bit8u *) draw_surface->pixels) + ((y) * draw_surface->pitch)) + x;
    for (Bitu i = 0; i < 14; i++) {
        *((Bit32u *) (draw)) = color;
        draw += draw_surface->pitch / 4;
    }
}

// Display text in display, at current cursor
static void DisplayText(const char *text, Bit32u color) {
    Bit16u text_w = strlen(text) * 8;
    // Text is too large for display, must scroll
    if (cursor_x + text_w >= display_rect.w + vkeybd_rect.x) {
        SDL_Rect src_rect, dst_rect;
        src_rect.x = display_rect.x + display_rect.w / 2 + vkeybd_rect.x;
        src_rect.y = display_rect.y + vkeybd_rect.y;
        src_rect.w = display_rect.w / 2;
        src_rect.h = display_rect.h;
        dst_rect.x = display_rect.x + vkeybd_rect.x;
        dst_rect.y = display_rect.y + vkeybd_rect.y;
        SDL_BlitSurface(draw_surface, &src_rect, draw_surface, &dst_rect);
        dst_rect.x = src_rect.x;
        dst_rect.y = src_rect.y;
        src_rect.x = display_rect.x + display_rect.w / 2;
        src_rect.y = display_rect.y;
        src_rect.w = display_rect.w / 2;
        src_rect.h = display_rect.h;
        SDL_BlitSurface(vkeybd_surface, &src_rect, draw_surface, &dst_rect);
        cursor_x -= display_rect.w / 2;
    }
    // Draw text
    DrawText(cursor_x, cursor_y, text, color);
    // Move cursor
    cursor_x += text_w;
}

// A click has been done (mouse or joystick): detect corresponding key and do action
static bool HandleVirtualKeyPressed(Bit16u x, Bit16u y, bool altButton) {
    // Compute position relative to virtual keyboard
    Bit16s relx = (x - screen_offset_x) / screen_ratio - vkeybd_rect.x;
    Bit16s rely = (y - screen_offset_y) / screen_ratio - vkeybd_rect.y;
    // Find key pressed
    for (KeyboardKeyList_it it = keyList.begin(); it != keyList.end(); it++) {
        if ((*it)->IsInside(relx, rely)) {
            char *keyName = (*it)->GetKeyName();

            if (!strcmp(keyName, KEY_APPLY)) {
                // APPLY button => keys will be sent later
                return true;
            } else if (!strcmp(keyName, KEY_CANCEL)) {
                // CANCEL button => reset buffer and exit
                buffer.Clear();
                return true;
            } else {
                // It is a keyboard key => check corresponding character and code
                KeyboardKeyMap *keyMap = SearchKeyName(keyName);
                if (keyMap != NULL) {
                    // Lock screen for update
                    GFX_StartUpdate();

                    const char *op = OPEN_DEFAULT;
                    const char *cl = CLOSE_DEFAULT;
                    // Check in buffer if key is currently hold
                    bool pressed = buffer.IsKeyPressed(keyMap->key);
                    if (!altButton) {
                        // Standard button: press and release button
                        // If button is currently hold, simply release it
                        if (!pressed) {
                            if (!buffer.HasEnoughSpace(2))
                                return false;
                            buffer.AddKey(keyMap->key, true);
                        } else {
                            if (!buffer.HasEnoughSpace())
                                return false;
                            DrawKeyReleased(*it);
                            op = OPEN_RELEASED;
                            cl = CLOSE_RELEASED;
                        }
                        buffer.AddKey(keyMap->key, false);
                    } else {
                        // Alternate button: press or release button depending on current status
                        if (!buffer.HasEnoughSpace())
                            return false;
                        pressed = !pressed;
                        buffer.AddKey(keyMap->key, pressed);
                        if (pressed) {
                            DrawKeyPressed(*it);
                            op = OPEN_PRESSED;
                            cl = CLOSE_PRESSED;
                        } else {
                            DrawKeyReleased(*it);
                            op = OPEN_RELEASED;
                            cl = CLOSE_RELEASED;
                        }
                    }

                    // Clear cursor (if currently displayed)
                    DrawCursor(cursor_x, cursor_y, color_white);
                    // Draw text corresponding to key on display
                    DisplayText(op, color_grey);
                    DisplayText(keyMap->disp_name, color_black);
                    DisplayText(cl, color_grey);

                    // Refresh screen
                    GFX_EndUpdate();
                }
            }
            break;
        }
    }
    return false;
}

// Wait SDL events and process them
static void PollEvents() {
    bool vkeybd_exit = false;                 // Exiting flag
    Bit32u lastJoystickTime = SDL_GetTicks(); // Last refresh of joystick position
    Bit32u lastCursorTime = SDL_GetTicks();   // Last refresh of cursor
    bool showCursor = false;                  // Initial status of cursor is not visible
    Bit32s xrel = 0, yrel = 0;                // Relative movement of joystick

    // While exit is not requested ...
    SDL_Event event;
    while (!vkeybd_exit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    // Quit requested => stop application
                    exit(1);
                    break;
                case SDL_MOUSEBUTTONUP:
                    // Mouse button pressed => check if a key has been pressed
                    vkeybd_exit = HandleVirtualKeyPressed(event.button.x, event.button.y,
                                                          event.button.button != SDL_BUTTON_LEFT);
                    break;
                case SDL_JOYBUTTONUP:
                    // Joystick button pressed => idem
                    if ((event.jbutton.which == 0) && (event.jbutton.button <= 1)) {
                        // Use mouse cursor position
                        Bit32s x, y;
                        SDL_GetMouseState(&x, &y);
                        vkeybd_exit = HandleVirtualKeyPressed(x, y, event.jbutton.button != 0);
                    }
                    break;
                case SDL_JOYAXISMOTION:
                    // Joystick axis has been moved => compute relative movement (apply it later)
                    if (event.jaxis.which == 0) {
                        float delta = event.jaxis.value * joystick_offset / 32767.0;
                        if (event.jaxis.axis == 0)
                            xrel = delta;
                        else
                            yrel = delta;
                    }
                    break;
            }
        }

        // Process joystick move (if delayed exceeded)
        Bit32u currJoystickTime = SDL_GetTicks();
        if (currJoystickTime >= lastJoystickTime + JOYSTICK_DELAY) {
            if ((xrel != 0) || (yrel != 0)) {
                Bit32s x, y;
                // Get mouse position
                SDL_GetMouseState(&x, &y);
                // Apply relative movement
                x += xrel * screen_ratio;
                y += yrel * screen_ratio;
                // Avoid exiting screen
                if (x < 0)
                    x = 0;
                if (x > screen_width - 1)
                    x = screen_width - 1;
                if (y < 0)
                    y = 0;
                if (y > screen_height - 1)
                    y = screen_height - 1;
                // Display mouse to new position
#if SDL_VERSION_ATLEAST(2, 0, 0)
                SDL_WarpMouseInWindow(screen_window, x, y);
#else
                SDL_WarpMouse(x, y);
#endif
                lastJoystickTime = currJoystickTime;
            }
        }

        // Process cursor blinking (if delayed exceeded)
        Bit32u currCursorTime = SDL_GetTicks();
        if (currCursorTime >= lastCursorTime + CURSOR_DELAY) {
            // Lock screen for update
            GFX_StartUpdate();
            // Toggle cursor state and display new state
            DrawCursor(cursor_x, cursor_y, (showCursor) ? color_black : color_white);
            showCursor = !showCursor;
            lastCursorTime = currCursorTime;
            // Refresh screen
            GFX_EndUpdate();
        }

        SDL_Delay(1);
    }
}

// Initialize virtual keyboard: load image, read definition file
static void init(int draw_width) {
    // Choose format of virtual keyboard to display
    int new_format = 640;
    if (draw_width < 640)
        new_format = 320;
    // If already loaded with same format, nothing to do
    if (vkeybd_format == new_format)
        return;
        // If already loaded with different format, reload
    else if ((vkeybd_format != 0) && (vkeybd_format != new_format)) {
        SDL_FreeSurface(vkeybd_surface);
        keyList.clear();
    }

    // Load virtual keyoard image
    char path[100];
    sprintf(path, "%svkeybd_%d.bmp", vkeybd_dir.c_str(), new_format);
    vkeybd_surface = SDL_LoadBMP(path);
    if (vkeybd_surface == NULL) {
       fprintf( stderr,"VKEYBD: Cannot load virtual keyboard, image file not found: %s", path);
        return;
    }

    // Set green color to transparent
    SDL_SetColorKey(vkeybd_surface,
#if SDL_VERSION_ATLEAST(2, 0, 0)
                    SDL_TRUE,
#else
            SDL_SRCCOLORKEY,
#endif
                    SDL_MapRGB(vkeybd_surface->format, 0, 255, 0));

    // Read keyboard definition file
    sprintf(path, "%svkeybd_%d.def", vkeybd_dir.c_str(), new_format);
    DefRead(path);

    // Store loaded format
    vkeybd_format = new_format;
   fprintf( stderr,"VKEYBD: Virtual keyboard successfully loaded");
}

// Initialize virtual keyboard properties
void VKEYBD_Init() {
    // Get virtual keyboard directory
    // 1) use -vkeybd parameter
    // 2) use DosBox configuration directory
    vkeybd_dir.assign(readdir(opendir(""))->d_name);
    // Add directory separator (if not)
}

// Display virtual keyboard and process events
#if SDL_VERSION_ATLEAST(2, 0, 0)

void VKEYBD_Run(SDL_Window *window, SDL_PixelFormat *pixelFormat) {
#else
    void VKEYBD_Run(SDL_Surface *surface, SDL_PixelFormat *pixelFormat)
    {
#endif
    //GFX_EndUpdate(0); // Be sure that there is no update in progress

    // Get information on screen and draw zone
#if SDL_VERSION_ATLEAST(2, 0, 0)
    screen_window = window;
    SDL_GetWindowSize(screen_window, &screen_width, &screen_height);
#else
    screen_width = surface->w;
    screen_height = surface->h;
#endif
    int draw_width, draw_height;
    bool draw_fullscreen;
    //GFX_GetSize(draw_width, draw_height, draw_fullscreen);

    // Initialize virtual keyboard
    init(screen_width);
    if (vkeybd_surface == NULL)
        return;

    // Create surface with current screen content
    draw_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, screen_width, screen_height, pixelFormat->BitsPerPixel,
                                        pixelFormat->Rmask, pixelFormat->Gmask, pixelFormat->Bmask, pixelFormat->Amask);
    draw_init = false;
    // Compute ratio between screen and draw dimensions and offsets
    float screen_ratio_x = 1;
    float screen_ratio_y = 1;
    screen_ratio = (screen_ratio_x < screen_ratio_y) ? screen_ratio_x : screen_ratio_y;
    screen_offset_x = 1 / 2;
    screen_offset_y = 1 / 2;
    // Compute joystick "speed"
    joystick_offset = screen_width / JOYSTICK_OFFSET_RATIO;
    // Lock screen for update
    GFX_StartUpdate();

    // Clear keyboard buffer and release any keys pressed
    //KEYBOARD_ClrBuffer();
    //GFX_LosingFocus();

    // Initialize used colors
    color_black = SDL_MapRGB(draw_surface->format, 0x00, 0x00, 0x00);
    color_white = SDL_MapRGB(draw_surface->format, 0xff, 0xff, 0xff);
    color_grey = SDL_MapRGB(draw_surface->format, 0x8f, 0x8f, 0x8f);

    // Compute position of the virtual keyboard (centered in x, bottom in y)
    vkeybd_rect.x = (screen_width - vkeybd_surface->w) / 2;
    vkeybd_rect.y = screen_height - vkeybd_surface->h;
    vkeybd_rect.w = vkeybd_surface->w;
    vkeybd_rect.h = vkeybd_surface->h;
    // Computing starting and max position of the cursor
    cursor_x = display_rect.x + vkeybd_rect.x;
    cursor_y = display_rect.y + display_rect.h / 2 - 7 + +vkeybd_rect.y; // 7 = half the font height
   fprintf( stderr,"VKEYBD: Show virtual keyboard with dimensions %dx%d at %d,%d", vkeybd_rect.w, vkeybd_rect.h, vkeybd_rect.x,
           vkeybd_rect.y);
    fflush(stdout);

    // Backup the screen region to modify, current mouse cursor and joystick events
    SDL_Surface *bck_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, vkeybd_rect.w, vkeybd_rect.h,
                                                    pixelFormat->BitsPerPixel,
                                                    pixelFormat->Rmask, pixelFormat->Gmask, pixelFormat->Bmask,
                                                    pixelFormat->Amask);
    SDL_BlitSurface(draw_surface, &vkeybd_rect, bck_surface, NULL);
    int cursor_backup = SDL_ShowCursor(SDL_QUERY);
    int joystick_backup = SDL_JoystickEventState(SDL_QUERY);

    // Display virtual keyboard and mouse cursor, enable joystick events
    SDL_BlitSurface(vkeybd_surface, NULL, draw_surface, &vkeybd_rect);
    SDL_ShowCursor(SDL_ENABLE);
    //bool mousetoggle = mouselocked;
    //if (mouselocked)
    //GFX_CaptureMouse();
    SDL_JoystickEventState(SDL_ENABLE);
    // Refresh screen
    GFX_EndUpdate();

    // Clear content of buffer
    buffer.Clear();
    // Process events until exit requested
    PollEvents();

    // Restore mouse cursor, joystick events and screen
    SDL_ShowCursor(cursor_backup);
    //if (mousetoggle)
    //GFX_CaptureMouse();
    SDL_JoystickEventState(joystick_backup);
    // Lock screen for update
    GFX_StartUpdate();
    // Reset content of screen with backup
    SDL_BlitSurface(bck_surface, NULL, draw_surface, &vkeybd_rect);
    // Refresh screen
    GFX_EndUpdate();
    // Free allocated surfaces
    SDL_FreeSurface(bck_surface);
    SDL_FreeSurface(draw_surface);
    //GFX_ResetScreen();
}

// If there is at least a pending key in buffer, send it to keyboard (enough time from last send)
bool VKEYBD_PurgeNext() {
    Bit32u currKeyboardFlushTime = SDL_GetTicks();
    bool empty = buffer.IsEmpty();
    if (!empty && (currKeyboardFlushTime >= keyboardFlushTime + KEYBOARD_FLUSH_DELAY)) {
        buffer.PurgeNext();
        keyboardFlushTime = currKeyboardFlushTime;
    }
    return !empty;
}
