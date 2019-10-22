#ifndef _FILES_H_
#define _FILES_H_
sensor_config_t f_sensor_config_lps = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 120000,
    .threshold_low = 90000,
};


sensor_config_t f_sensor_config_temp = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 9000,
    .threshold_low = 7000,
};


sensor_config_t f_sensor_config_hmc = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 500,
    .threshold_high = 1000,
    .threshold_low = -1000,
};

sensor_config_t f_sensor_config_lsm = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 500,
    .threshold_low = -500,
};

#endif
