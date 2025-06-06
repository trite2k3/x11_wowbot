#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <unistd.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <X11/extensions/XInput.h>
#include <cstdlib>
#include <iomanip>

using namespace std;
using namespace std::chrono;

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

static TimePoint steerPauseUntil = Clock::now();

TimePoint lastQ         = Clock::now() - std::chrono::minutes(16);
TimePoint lastE         = Clock::now() - std::chrono::minutes(11);
TimePoint lastClearTime = Clock::now() - std::chrono::seconds(6);
TimePoint lastHealTime  = Clock::now() - std::chrono::seconds(16);
TimePoint lastInnervate = Clock::now() - std::chrono::minutes(7);
TimePoint lastSeal = Clock::now() - std::chrono::seconds(25);

bool isTurningLeft = false;
bool isTurningRight = false;
bool isMovingForward = false;
bool isHealing = false;
bool hasTarget = false;

const int Y_BOX_OFFSET = 17;          // boxAggro: baseX+90, baseY
const int COMBAT_BOX_OFFSET = 33;
const int HP_BOX_OFFSET = 50;
const int TARGET_BOX_OFFSET = 67;
/*
const int CAT_BOX_OFFSET = 84;
*/
const int MANA_BOX_OFFSET = 84;
const int FACING_BOX_OFFSET = 118;
const int POSITION_BOX_OFFSET = 135;
const int AGGRO_BOX_OFFSET = 152;


// first color box coords
int baseX = 71, baseY = 151;

struct RGB {
    int r, g, b;
};

float decode_component(int val) {
    return ((float)val / 255.0f) * 100.0f;
}

// define a path
struct Point {
    float x, y;
};

/* PATH loch modan
std::vector<Point> path = {
    {71.3726, 39.2157},
    {71.3726, 40.3922},
    {72.1569, 42.3529},
    {73.3333, 43.1373},
    {72.9412, 44.7059},
    {74.1176, 45.4902},
    {74.5098, 44.3137},
    {76.0784, 44.3137},
    {76.4706, 43.1373},
    {76.0784, 41.5686},
    {78.0392, 41.9608},
    {77.6471, 40},
    {76.8627, 38.4314},
    {76.4706, 37.6471},
    {76.8627, 36.0784},
    {78.0392, 36.4706},
    {77.2549, 34.902},
    {77.6471, 33.7255},
    {76.4706, 34.5098},
    {75.6863, 36.0784},
    {76.0784, 38.4314},
    {74.1176, 37.6471},
    {74.902, 36.4706},
    {74.1176, 34.5098},
    {72.9412, 36.0784},
    {72.549, 36.0784},
    {71.7647, 36.8627},
    {71.3726, 37.6471},
    {72.549, 38.0392},
    {71.3726, 40.3922},
    {72.1569, 41.1765},
    {71.7647, 41.9608},
    {72.9412, 42.7451},
    {72.9412, 40.7843},
};

*/

/* Marsh
std::vector<Point> path = {
    {20.7843, 20.3922},
    {21.5686, 20},
    {22.3529, 21.1765},
    {25.098, 20.7843},
    {27.0588, 19.2157},
    {29.0196, 17.6471},
    {30.1961, 16.8627},
    {31.3726, 16.0784},
    {32.549, 15.6863},
    {32.9412, 18.4314},
    {32.1569, 20},
    {30.5882, 19.2157},
    {29.8039, 18.8235},
    {29.0196, 20.7843},
    {25.8824, 22.7451},
    {24.7059, 23.5294},
    {23.1373, 23.9216},
    {22.3529, 24.3137},
    {21.5686, 25.8824},
    {20.3922, 23.9216},
    {18.8235, 24.7059},
    {17.2549, 23.9216},
};

*/

/* arathi

std::vector<Point> path = {
    {41.1765, 70.5882},
    {40.7843, 69.4118},
    {41.1765, 67.8431},
    {40.7843, 66.6667},
    {41.5686, 65.098},
    {40.3922, 64.7059},
    {39.2157, 64.7059},
    {38.0392, 66.2745},
    {38.4314, 67.451},
    {38.8235, 68.6275},
    {38.4314, 69.8039},
    {38.8235, 70.1961},
    {39.2157, 70.9804},
    {40, 70.9804},
    {40.7843, 70.9804},
    {40.7843, 69.8039},
    {40, 68.2353},
    {39.2157, 66.2745},
    {39.6078, 65.098},
    {40.3922, 66.6667},
    {40.3922, 67.8431},

};
*/

/*
 stv monke

std::vector<Point> path = {
    {36.4706, 16.4706},
    {36.4706, 16.8627},
    {36.8627, 16.4706},
    {37.2549, 18.0392},
    {37.2549, 18.4314},
    {37.2549, 18.8235},
    {37.6471, 18.8235},
    {37.6471, 18.4314},
    {38.0392, 18.8235},
    {38.8235, 18.4314},
    {39.2157, 17.6471},
    {38.8235, 17.6471},
    {38.8235, 17.2549},
    {38.8235, 16.8627},
    {38.8235, 16.0784},
    {38.4314, 15.2941},
    {38.0392, 15.6863},
    {37.6471, 16.8627},
    {37.2549, 16.0784},
    {36.8627, 16.4706},
};
*/

/* stv tigers

std::vector<Point> path = {
    {36.4706, 34.902},
    {36.0784, 36.0784},
    {35.2941, 36.0784},
    {34.902, 36.8627},
    {34.902, 37.2549},
    {34.5098, 37.6471},
    {34.902, 38.8235},
    {35.6863, 38.0392},
    {35.6863, 38.4314},
    {36.0784, 38.4314},
    {35.2941, 38.8235},
    {35.2941, 39.6078},
    {35.2941, 40},
    {35.6863, 40.7843},
    {35.6863, 41.1765},
    {35.6863, 41.9608},
    {35.2941, 43.5294},
    {35.6863, 43.5294},
    {35.6863, 43.5294},
    {36.0784, 43.9216},
    {36.0784, 43.5294},
    {36.4706, 43.9216},
    {36.8627, 43.9216},
    {37.2549, 44.3137},
    {37.2549, 43.1373},
    {37.6471, 42.7451},
    {37.6471, 41.5686},
    {37.6471, 40.3922},
    {37.6471, 40},
    {38.0392, 39.6078},
    {38.0392, 39.2157},
    {38.8235, 38.8235},
    {38.8235, 38.4314},
    {38.8235, 36.8627},
    {38.0392, 36.8627},
    {38.0392, 36.4706},
    {37.6471, 36.4706},
    {37.2549, 36.0784},
    {37.2549, 35.2941},
    {36.8627, 34.902},
    {36.4706, 34.902},
    {36.0784, 34.902},
};
*/

/*
 Feralas apes

std::vector<Point> path = {
    {56.4706, 59.2157},
    {56.4706, 60},
    {57.2549, 60},
    {58.0392, 60.3922},
    {57.2549, 61.1765},
    {58.0392, 60.7843},
    {58.4314, 60.3922},
    {58.8235, 61.9608},
    {58.8235, 60.3922},
    {59.2157, 60.3922},
    {59.6078, 60.7843},
    {59.6078, 60},
    {58.8235, 59.6078},
    {59.2157, 59.2157},
    {59.2157, 58.8235},
    {58.4314, 58.8235},
    {57.6471, 58.4314},
    {57.2549, 58.4314},
    {56.8627, 58.0392},
    {56.4706, 58.8235},
};
*/

/*
 STV Hyena


std::vector<Point> path = {
    {62.3529, 61.1765},
    {61.5686, 60.3922},
    {61.1765, 60.7843},
    {60.7843, 60.3922},
    {60.3922, 61.1765},
    {60, 60.3922},
    {59.6078, 60.3922},
    {58.4314, 61.5686},
    {57.6471, 60},
    {58.0392, 58.8235},
    {58.8235, 58.8235},
    {58.8235, 57.2549},
    {59.6078, 55.6863},
    {60.7843, 56.4706},
    {60.7843, 54.5098},
    {61.5686, 54.5098},
    {61.9608, 56.0784},
    {62.7451, 55.6863},
    {63.1373, 56.0784},
    {63.9216, 56.0784},
    {63.9216, 57.2549},
    {63.9216, 59.2157},
    {63.1373, 60.3922},
};
*/

/*
 Feralas Bears
*/
std::vector<Point> path = {
    {51.3726, 33.3333},
    {50.5882, 32.549},
    {50.1961, 31.7647},
    {50.5882, 30.9804},
    {50.1961, 29.8039},
    {50.5882, 29.4118},
    {50.5882, 29.0196},
    {50.5882, 28.2353},
    {50.1961, 27.8431},
    {50.1961, 27.8431},
    {49.8039, 28.2353},
    {50.1961, 29.0196},
    {49.4118, 28.2353},
    {49.8039, 29.8039},
    {49.4118, 30.9804},
    {49.4118, 31.3726},
    {50.1961, 32.1569},
    {49.8039, 31.7647},
    {49.0196, 32.1569},
    {49.4118, 33.3333},
    {49.8039, 34.1176},
    {50.1961, 32.9412},
    {50.5882, 32.549},
};

RGB get_pixel_color(Display* display, int x, int y) {
    Window root = DefaultRootWindow(display);
    XImage* image = XGetImage(display, root, x, y, 1, 1, AllPlanes, ZPixmap);
    if (!image) {
        std::cerr << "Failed to capture screen at (" << x << "," << y << ")\n";
        return {0, 0, 0};
    }

    unsigned long pixel = XGetPixel(image, 0, 0);
    int r = (pixel >> 16) & 0xFF;
    int g = (pixel >> 8) & 0xFF;
    int b = pixel & 0xFF;

    XDestroyImage(image);
    return {r, g, b};
}

// 0‑255  →  0‑100 (decode_component)  →  0‑1  →  0‑2π
inline float decode_heading_rad(int red)
{
    float pct   = decode_component(red) / 100.0f;   // 0‑1
    return pct * 2.0f * M_PI;                       // radians
}

inline bool box_is_green(const RGB& c) {     // G high, R/B low
    return c.g > 200 && c.r < 80 && c.b < 80;
}
inline bool box_is_red(const  RGB& c) {      // R high, G/B low
    return c.r > 200 && c.g < 80 && c.b < 80;
}

void press_key(Display* display, KeySym key, bool hold, int delayMs = 0) {
    KeyCode k = XKeysymToKeycode(display, key);

    if (hold) {
        XTestFakeKeyEvent(display, k, True, CurrentTime);
        XFlush(display);

        if (delayMs > 0) {
            usleep(delayMs * 1000);
            XTestFakeKeyEvent(display, k, False, CurrentTime);
            XFlush(display);
        }

    } else {
        XTestFakeKeyEvent(display, k, True, CurrentTime);
        usleep(delayMs * 1000);
        XTestFakeKeyEvent(display, k, False, CurrentTime);
        XFlush(display);
    }
}


float normalize_angle(float angle) {
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}


void release_key(Display* display, KeySym key, bool& flag, int delayMs = 0) {
    if (flag) {
        press_key(display, key, false, delayMs);
        flag = false;
    }
}


void press_key_if_needed(Display* display, KeySym key, bool& flag) {
    if (!flag) {
        press_key(display, key, true);
        flag = true;
    }
}

void release_all(Display* display) {
    release_key(display, XK_Left,  isTurningLeft, 0);
    release_key(display, XK_Right, isTurningRight, 0);
    release_key(display, XK_Up,    isMovingForward, 0);

    press_key(display, XK_Left,  false, 0);
    press_key(display, XK_Right, false, 0);
}

bool is_positioning_error(Display* display, int errorBoxX, int errorBoxY) {
    RGB errorBox = get_pixel_color(display, errorBoxX, errorBoxY);

    // Orange: too far → high red, medium green, low blue
    bool isTooFar =
        errorBox.r > 200 &&
        errorBox.g > 80 && errorBox.g < 180 &&
        errorBox.b < 80;

    // Magenta: not facing → high red and blue, low green
    bool isNotFacing =
        errorBox.r > 200 &&
        errorBox.g < 50 &&
        errorBox.b > 200;

    return isTooFar || isNotFacing;
}


void move_towards(Display* display,
                  float x, float y,
                  float tx, float ty,
                  float heading)
{
    float dx = tx - x;
    float dy = ty - y;

    float desired = std::atan2(dx, dy); // CW from north
    float delta   = normalize_angle(desired - heading);

    press_key_if_needed(display, XK_Up, isMovingForward);

    constexpr float FACING_AWAY_TOLERANCE = 0.24f; // ~17 degrees from exact opposite

    if (std::fabs(std::fabs(delta) - M_PI) < FACING_AWAY_TOLERANCE) {
        // Facing directly away from target — don't turn
        release_key(display, XK_Left, isTurningLeft);
        release_key(display, XK_Right, isTurningRight);
        return;
    }

    // Decide which direction we need to turn
    if (delta > 0) {
        // Need to turn RIGHT
        release_key(display, XK_Left, isTurningLeft);        // make sure LEFT is released
        press_key_if_needed(display, XK_Right, isTurningRight); // hold RIGHT if not held
    } else {
        // Need to turn LEFT
        release_key(display, XK_Right, isTurningRight);        // make sure RIGHT is released
        press_key_if_needed(display, XK_Left, isTurningLeft);  // hold LEFT if not held
    }

    std::cout << std::fixed << std::setprecision(3)
              << "hdg="  << heading
              << " tgt=" << desired
              << " δ="   << delta
              << " turn=" << (delta > 0 ? "RIGHT" : "LEFT")
              << '\n';
}


void press_combo(Display* display, KeySym modifier, KeySym key, int delayMs = 0) {
    KeyCode mod = XKeysymToKeycode(display, modifier);
    KeyCode k = XKeysymToKeycode(display, key);

    XTestFakeKeyEvent(display, mod, True, CurrentTime);
    XTestFakeKeyEvent(display, k, True, CurrentTime);
    XTestFakeKeyEvent(display, k, False, CurrentTime);
    XTestFakeKeyEvent(display, mod, False, CurrentTime);
    XFlush(display);
    if (delayMs > 0) usleep(delayMs * 1000);
}

void right_click(Display* display, int x, int y) {
    Window root = DefaultRootWindow(display);
    XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
    XTestFakeButtonEvent(display, 3, True, CurrentTime);
    XTestFakeButtonEvent(display, 3, False, CurrentTime);
    XFlush(display);
}

// Wait long enough for the Global‑Cool‑Down to finish
inline void waitForGCD() { usleep(1600 * 1000); }   // 1.6 s

/*
// Call every time you are about to decide on a form
inline RGB refreshCatBox(Display* d) {
    return get_pixel_color(d, baseX + CAT_BOX_OFFSET, baseY);
}
*/

/*
void ensure_form(Display* display, RGB catBox, bool wantCatForm) {
    bool inCatForm = catBox.b > 200;

    if (inCatForm != wantCatForm) {
        std::cout << (wantCatForm ? "Entering Cat Form...\n" : "Exiting Cat Form...\n");
        waitForGCD();
        press_combo(display, XK_Control_L, XK_3, 1000);
        waitForGCD();
    }
}
*/

void ensure_targeting_player(Display* display)
{
    for (int n = 0; n < 8; ++n) {                        // safety limit
        RGB aggro = get_pixel_color(display,
                                    baseX + AGGRO_BOX_OFFSET, baseY);
        if (box_is_red(aggro))                           // success
            return;

        press_key(display, XK_Tab, true, 60);            // quick <Tab>
        usleep(120'000);                                 // let UI update
    }
    std::cout << "Could not grab aggro after several TABs – carrying on\n";
}

void clear_target_if_dead(Display* display) {
    auto now = steady_clock::now();
    float secondsSinceLastClear = duration<float>(now - lastClearTime).count();

    if (secondsSinceLastClear >= 5.0f) {
        RGB targetBox = get_pixel_color(display, baseX + TARGET_BOX_OFFSET, baseY);
        bool hasTarget = targetBox.r > 200;
        bool isTargetDead = targetBox.g < 100;

        if (hasTarget && isTargetDead) {
            std::cout << "Target is dead — clearing with ESC...\n";
            press_key(display, XK_Escape, true, 100);
            lastClearTime = now;  // reset cooldown
        }
    }
}

void tryCastInnervate(Display* display) {
    static TimePoint lastInnervate = Clock::now() - std::chrono::minutes(7); // Initially off cooldown
    TimePoint now = Clock::now();

    float minutesSinceLastInnervate = std::chrono::duration<float>(now - lastInnervate).count();

    if (minutesSinceLastInnervate >= 7.0f) {
        std::cout << "Casting Innervate.\n";
        /*
        ensure_form(display, refreshCatBox(display), false);
        */
        waitForGCD();
        press_key(display, XK_R, true, 210);
        waitForGCD();
        /*
        ensure_form(display, refreshCatBox(display), true);
        */
        waitForGCD();
        lastInnervate = now;
    }
}

/*
void buff(Display* display, RGB catBox)
*/
void buff(Display* display)
{
    auto now = Clock::now();

    if (now - lastQ >= 15min) {            // Alt‑Q
        press_combo(display, XK_Alt_L, XK_Q);
        waitForGCD();
        lastQ = now;
    }

    /* now go back to Cat */
    RGB manaBox = get_pixel_color(display, baseX + MANA_BOX_OFFSET, baseY);
    float manaPercent = decode_component(manaBox.b);
    if (manaPercent < 15.0f) {
        tryCastInnervate(display);
    }
    /*
    ensure_form(display, refreshCatBox(display), true);
    */
}

void seal(Display* display) {
    auto now = steady_clock::now();
    float secondsSinceSeal = duration<float>(now - lastSeal).count();
    if (secondsSinceSeal >= 27.0f) {
        press_key(display, XK_1, true, 250);
        press_key(display, XK_3, true, 250);
        waitForGCD();
        lastSeal = now;  // cooldown reset
    }

}

/*
void heal_if_needed(Display* display, float hpPercent, RGB catBox) {
*/
void heal_if_needed(Display* display, float hpPercent) {
    auto now = steady_clock::now();
    float secondsSinceLastHeal = duration<float>(now - lastHealTime).count();

    if (hpPercent < 20.0) {
        waitForGCD();
        press_key(display, XK_F, true, 200);
        waitForGCD();
    }

    if (hpPercent < 60.0 && secondsSinceLastHeal >= 14.0f) {
        RGB manaBox = get_pixel_color(display, baseX + MANA_BOX_OFFSET, baseY);
        float manaPercent = decode_component(manaBox.b);

        if (manaPercent > 15.0f) {
            release_all(display);
            isHealing = true;                         // ← keep
            /*
            ensure_form(display, refreshCatBox(display), false);   // will now run
            */
            /* new spec different keybindings
            press_combo(display, XK_Shift_L, XK_F);   // Rejuvenation
            waitForGCD();
            sleep(2);
            press_combo(display, XK_Shift_L, XK_E);   // Regrowth
            */
            waitForGCD();
            press_key(display, XK_Q, true, 200);    // Holy Light
            waitForGCD();
            sleep(3);
            /*
            sleep(2);
            press_key(display, XK_E, true, 200);    // Regrowth
            waitForGCD();
            */

            isHealing = false;                        // **moved up**
            /*
            buff(display, refreshCatBox(display));    // buffs + re‑enter Cat Form
            */
            buff(display);    // buffs + re‑enter Cat Form
            lastHealTime = now;  // cooldown reset
        } else {
            std::cout << "Not enough mana to heal (" << manaPercent << "%)\n";
            tryCastInnervate(display);
        }
    }
}

/*
bool combat_routine(Display* display, int combatPixelX, int combatPixelY, RGB catBox) {
*/
bool combat_routine(Display* display, int combatPixelX, int combatPixelY) {
    int maxCombatDuration = 30;
    int attackCount = 0;

    for (int i = 0; i < maxCombatDuration; ++i) {
        RGB combatBox = get_pixel_color(display, combatPixelX, combatPixelY);
        bool stillInCombat = combatBox.r > 200;

        if (!stillInCombat) {
            usleep(400000);
            combatBox = get_pixel_color(display, combatPixelX, combatPixelY);
            if (combatBox.r <= 200) {
                std::cout << "Combat really ended.\n";
                return true;
            }
        }

        /*
        ensure_form(display, refreshCatBox(display), true);
        */

        seal(display);

        std::cout << "Attacking with 4...\n";
        press_key(display, XK_4, true, 200);
        usleep(200000);
        press_combo(display, XK_Shift_L, XK_space);
        usleep(200000);
        //press_key(display, XK_2, true, 200);
        //usleep(200000);

        ensure_targeting_player(display);

        if (is_positioning_error(display, baseX + POSITION_BOX_OFFSET, baseY)) {
            std::cout << "Positioning error detected (Too Far or Not Facing)!\n";
            press_key(display, XK_Down, true, 700);
        }

        attackCount++;

        /* no, new spec not needed
        if (attackCount >= 9) {
            std::cout << "Using Utilities\n";
            press_key(display, XK_F, true, 250);
            press_key(display, XK_Q, true, 210);
            attackCount = 0;
        }
        */

        RGB hpBox = get_pixel_color(display, baseX + HP_BOX_OFFSET, baseY);
        float hpPercent = decode_component(hpBox.b);
        /*
        RGB catBox = get_pixel_color(display, baseX + CAT_BOX_OFFSET, baseY);
        */
        /*
        heal_if_needed(display, hpPercent, catBox);
        */
        heal_if_needed(display, hpPercent);
        clear_target_if_dead(display);
    }

    std::cout << "Combat timeout.\n";
    return false;
}

void loot(Display* display, int screenWidth, int screenHeight) {
    std::cout << "Attempting to loot...\n";
    for (int i = 0; i < 5; ++i) {
        int x = screenWidth / 2 + (rand() % 150 - 70);
        int y = screenHeight / 2 - 10 + (rand() % 100 - 60);
        right_click(display, x, y);
        usleep(300000);
    }

    clear_target_if_dead(display);

    if (hasTarget) {
        std::cout << "Trying to clear target...\n";

        for (int attempt = 0; attempt < 3; ++attempt) {
            press_key(display, XK_Escape, true, 200);

            RGB targetBox = get_pixel_color(display, baseX + TARGET_BOX_OFFSET, baseY);
            bool stillHasTarget = targetBox.r > 200;

            if (!stillHasTarget) {
                std::cout << "Target cleared.\n";
                return;
            }
        }

        std::cout << "Failed to clear target after multiple attempts.\n";
    }
}


int main() {
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        cerr << "Unable to open X display.\n";
        return 1;
    }

    std::cout << "Starting in 3...\n";
    sleep(1);
    std::cout << "2...\n";
    sleep(1);
    std::cout << "1...\n";
    sleep(1);

    // const float WAYPOINT_RADIUS = 0.30f;
    const float WAYPOINT_RADIUS = 0.15f;
    while (true) {
        for (const auto& waypoint : path) {

            while (true) {
                RGB xBox = get_pixel_color(display, baseX, baseY);
                RGB yBox = get_pixel_color(display, baseX + Y_BOX_OFFSET, baseY);
                RGB combatBox = get_pixel_color(display, baseX + COMBAT_BOX_OFFSET, baseY);
                RGB hpBox = get_pixel_color(display, baseX + HP_BOX_OFFSET, baseY);
                RGB targetBox = get_pixel_color(display, baseX + TARGET_BOX_OFFSET, baseY);
                /*
                RGB catBox = get_pixel_color(display, baseX + CAT_BOX_OFFSET, baseY);
                */
                RGB facingBox = get_pixel_color(display, baseX + FACING_BOX_OFFSET, baseY);

                float xCoord = decode_component(xBox.r);
                float yCoord = decode_component(yBox.g);
                float hpPercent = decode_component(hpBox.b);
                bool inCombat = combatBox.r > 200;
                bool hasTarget = targetBox.r > 200;
                bool isTargetDead = targetBox.g < 100;

                float dx = fabs(xCoord - waypoint.x);
                float dy = fabs(yCoord - waypoint.y);

                std::cout << "Current: " << xCoord << "," << yCoord << " Target: " << waypoint.x << "," << waypoint.y << " HP: " << hpPercent << "% Combat: " << (inCombat ? "YES" : "NO") << std::endl;

                /*
                ensure_form(display, refreshCatBox(display), true);
                */
                clear_target_if_dead(display);

                auto now = std::chrono::steady_clock::now();

                if (inCombat) {
                        release_all(display);
                        std::cout << "Entering combat loop...\n";

                        if (is_positioning_error(display, baseX + 135, baseY)) {
                            std::cout << "Positioning error detected (Too Far or Not Facing)!\n";
                            press_key(display, XK_Down, true, 1000);
                            sleep(1);
                            press_key(display, XK_Down, true, 400);
                        }
                    /*
                    bool combatEnded = combat_routine(display, baseX + COMBAT_BOX_OFFSET, baseY, catBox);
                    */
                    bool combatEnded = combat_routine(display, baseX + COMBAT_BOX_OFFSET, baseY);
                    if (combatEnded) {
                        usleep(500000);
                        RGB confirmCombatBox = get_pixel_color(display, baseX + COMBAT_BOX_OFFSET, baseY);
                        if (confirmCombatBox.r <= 200) {
                            loot(display, 2560, 1440);
                        } else {
                            std::cout << "Skipping loot — still in combat.\n";
                        }
                    }
                    continue;
                }

                float dist2 = (xCoord - waypoint.x) * (xCoord - waypoint.x) +
                            (yCoord - waypoint.y) * (yCoord - waypoint.y);

                // meh close enough, move to next wp
                if (dist2 < WAYPOINT_RADIUS * WAYPOINT_RADIUS) {
                    release_all(display);
                    break;
                }

                float headingRad = decode_heading_rad(facingBox.r);
                move_towards(display,
                            xCoord, yCoord,
                            waypoint.x, waypoint.y,
                            headingRad);

                usleep(100000);
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}
