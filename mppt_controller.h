#include "esphome.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

// using namespace std;

class mppt_controller : public Component, public UARTDevice
{

public:
    mppt_controller(UARTComponent *parent) : UARTDevice(parent) {}

    Sensor *power_sensor = new Sensor();
    Sensor *cwu_temperature_sensor = new Sensor();
    Sensor *energy_total_sensor = new Sensor();
    Sensor *enabled_sensor = new Sensor();

    Sensor *voltage_sensor = new Sensor();
    Sensor *current_sensor = new Sensor();
    Sensor *max_power_sensor = new Sensor();
    Sensor *max_temperature_sensor = new Sensor();
    Sensor *low_voltage_detector_sensor = new Sensor();
    Sensor *current_unit_temperature_sensor = new Sensor();
    Sensor *current_pwm_value_sensor = new Sensor();
  
    std::string current = "";
    std::vector<std::string> values;
    float prev_cwu = 0;

    void loop() override
    {
        while (available() > 0)
        {
            char c = read();
            if(c == '\r') continue;

            current += c;

            if(c == '\n' || c == '\r')
            {
                // ESP_LOGD("custom", "Received all data: %s", current.c_str());

                if(current.size() > 64)
                {
                    ESP_LOGD("custom", "To big size %i. Clearing...", current.size());
                    current.clear();
                    continue;
                }

                std::stringstream ss(current);
                std::string item;
                while (getline(ss, item, ';')) {
                    // ESP_LOGD("custom", "Item: %s", item.c_str());
                    values.push_back(item);
                }

                // ESP_LOGD("custom", "Values size: %i", values.size());

                if(!values.empty() && values[0] == "AA" && values.size() >= 5)
                {
                    // power_sensor->publish_state(customStof(values[1]));
                    
                    float cwu_temp = customStof(values[2], 2) / 10;
                    if((cwu_temp < 100 && cwu_temp > 0) && (prev_cwu != 0 && abs(cwu_temp - prev_cwu) <= 5))
                    {
                        cwu_temperature_sensor->publish_state(cwu_temp);
                    }

                    prev_cwu = cwu_temp;
                    
                    energy_total_sensor->publish_state(customStof(values[3], 3));

                    int enabled = std::stoi(values[4]);
                    if(enabled >= 1)
                    {
                        enabled_sensor->publish_state(1);
                    }
                    else
                    {
                        enabled_sensor->publish_state(0);
                    }
                    
                }
                else if(!values.empty() && values[0] == "SR" && values.size() >= 8)
                {
                    voltage_sensor->publish_state(std::stoi(values[1]));
                    current_sensor->publish_state(customStof(values[2], 1) / 10);
                    float currentPower = customStof(values[3], 1);
                    if(currentPower < 4000)
                    {
                        power_sensor->publish_state(currentPower);
                    }                    
                    max_power_sensor->publish_state(customStof(values[4], 1));
                    max_temperature_sensor->publish_state(customStof(values[5], 2) / 10);
                    low_voltage_detector_sensor->publish_state(std::stoi(values[6]));
                    std:int pwmVal = std::stoi(values[7]);
                    if(pwmVal < 256)
                    {
                        current_pwm_value_sensor->publish_state(pwmVal);
                    }
                    
                    current_unit_temperature_sensor->publish_state(customStof(values[8], 2) / 10);
                }

                current.clear();
                values.clear();
            } 
        }
    }

    float customStof(std::string s, int precision) {
        replace(s.begin(), s.end(), ',', '.'); // replace comma with period
        size_t pos = s.find('.');
        if (pos != std::string::npos && s.size() > pos + precision + 1) {
            return std::stof(s.substr(0, pos + precision + 1));
        }
        return std::stof(s);
    }
 };
