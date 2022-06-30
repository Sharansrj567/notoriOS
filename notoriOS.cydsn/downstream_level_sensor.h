#include "level_sensor.h"
#include "project.h"
#include "testing.h"


//ISR: Interrupt Service Routine. The macro CY_ISR_PROTO declares an ISR function prototype.
CY_ISR_PROTO(Downstream_Level_Sensor_ISR);

level_sensor_t downstream_level_sensor_take_reading();
test_t downstream_level_sensor_test();