#ifndef _FILES_H_
#define _FILES_H_
// These are just for show, we'll always send a report on the 2nd sample
sensor_config_t f_sensor_config_lps = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 12000,
    .threshold_low = 9000,
};


sensor_config_t f_sensor_config_temp = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 9000,
    .threshold_low = 7000,
};

sensor_config_t f_sensor_config_light = {
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

sensor_config_t f_sensor_config_accel = {
    .report_type = REPORT_ON_DIFFERENCE,
    .read_period = 1000,
    .max_period = 3600,
    .max_diff = 100,
    .threshold_high = 500,
    .threshold_low = -500,
};

#endif
