#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cmath>
#include "Arduino.h"

struct Environment {
    float temp;
    float rh;
};

class CellarSimulator {
public:
    CellarSimulator() :
        inside_({10.0f, 80.0f}),
        outside_({15.0f, 60.0f}),
        fan_on_(false),
        ozone_on_(false),
        last_update_ms_(0) {}

    void Update(unsigned long current_ms) {
        if (last_update_ms_ == 0) {
            last_update_ms_ = current_ms;
            return;
        }

        float dt_hours = (float)(current_ms - last_update_ms_) / (1000.0f * 3600.0f);

        float virtual_hour = (float)(current_ms / (1000.0f * 3600.0f));
        outside_.temp = 15.0f + 10.0f * sinf((virtual_hour - 8.0f) * M_PI / 12.0f);
        outside_.rh = 70.0f - 20.0f * sinf((virtual_hour - 8.0f) * M_PI / 12.0f);

        float k_inertia = 0.01f;
        inside_.temp += (8.0f - inside_.temp) * k_inertia * dt_hours;

        if (fan_on_) {
            float k_fan = 2.0f;
            inside_.temp += (outside_.temp - inside_.temp) * k_fan * dt_hours;
            inside_.rh += (outside_.rh - inside_.rh) * k_fan * dt_hours;
        } else {
            inside_.rh += (90.0f - inside_.rh) * 0.05f * dt_hours;
        }

        if (inside_.rh > 100.0f) inside_.rh = 100.0f;
        if (inside_.rh < 20.0f) inside_.rh = 20.0f;

        last_update_ms_ = current_ms;
    }

    void SetFan(bool on) { fan_on_ = on; }
    void SetOzone(bool on) { ozone_on_ = on; }

    Environment GetInside() const { return inside_; }
    Environment GetOutside() const { return outside_; }

private:
    Environment inside_;
    Environment outside_;
    bool fan_on_;
    bool ozone_on_;
    unsigned long last_update_ms_;
};

extern CellarSimulator g_simulator;

#endif
