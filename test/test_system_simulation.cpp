#include <unity.h>
#include "Arduino.h"
#include "Simulator.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "TimeManager.h"
#include "Controller.h"
#include "DisplayUI.h"
#include <chrono>
#include <thread>

// External multiplier from mocks
extern unsigned long g_millis_multiplier;

void setUp(void) {
    g_millis_multiplier = 225;
}

void tearDown(void) {}

void test_system_long_run(void) {
    SensorManager sensor_manager;
    RelayManager relay_manager;
    TimeManager time_manager;
    Controller controller(&sensor_manager, &relay_manager, &time_manager);
    DisplayUI ui(&controller, &sensor_manager, &time_manager);

    controller.SetUI(&ui);

    relay_manager.Init();
    sensor_manager.Init();
    time_manager.Init();
    ui.Init();

    // Seed initial valid data to avoid immediate errors
    g_simulator.Update(millis());
    sensor_manager.Update();

    controller.Init();

    unsigned long last_log_ms = 0;
    unsigned long start_ms = millis();
    unsigned long duration_ms = 48UL * 3600UL * 1000UL;

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\n--- STARTING 48-HOUR SIMULATION (225x Speed) ---\n");
    printf("Time(v) | State    | Err | T_in | H_in | T_out| H_out| Fan | Ozone\n");
    printf("------------------------------------------------------------------\n");

    while (millis() - start_ms < duration_ms) {
        unsigned long current_millis = millis();

        g_simulator.Update(current_millis);

        // System update logic (same as main.cpp)
        static unsigned long last_sensor_update = 0;
        if (current_millis - last_sensor_update >= kSensorPollInterval) {
            last_sensor_update = current_millis;
            sensor_manager.Update();
        }

        static unsigned long last_time_update = 0;
        if (current_millis - last_time_update >= 1000) {
            last_time_update = current_millis;
            time_manager.Update();
        }

        controller.Tick();
        ui.Update();

        // Logging every 1s real time (~4 virtual minutes)
        static auto last_real_ms = std::chrono::steady_clock::now();
        auto now_real = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now_real - last_real_ms).count() >= 1000) {
            last_real_ms = now_real;

            DateTime now_v = time_manager.GetTime();
            printf("%02d:%02d:%02d | %-8s | %-4s | %5.2f | %5.2f | %5.2f | %5.2f | %-3s | %-3s\n",
                now_v.hour(), now_v.minute(), now_v.second(),
                StateToString(controller.GetState()),
                ErrorToString(controller.GetError()),
                sensor_manager.GetInside().temp,
                sensor_manager.GetInside().rh,
                sensor_manager.GetOutside().temp,
                sensor_manager.GetOutside().rh,
                relay_manager.GetFanState() ? "ON" : "OFF",
                relay_manager.GetOzoneState() ? "ON" : "OFF"
            );
        }

        // Give some air to the host OS
        std::this_thread::yield();
    }

    printf("--- SIMULATION FINISHED ---\n");
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_system_long_run);
    return UNITY_END();
}
