#include "raylib.h"

#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/file.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <signal.h>
#include <pthread.h>

typedef uint16_t u16;
typedef uint8_t u8;

#define MAX_SAMPLES 512
#define MAX_SAMPLES_PER_UPDATE 4092
#define NUM_SENSORS 5

#define PX 1
#define CM 1
#define PX_PER_CM (200.0*PX/10.0*CM)

#define WINDOW_SIZE 5

float frequency = 440.0f;
float audioFrequency = 440.0f;
float sineIdx = 0.0f;

double clamp(double val, double lo, double hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

bool close_enough(double a, double b) {
    return fabs(a - b) < DBL_EPSILON;
}

void audio_input_callback(void *buffer, unsigned int frames) {
    audioFrequency = frequency + (audioFrequency - frequency ) * 0.95f;
    
    float incr = audioFrequency/44100.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
        {
            d[i] = (short)(32000.0f*sinf(2*PI*sineIdx));
            sineIdx += incr;
            if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }    
}



static uint16_t raw_adc = 0;
static int serial = 0;

bool RUN = true;

void exit_cli(int sig) {
	RUN = false;
	printf("\nclosing down ... ");
}

void *read_serial(void * _) {
    while (RUN) {
        read(serial, &raw_adc, 2);
    }

    return NULL;
}

typedef struct Moving_Average {
    u16 sum;
    u16 average;
    u16 readings[WINDOW_SIZE];
    u16 index;
} Moving_Average;

typedef struct Hall_Effect {
    u8 port;

    // these four are measured in adc bits
    
    // these values were collected by measuring the sensor with no magnetic field
    // and with the magnet of choice (not telling) touching the sensor
    u16 operational_max_adc;
    u16 operational_min_adc;

    // these values should be measured when calibrating the keyboard
    // the min should be greater than the operational min
    // the max should be less than the operational max
    u16 max_adc;
    u16 min_adc;

    // measured in mm
    // these are just the numbers from the magic function in halleffect.c at the max_adc and min_adc
    float max_distance;
    float min_distance;

    bool parameter_changed;

    Moving_Average ma;
} Hall_Effect;

float halleffect_distance_curve(u8 port, float index) {
    // magic numbers. oooooooooooh. aaaaaaaaaaaaah
    switch (port) {
    case 0:
        return 45.958904f * powf(index, -0.44967207f);
    case 1:
        return 45.426605f * powf(index, -0.4443363f);
    case 2:
        return 45.75797f * powf(index, -0.4476087f);
    case 3:
        return 45.3737f * powf(index, -0.44436312f);
    case 4:
        return 46.21221f * powf(index, -0.4514614f);
    default:
        // this should not execute. just here to make the compiler happy
        return 0.0f;
    }
}

Hall_Effect halleffect_make(u8 port, u16 op_min_adc, u16 op_max_adc, u16 min_adc, u16 max_adc) {
    return (Hall_Effect) {
        .port = port,
        .operational_min_adc = op_min_adc,
        .operational_max_adc = op_max_adc,
        .min_adc = min_adc,
        .max_adc = max_adc,
        .max_distance = halleffect_distance_curve(port, 1.0f),
        .min_distance = halleffect_distance_curve(port, max_adc - min_adc + 1),
        .parameter_changed = false,

        // haha. initialization matters.
        // I know this is automatically initialized to zero when a struct is created like this
        // but now it is certainly initialized
        // without a doubt
        // definitely
        .ma = {0}, 
    };
}

u16 movingaverage_process(Moving_Average *ma, u16 raw_adc) {
    ma->sum -= ma->readings[ma->index];
    ma->readings[ma->index] = raw_adc;
    ma->sum += raw_adc;
    ma->index = (ma->index+1) % WINDOW_SIZE;
    ma->average = ma->sum / WINDOW_SIZE;

    return ma->average;
}

float halleffect_get_value(Hall_Effect *sensor, u16 raw_adc) {
    /* if (raw_adc < sensor->min_adc) { */
    /*     sensor->min_adc = raw_adc; */
    /*     sensor->parameter_changed = true; */
    /* } */

    /* if (raw_adc > sensor->max_adc) { */
    /*     sensor->max_adc = raw_adc; */
    /*     sensor->parameter_changed = true; */
    /* } */

    /* if (sensor->parameter_changed) { */
    /*     sensor->min_distance = halleffect_distance_curve(sensor->port, sensor->max_adc - sensor->min_adc + 1); */
    /* } */

    u16 averaged_adc = movingaverage_process(&sensor->ma, raw_adc);
    
    // less than this means sensor is not calibrated or sensor jitter
    // it should always be greater than operational_min_adc
    // so we don't need to worry about leaving the function range
    if (averaged_adc < sensor->min_adc)
        averaged_adc = sensor->min_adc;

    // higher than this and the functions stop working...
    if (averaged_adc > sensor->operational_max_adc)
        averaged_adc = sensor->operational_max_adc;
    
    float index = (float)(averaged_adc - sensor->min_adc + 1);
    float offset = 3.0f;

    /* if (averaged_adc >= sensor->max_adc) { */
    /*     index = sensor->max_adc - sensor->min_adc + 1; */
    /* } */

    // We want the number to go up, not down, so subtract distance from sensor from max distance
    return sensor->max_distance - halleffect_distance_curve(sensor->port, index + offset);
}

typedef struct Key_Hammer {
    float key_pos;
    float key_pos_prev;
    float key_velocity;
    float key_strike_distance;
    
    float hammer_pos;
    float hammer_velocity;
    float hammer_travel;
    
    float gravity;
    bool hammer_is_striking;
    bool key_is_striking;
    bool note_on_sent;
    bool note_off_sent;
} Key_Hammer;

Key_Hammer keyhammer_make(float travel) {
    // velocity is measured in mm/s
    // position is measured in mm
    return (Key_Hammer) {
        .key_pos = 0.0f,
        .key_pos_prev = 0.0f,
        .key_velocity = 0.0f,
        .hammer_pos = 0.0f,
        .hammer_velocity = 0.0f,
        .hammer_travel = travel + 0.01f * travel,
        .key_strike_distance = travel - 2.0f,
        .gravity = 9806.65f,
        .hammer_is_striking = false,
        .key_is_striking = false,
        .note_on_sent = false,
        .note_off_sent = false,
    };
}

void keyhammer_update(Key_Hammer* kh, float pos, float dt) {
    kh->key_pos_prev = kh->key_pos;
    kh->key_pos = pos;
    kh->key_velocity = 1.1*(kh->key_pos - kh->key_pos_prev) / dt;

    float original_speed = kh->hammer_velocity;
    kh->hammer_velocity -= kh->gravity * dt;
    kh->hammer_pos += (original_speed + kh->hammer_velocity) * dt / 2;

    /* printf("%f\n", kh->hammer_velocity); */

    if (kh->hammer_pos < kh->key_pos) {
        kh->hammer_pos = kh->key_pos;
        if (kh->hammer_velocity < kh->key_velocity) {
            kh->hammer_velocity = kh->key_velocity;
        }
    }

    kh->key_is_striking = kh->key_pos < 5.2f;
    kh->hammer_is_striking = kh->hammer_pos > kh->hammer_travel;

    if (kh->hammer_is_striking) {
        /* printf("%f\n", kh->hammer_pos); */
        kh->hammer_velocity = -kh->hammer_velocity;
        kh->hammer_pos = kh->hammer_travel;
    }
}

void key_and_hammer(void) {
    InitWindow(1000, 1000, "Analog");
    /**/
    struct termios oldtio, newtio;
	struct serial_struct ser_info;
    
    serial = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);

    if (serial < 0) {
        printf("Can't open serial port\n");
        goto cleanup;
    }

    if (tcgetattr(serial, &oldtio) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        goto cleanup;
    }

	/* clear struct for new port settings */
	bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 10;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;     /* blocking read until n character arrives */

    tcflush(serial, TCIFLUSH);
	if (tcsetattr(serial, TCSANOW, &newtio) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        goto cleanup;
    }

    ioctl(serial, TIOCGSERIAL, &ser_info);
	ser_info.flags |= ASYNC_LOW_LATENCY;
	ioctl(serial, TIOCSSERIAL, &ser_info);
    /**/

    pthread_t serial_thread;
    RUN = true;
    pthread_create(&serial_thread, NULL, read_serial, NULL);
    signal(SIGINT, exit_cli);
	signal(SIGTERM, exit_cli);
    
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
    AudioStream stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(stream, audio_input_callback);
    short *data = (short *)malloc(sizeof(short)*MAX_SAMPLES);
    short *writeBuf = (short *)malloc(sizeof(short)*MAX_SAMPLES_PER_UPDATE);

    int gamepad = 0;
    int axis = 5;

    float baseline = 650.0f;
    Rectangle rham = { 405, baseline, 10, 16 };

    int targetFPS = 72;
    double prev_time = GetTime();
    double curr_time = 0.0;
    double update_time = 0.0;
    double wait_time = 0.0;
    double delta_time = 0.0;

    int port = 0;

    Hall_Effect sensors[NUM_SENSORS] = {
        // don't change the second and third arguments please
        halleffect_make(0, 532, 879, 546, 875),
        halleffect_make(1, 518, 878, 533, 875),
        halleffect_make(2, 527, 879, 541, 649),
        halleffect_make(3, 518, 877, 537, 870),
        halleffect_make(4, 536, 880, 548, 730),
    };

    Key_Hammer keyhammers[NUM_SENSORS] = {
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[1].max_distance - sensors[1].min_distance),
        keyhammer_make(sensors[2].max_distance - sensors[2].min_distance),
        keyhammer_make(sensors[3].max_distance - sensors[3].min_distance),
        keyhammer_make(sensors[4].max_distance - sensors[4].min_distance),
    };

    Key_Hammer h = keyhammers[port];
    Hall_Effect sensor = sensors[port];

    while (!WindowShouldClose()) {
        PollInputEvents();
        /* double trigger = (GetGamepadAxisMovement(gamepad, axis) + 1) / 2.0 * 10.0f; */


        float current_adc = (float)raw_adc;

        float position_mm = (float)halleffect_get_value(&sensor, current_adc);

        if (sensor.parameter_changed) {
            h.hammer_travel = sensor.max_distance - sensor.min_distance;
            sensor.parameter_changed = false;
        }
            
        keyhammer_update(&h, position_mm, delta_time);


        rham.y = baseline - h.hammer_pos;
        /* print_key_hammer(&h); */
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            DrawRectanglePro(rham, (Vector2){ 0, rham.height / 2 }, 0.0f, BLACK);
            /* DrawText(TextFormat("h.pos: %f\nh.key.pos: %f\nsensor: %f\nrham.y: %f\nvelocity: %f\n", h.hammer_pos, h.key_pos, trigger, rham.y, h.key_velocity), 0, 0, 20, BLACK); */
            DrawText(TextFormat("ADC: %f\nTrigger: %f\nTravel: %f\n", current_adc, position_mm, h.hammer_travel), 0, 0, 20, BLACK);
            if (h.hammer_is_striking) {
                ClearBackground(GREEN);
                /* printf("VELOCITY: %f\n", h.speed); */
            }
        }
        EndDrawing();

        SwapScreenBuffer();

        curr_time = GetTime();
        update_time = curr_time - prev_time;

        wait_time = (1.0f / (float)targetFPS) - update_time;
        if (wait_time > 0.0) {
            WaitTime((float)wait_time);
            curr_time = GetTime();
            delta_time = (float)(curr_time - prev_time);
        }

        prev_time = curr_time;
    }

  cleanup:

    free(data);
    free(writeBuf);

    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();
    
    close(serial);
    tcsetattr(serial, TCSANOW, &oldtio);
}

int main(void) {
    key_and_hammer();
}
