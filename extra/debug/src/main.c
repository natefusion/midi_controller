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

#define MAX_SAMPLES 512
#define MAX_SAMPLES_PER_UPDATE 4092

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

int key = 1;
int min_adc = 558;
int max_adc = 751;
int offset = 3;

double halleffect_distance_curve(int port, double index) {
    // magic numbers. oooooooooooh. aaaaaaaaaaaaah
    switch (port) {
    case 0:
        return 45.958904f * pow(index, -0.44967207f);
    case 1:
        return 45.426605f * pow(index, -0.4443363f);
    case 2:
        return 45.75797f * pow(index, -0.4476087f);
    case 3:
        return 45.3737f * pow(index, -0.44436312f);
    case 4:
        return 46.21221f * pow(index, -0.4514614f);
    default:
        // this should not execute. just here to make the compiler happy
        return 0.0f;
    }
}

typedef struct Key_Hammer {
    float key_pos;
    float key_pos_prev;
    float key_velocity;
    
    float hammer_pos;
    float hammer_velocity;
    float hammer_travel;
    
    float gravity;
    bool hammer_is_striking;
    bool key_is_striking;
} Key_Hammer;

Key_Hammer keyhammer_make(void) {
    // velocity is measured in mm/s
    // position is measured in mm
    return (Key_Hammer) {
        .key_pos = 0.0f,
        .key_pos_prev = 0.0f,
        .key_velocity = 0.0f,
        .hammer_pos = 0.0f,
        .hammer_velocity = 0.0f,
        .hammer_travel = halleffect_distance_curve(key, 1) - halleffect_distance_curve(key, max_adc - min_adc + 1),
        .gravity = 9806.65,
        .hammer_is_striking = false,
        .key_is_striking = false,
    };
}

void keyhammer_update(Key_Hammer* kh, float pos, float dt) {
    kh->key_pos_prev = kh->key_pos;
    kh->key_pos = pos;
    kh->key_velocity = (kh->key_pos - kh->key_pos_prev) / dt;

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

    Key_Hammer h = keyhammer_make();

    double highest_value = halleffect_distance_curve(key, 1.0);

    while (!WindowShouldClose()) {
        PollInputEvents();
        /* double trigger = (GetGamepadAxisMovement(gamepad, axis) + 1) / 2.0 * 10.0f; */


        float current_adc = (float)raw_adc;

        if (current_adc < min_adc) {
            current_adc = min_adc;
        }

        /* printf("%f\n", current_adc); */

        double trigger = highest_value - halleffect_distance_curve(key, current_adc - min_adc + 1 + offset);
        
        keyhammer_update(&h, trigger, delta_time);

        rham.y = baseline - h.hammer_pos;
        /* print_key_hammer(&h); */
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            DrawRectanglePro(rham, (Vector2){ 0, rham.height / 2 }, 0.0f, BLACK);
            /* DrawText(TextFormat("h.pos: %f\nh.key.pos: %f\nsensor: %f\nrham.y: %f\nvelocity: %f\n", h.hammer_pos, h.key_pos, trigger, rham.y, h.key_velocity), 0, 0, 20, BLACK); */
            DrawText(TextFormat("ADC: %f\nTrigger: %f\nTravel: %f\n", current_adc, trigger, h.hammer_travel), 0, 0, 20, BLACK);
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
