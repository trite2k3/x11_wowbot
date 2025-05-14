// record_path.cpp         —  g++ -std=c++17 -O2 record_path.cpp -lX11 -o record_path

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>          // NEW
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <vector>

struct Point { float x, y; };

// -----------------------------------------------------------
//  CONFIG – change these two if your bot uses different boxes
// -----------------------------------------------------------
int baseX = 71, baseY = 151;          // top‑left pixel of your “info bar”
// -----------------------------------------------------------

static volatile sig_atomic_t g_stop = 0;
void handle_sigint(int) { g_stop = 1; }

bool get_rgb(Display* d, int x, int y, int& r, int& g, int& b)
{
    Window root = DefaultRootWindow(d);
    XImage* img = XGetImage(d, root, x, y, 1, 1, AllPlanes, ZPixmap);
    if (!img) return false;
    unsigned long p = XGetPixel(img, 0, 0);
    r = (p >> 16) & 0xFF;
    g = (p >> 8)  & 0xFF;
    b =  p        & 0xFF;
    XDestroyImage(img);
    return true;
}

float decode_component(int val) { return (float(val) / 255.0f) * 100.0f; }

int main()
{
    std::signal(SIGINT, handle_sigint);          // Ctrl‑C = stop & dump
    Display* d = XOpenDisplay(nullptr);
    if (!d) { std::cerr << "Cannot open X display\n"; return 1; }

    // --- set up key we want to watch -------------------------------------
    const int keycode9 = XKeysymToKeycode(d, XStringToKeysym("9"));
    if (keycode9 == 0) {
        std::cerr << "Couldn’t resolve keycode for key ‘9’\n";
        return 1;
    }

    std::vector<Point> path;
    std::cout << "Recording path…  Walk in‑game.\n"
                 "Press the ‘9’ key to add a waypoint, Ctrl‑C to finish.\n";

    bool prevPressed = false;                    // edge‑detection flag

    while (!g_stop) {
        // poll current keyboard state
        char keys[32]{};
        XQueryKeymap(d, keys);
        bool pressed = keys[keycode9 / 8] & (1 << (keycode9 % 8));

        // rising edge?  (just went from not‑pressed → pressed)
        if (pressed && !prevPressed) {
            int r=0,g=0,b=0;
            if (!get_rgb(d, baseX,         baseY, r,g,b)) break;
            float xCoord = decode_component(r);
            if (!get_rgb(d, baseX + 17,    baseY, r,g,b)) break;
            float yCoord = decode_component(g);

            path.push_back({xCoord, yCoord});
            std::cout << "  sample " << path.size()
                      << "  (" << xCoord << ", " << yCoord << ")\n";
        }
        prevPressed = pressed;
        usleep(20'000);              // 20 ms poll interval – light on CPU
    }

    XCloseDisplay(d);

    // ---- dump C++ literal -------------------------------------------------
    std::cout << "\nstd::vector<Point> path = {\n";
    for (const auto& p : path)
        std::cout << "    {" << p.x << ", " << p.y << "},\n";
    std::cout << "};\n";
    return 0;
}
