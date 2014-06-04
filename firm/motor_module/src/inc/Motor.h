// pins

#define PIN_M_EN		PD7
#define PIN_M_1A		PD5
#define PIN_M_2A		PD6

/* PID controller data */
typedef struct {
	float epsilon;
	float dt;
	float Kp, Kd, Ki;

	float integral;
	float pre_error;
} PidData;

typedef enum {
	POSITION,
	VELOCITY
} MotorMode;

typedef struct {
	MotorMode mode;

	volatile uint32_t pos;
	volatile double vel;

	uint32_t target_pos;
	double target_vel;

	PidData pid;
} Motor;

extern Motor motor;

void motor_init(void);

void motor_stop(void);
void motor_brake(void);

void motor_set_pos(uint32_t pos);
void motor_set_vel(int32_t vel);

void pid_reset();
float pid_calc(PidData* prefs, float current, float target);
