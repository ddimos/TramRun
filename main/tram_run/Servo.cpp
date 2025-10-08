#include "tram_run/Servo.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

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

    class Servo
    {
    public:
        Servo();
        ~Servo();

        void rotate(int _angleDeg);

    private:
        mcpwm_timer_handle_t m_timer = NULL;
        mcpwm_oper_handle_t m_operator = NULL;
        mcpwm_cmpr_handle_t m_comparator = NULL;
        mcpwm_gen_handle_t m_generator = NULL;
    };

    Servo::Servo()
    {
        ESP_LOGI(TAG, "Create timer and operator");
        
        mcpwm_timer_config_t timer_config;
        timer_config.group_id = GroupId;
        timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
        timer_config.resolution_hz = ServoTimebaseResolutionHz;
        timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
        timer_config.period_ticks = ServoTimebasePeriod;
        timer_config.intr_priority = 0;

        ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &m_timer));

        mcpwm_operator_config_t operator_config;
        operator_config.group_id = GroupId;
        operator_config.intr_priority = 0;

        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &m_operator));

        ESP_LOGI(TAG, "Connect timer and operator");
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(m_operator, m_timer));

        ESP_LOGI(TAG, "Create comparator and generator from the operator");
        mcpwm_comparator_config_t comparator_config;
        comparator_config.intr_priority = 0;
        comparator_config.flags.update_cmp_on_tez = true;

        ESP_ERROR_CHECK(mcpwm_new_comparator(m_operator, &comparator_config, &m_comparator));

        mcpwm_generator_config_t generator_config;
        generator_config.gen_gpio_num = CONFIG_TR_SERVO_PULSE_GPIO;

        ESP_ERROR_CHECK(mcpwm_new_generator(m_operator, &generator_config, &m_generator));

        // set the initial compare value, so that the servo will spin to the center position
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(m_comparator, angleToCompare(0)));

        ESP_LOGI(TAG, "Set generator action on timer and compare event");
        // go high on counter empty
        ESP_ERROR_CHECK(
            mcpwm_generator_set_action_on_timer_event(m_generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH))
        );

        // go low on compare threshold
        ESP_ERROR_CHECK(
            mcpwm_generator_set_action_on_compare_event(m_generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, m_comparator, MCPWM_GEN_ACTION_LOW))
        );

        ESP_LOGI(TAG, "Enable and start timer");
        ESP_ERROR_CHECK(mcpwm_timer_enable(m_timer));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(m_timer, MCPWM_TIMER_START_NO_STOP));
    }

    Servo::~Servo()
    {
        // TODO to think how to properly deinit everyting
    }

    void Servo::rotate(int _angleDeg)
    {
        ESP_LOGI(TAG, "Angle of rotation: %d", _angleDeg);
        if (_angleDeg < ServoMinDegree)
            _angleDeg = ServoMinDegree;
        else if (_angleDeg > ServoMaxDegree)
            _angleDeg = ServoMaxDegree;
        
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(m_comparator, angleToCompare(_angleDeg)));
    }

    static QueueHandle_t g_queue = nullptr;
    static TaskHandle_t g_task = nullptr;
    
    void task(void* _pvParameter)
    {
        Servo servo;

        tr::servo::Event event;
        while (true)
        {
            if (xQueueReceive(g_queue, &event, portMAX_DELAY))
            {
                ESP_LOGI(TAG, "Rotate, angle: %d", event.desiredRotationDeg);
                servo.rotate(event.desiredRotationDeg);
            }
        }
    }

} // namespace

namespace tr::servo
{
    void init()
    {
        ESP_LOGI(TAG, "Init");
        // TODO assert
        if (g_queue != nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        g_queue = xQueueCreate(3, sizeof(Event));

        if (g_queue == nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        // TODO the stack size is higher that it could be because the initialization needs more memory
        BaseType_t ret = xTaskCreate(task, "ServoTask", 3062, NULL, 8, &g_task);
        if (ret != pdPASS)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }
    }

    void deinit()
    {
        if (g_queue == nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        vQueueDelete(g_queue);
        vTaskDelete(g_task);
    }

    void sendEvent(Event _event)
    {
        xQueueSend(g_queue, &_event, portMAX_DELAY);
    }

} // namespace tr::servo
