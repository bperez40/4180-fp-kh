
#include "Thread.h"
#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"
#include "Motor.h"
#include "PinDetect.h"
#include <cstdio>

// Definitions for motor code
#define second 1000000
#define LEFT_TURN -1
#define RIGHT_TURN 1
#define DEFAULT 0

Motor rm(p21, p19, p20); // pwm, fwd, rev
Motor lm(p22, p23, p24); // pwm, fwd, rev

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
uLCD_4DGL uLCD(p9, p10, p11);  // uLCD

AnalogOut DACout(p18);

Serial blue(p13, p14);

wave_player waver(&DACout);

PinDetect lbump3(p28);
PinDetect lbump4(p29);
PinDetect lbump5(p30);

PinDetect rbump0(p25);
PinDetect rbump1(p26);
PinDetect rbump2(p27);

DigitalOut led1(LED1);
DigitalOut led2(LED2);

Timeout recover;
Timeout normalize;

volatile float drive;
volatile int correction;

Mutex uLCD_mutex;
FILE *wave_file;

// uLCD setup text animation
char* init_text_array[] = {(char *) "Initializing.", (char *) "Initializing..", (char *) "Initializing..."};

// array containing music file paths, titles, and lengths
char* song_list[] = {(char*) "/sd/song/1.wav", (char*) "/sd/song/2.wav", (char*) "/sd/song/3.wav"};
char* song_title[] = {(char*) "Animal Crossing Intro", (char*) "Generator", (char*) "God's Menu"};
int song_length[] = {69, 227, 187};
int num_songs = 3;

// progress bar variables
volatile int restart = 0;
volatile int stop = 0;

// index of current song to play from song_list
volatile int current_song = 256;

// used for enabling debug print statements
int verbosity = 0;

// switches back to default forward motion
void default_routine(void) {
    correction = DEFAULT;
    drive = 0.3;
}

// called when left bumper is triggered
// makes DJ roomba turn in the right direction
void right_recover_routine(void) {
    correction = RIGHT_TURN;
    drive = 0.2;
    normalize.attach_us(&default_routine, 0.5*second);
}

// called when right bumper is triggered
// makes DJ roomba turn in the left direction
void left_recover_routine(void) {
    correction = LEFT_TURN;
    drive = 0.2;
    normalize.attach_us(&default_routine, 0.5*second);
}

// callback functions for each bumper button :D
void lbump5_callback(void) {
    drive = -0.2;
    recover.attach_us(&right_recover_routine, 1*second);
}
void rbump0_callback(void) {
    drive = -0.2;
    recover.attach_us(&left_recover_routine, 1*second);
}
void lbump4_callback(void) {
    drive = -0.25;
    recover.attach_us(&right_recover_routine, 1*second);
}
void rbump1_callback(void) {
    drive = -0.25;
    recover.attach_us(&left_recover_routine, 1*second);
}
void lbump3_callback(void) {
    drive = -0.3;
    recover.attach_us(&right_recover_routine, 1*second);
}
void rbump2_callback(void) {
    drive = -0.3;
    recover.attach_us(&left_recover_routine, 1*second);
}

void draw_UI() {
    uLCD.cls();
    // Draw the outline for the progress bar and the player buttons
    uLCD.rectangle(10, 68, 120, 78, WHITE);
    uLCD.circle(25, 100, 15, WHITE);
    uLCD.circle(65, 100, 15, WHITE);
    uLCD.circle(105, 100, 15, WHITE);

    // Draw left triangle
    uLCD.triangle(15, 100, 30, 92, 30, 108, WHITE);
    // Draw right triangle
    uLCD.triangle(115, 100, 100, 92, 100, 108, WHITE);

    // Middle square
    uLCD.filled_rectangle(60, 95, 70, 105, RED);
}

void display_progress(void const *args) {
    while(1) {
        // reset retart variaible to 0
        restart = 0;
        
        // lock uLCD
        uLCD_mutex.lock();
        // clear the progress bar and update song title
        uLCD.filled_rectangle(11, 69, 119, 77, BLACK);

        // unlock uLCD
        uLCD_mutex.unlock();

        for (int t = 1; t <= 108; t++) {
            uLCD_mutex.lock();
            uLCD.filled_rectangle(11, 69, 11 + t, 77, GREEN);
            uLCD_mutex.unlock();

            // decrement t if stop flag is raised
            // essentially no progress in loop
            if (stop) t--;

            // break out of loop if restart flag is raised
            if (restart) break;

            float time = ((float) song_length[current_song % num_songs]) / 108;
            Thread::wait(1000 * time);
            
        }
    }
}

void pause(const void *args) {
    Thread::wait(1000); // initial wait
    while(1)
    {   
        Thread::wait(3000);
        waver.set_stop(1);
        printf("stop set\n\r");
        Thread::wait(3000);
        waver.set_stop(0);
        printf("continue set\n\r");
    }
}

void play_song(const void *args) {
    while (1) {
        wave_file=fopen(song_list[current_song % num_songs],"r");
        if(wave_file==NULL) {
			if (verbosity) printf("file open error!\n\n\r");
			continue;
		}
		waver.play(wave_file);
        fclose(wave_file);
		current_song++;
		Thread::wait(100);
    }
}

void bluetooth_control(const void *args) {
    char bnum=0;
    char bhit=0;
    while(1) {
        if (blue.readable()) {
			uLCD_mutex.lock();
        	if (blue.getc()=='!') {
            	if (blue.getc()=='B') { //button data packet
                	bnum = blue.getc(); //button number
                	bhit = blue.getc(); //1=hit, 0=release
                	if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK?
                    	led1 = !led1;                   
                    	switch (bnum) {
                        	case '1': //number button 1
                            	if (bhit=='1') {
                                	if(waver.get_stop()) {
										waver.set_stop(0);
                                        stop = 0;

									} else {
										waver.set_stop(1);
                                        stop = 1;
									}
                            	} else {
                                	//add release code here
                            	}
                            	break;
                        	case '2': //number button 2
								if (bhit=='1') {
									waver.set_kill(1);
                                    restart = 1;
                                    current_song--;
								} else {
									//add release code here
								}
								break;
							case '4': //number button 4
								if (bhit=='1') {
									waver.set_kill(1);
                                    restart = 1;
								} else {
									//add release code here
								}
								break;
							case '3': //number button 3
								if (bhit=='1') {
									waver.set_kill(1);
                                    restart = 1;
									current_song  = (current_song - 2);
								} else {
									//add release code here
								}
								break;
							case '5': //button 5 up arrow
								if (bhit=='1') {
									//add hit code here
								} else {
									//add release code here
								}
								break;
							case '6': //button 6 down arrow
								if (bhit=='1') {
									//add hit code here
								} else {
									//add release code here
								}
								break;
							case '7': //button 7 left arrow
								if (bhit=='1') {
									//add hit code here
								} else {
									//add release code here
								}
								break;
							case '8': //button 8 right arrow
								if (bhit=='1') {
									//add hit code here
								} else {
									//add release code here
								}
								break;
							default:
								break;
						}
                    }
                }
            }
			uLCD_mutex.unlock();
        }
        wait(0.05);
    }
}

int main()
{   
    // Motor and Pin Detect initialization
    drive = 0; // default fwd speed
    correction = 0;

    lbump5.mode(PullUp);
    lbump5.attach_deasserted(lbump5_callback);
    lbump5.setSampleFrequency();

    rbump0.mode(PullUp);
    rbump0.attach_deasserted(rbump0_callback);
    rbump0.setSampleFrequency();

    lbump4.mode(PullUp);
    lbump4.attach_deasserted(lbump4_callback);
    lbump4.setSampleFrequency();

    rbump1.mode(PullUp);
    rbump1.attach_deasserted(rbump1_callback);
    rbump1.setSampleFrequency();

    lbump3.mode(PullUp);
    lbump3.attach_deasserted(lbump3_callback);
    lbump3.setSampleFrequency();

    rbump2.mode(PullUp);
    rbump2.attach_deasserted(rbump2_callback);
    rbump2.setSampleFrequency();

    // Steup the uLCD
    uLCD.cls();
    uLCD.baudrate(1000000); // set baudrate to maximum possible speed
    for (int i = 0; i < 3; i++) {
        uLCD.locate(2, 7);
        uLCD.puts(init_text_array[i % 3]);
        Thread::wait(500);
    }

    // Initalize player UI
    draw_UI();

    printf("\r\n\nHello, wave world!\n\r");
    Thread::wait(1000);

    Thread progress_bar_thread(display_progress);
    printf("started progress bar thread...\r\n");

    Thread music_thread(play_song);
    printf("started music thread...\r\n");

    Thread bluetooth_thread(bluetooth_control);
    printf("bluetooth started...\r\n");

    while (1) {
        if (correction == RIGHT_TURN) {
            rm.speed(-drive);
            lm.speed(drive);
        } else if (correction == LEFT_TURN) {
            rm.speed(drive);
            lm.speed(-drive);
        } else {
            rm.speed(drive);
            lm.speed(drive);
        }
        Thread::wait(100);
    }
    
    // Terminate functions ensure proper termination of each thread after program main loop ends
    progress_bar_thread.terminate();
    music_thread.terminate();
    bluetooth_thread.terminate();
}