#include "tram_run/Servo.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

namespace
{
    static const char* TAG = "TR_SERVO";

    constexpr unsigned ServoMinPulsewidthUs = 500;  // Minimum pulse width in microsecond
    constexpr unsigned ServoMaxPulsewidthUs = 2500; // Maximum pulse width in microsecond
    constexpr int ServoMinDegree = -90;             // Minimum angle
    constexpr int ServoMaxDegree = 90;              // Maximum angle

    constexpr unsigned ServoTimebaseResolutionHz = 1000000; // 1MHz, 1us per tick
    constexpr unsigned ServoTimebasePeriod = 20000;         // 20000 ticks, 20ms

    constexpr int GroupId = 1;

    static inline uint32_t angleToCompare(int _angleDeg)
    {
        return (_angleDeg - ServoMinDegree) * (ServoMaxPulsewidthUs - ServoMinPulsewidthUs) / (ServoMaxDegree - ServoMinDegree) + ServoMinPulsewidthUs;
    }

    mcpwm_timer_handle_t g_timer = NULL;
    mcpwm_oper_handle_t g_operator = NULL;
    mcpwm_cmpr_handle_t g_comparator = NULL;
    mcpwm_gen_handle_t g_generator = NULL;
} // namespace


namespace tr
{
    Servo::Servo() = default;
    Servo::~Servo() = default;

    void Servo::init()
    {
        ESP_LOGI(TAG, "Create timer and operator");
        
        mcpwm_timer_config_t timer_config;
        timer_config.group_id = GroupId;
        timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
        timer_config.resolution_hz = ServoTimebaseResolutionHz;
        timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
        timer_config.period_ticks = ServoTimebasePeriod;
        timer_config.intr_priority = 0;

        ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &g_timer));

        mcpwm_operator_config_t operator_config;
        operator_config.group_id = GroupId;
        operator_config.intr_priority = 0;

        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &g_operator));

        ESP_LOGI(TAG, "Connect timer and operator");
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(g_operator, g_timer));

        ESP_LOGI(TAG, "Create comparator and generator from the operator");
        mcpwm_comparator_config_t comparator_config;
        comparator_config.intr_priority = 0;
        comparator_config.flags.update_cmp_on_tez = true;

        ESP_ERROR_CHECK(mcpwm_new_comparator(g_operator, &comparator_config, &g_comparator));

        mcpwm_generator_config_t generator_config;
        generator_config.gen_gpio_num = CONFIG_TR_SERVO_PULSE_GPIO;

        ESP_ERROR_CHECK(mcpwm_new_generator(g_operator, &generator_config, &g_generator));

        // set the initial compare value, so that the servo will spin to the center position
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(g_comparator, angleToCompare(0)));

        ESP_LOGI(TAG, "Set generator action on timer and compare event");
        // go high on counter empty
        ESP_ERROR_CHECK(
            mcpwm_generator_set_action_on_timer_event(g_generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH))
        );

        // go low on compare threshold
        ESP_ERROR_CHECK(
            mcpwm_generator_set_action_on_compare_event(g_generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, g_comparator, MCPWM_GEN_ACTION_LOW))
        );

        ESP_LOGI(TAG, "Enable and start timer");
        ESP_ERROR_CHECK(mcpwm_timer_enable(g_timer));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(g_timer, MCPWM_TIMER_START_NO_STOP));
    }

    void Servo::rotate(int _angleDeg)
    {
        ESP_LOGI(TAG, "Angle of rotation: %d", _angleDeg);
        if (_angleDeg < ServoMinDegree)
            _angleDeg = ServoMinDegree;
        else if (_angleDeg > ServoMaxDegree)
            _angleDeg = ServoMaxDegree;
        
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(g_comparator, angleToCompare(_angleDeg)));
    }
} // namespace tr
