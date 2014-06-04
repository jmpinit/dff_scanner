// pins

#define PIN_M_EN		PD7
#define PIN_M_1A		PD5
#define PIN_M_2A		PD6

void motor_init(void);

void motor_enable(void);
void motor_disable(void);
void motor_brake(void);

void motor_move(int32_t pos);
void motor_set_speed(uint8_t speed);

long motor_get_pos(void);
