#include <iostream>
#include <cstdint>
#include <array>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

#define I2C_SDA_GPIO GPIO_NUM_8
#define I2C_SCL_GPIO GPIO_NUM_9

class HapticI2C {
    public:
        esp_err_t init();

        esp_err_t addDevice(uint8_t address, i2c_master_dev_handle_t* device_handle);
        i2c_master_bus_handle_t getBusHandle() const;

    private:
        i2c_master_bus_handle_t bus_handle_ = nullptr;
        bool initialized_ = false;
};

esp_err_t HapticI2C::init() {
    // Prevent multiple initializations
    if (initialized_) { return ESP_OK; }

    i2c_master_bus_config_t i2c_config{};

    i2c_config.i2c_port = I2C_NUM_0;
    i2c_config.sda_io_num = I2C_SDA_GPIO;
    i2c_config.scl_io_num = I2C_SCL_GPIO;
    i2c_config.clk_source = I2C_CLK_SRC_DEFAULT;    // Not sure which clock to use must research
    i2c_config.glitch_ignore_cnt = 7;               // said in documentation to normally be 7 idk
    i2c_config.flags.enable_internal_pullup = true; // don't know what this means either yet

    esp_err_t result = i2c_new_master_bus(&i2c_config, &bus_handle_);

    // if creating bus handle failed, return the error 
    if (result != ESP_OK) { return result; }

    // if succeeded, mark as initialized and return success
    initialized_ = true;
    return result;

}

esp_err_t HapticI2C::addDevice(uint8_t address, i2c_master_dev_handle_t* device_handle{
    i2c_device_config_t config{};

    config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    config.device_address = address;
    config.scl_speed_hz = 100000;

    return i2c_master_bus_add_device(bus_handle_, &config, device_handle);
}

i2c_master_bus_handle_t HapticI2C::getBusHandle() const {
    return bus_handle_;
}

class HapticDriver {
    public:
        HapticDriver() = default;

        esp_err_t init(i2c_master_dev_handle_t dev_handle);

    private:
        // should I make this a pointer
        i2c_master_dev_handle_t device_handle_ = nullptr;
};

esp_err_t HapticDriver::init(i2c_master_dev_handle_t dev_handle){
    if (dev_handle != nullptr) { return ESP_ERR_INVALID_ARG; }
    device_handle_ = dev_handle;
    return ESP_OK;
}

class HapticMux {
    public:
        HapticMux() = default;
        esp_err_t init();
    private:
        i2c_master_dev_handle_t device_handle_ = nullptr;
        uint8_t address = 0x70;
};

esp_err_t HapticMux::init(){
    i2c_master_bus_handle_t handle;
    i2c_master_get_bus_handle(0, &handle);
}

class HapticController {
    public:
        HapticController();

        esp_err_t init();

    private:
        std::array<HapticDriver, 5> drivers_{};
        i2c_master_dev_handle_t driver_handle_ = nullptr;
        bool initialized_ = false;
};

esp_err_t HapticController::init(){
    if (initialized_) { return ESP_OK; }

    i2c_master_bus_handle_t handle;
    i2c_master_get_bus_handle(0, &handle);

    // create device handle for drivers
    i2c_device_config_t device_config{};

    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = 0x5A;        // i2c address for drivers
    device_config.scl_speed_hz = 100000;        // can decide on speed upon further research

    // returns ESP_OK if success, and some error flag if not
    auto result = i2c_master_bus_add_device(handle, &device_config, &driver_handle_);

    for (HapticDriver& driver : drivers_){
        driver.init(driver_handle_);
    }
}

extern "C" void app_main(void)
{
    HapticI2C i2c;
    HapticController controller;

    i2c.init();
    controller.init();

    std::cout << "hello world";
}
