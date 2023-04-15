
#include "Thread.h"
#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"

// Just checking if commits and push works

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
uLCD_4DGL uLCD(p9, p10, p11);  // uLCD

AnalogOut DACout(p18);

wave_player waver(&DACout);

Mutex uLCD_mutex;
// uLCD setup text animation
char* init_text_array[] = {(char *) "Initializing.", (char *) "Initializing..", (char *) "Initializing..."};

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
        uLCD_mutex.lock();
        // clear the progress bar
        uLCD.filled_rectangle(11, 69, 119, 77, BLACK);
        uLCD_mutex.unlock();
        for (int t = 1; t <= 108; t++) {
            uLCD_mutex.lock();
            uLCD.filled_rectangle(11, 69, 11 + t, 77, GREEN);
            uLCD_mutex.unlock();
            Thread::wait(995);
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

int main()
{
    // Steup the uLCD
    uLCD.cls();
    uLCD.baudrate(3000000); // set baudrate to maximum possible speed
    for (int i = 0; i < 3; i++) {
        uLCD.locate(2, 7);
        uLCD.puts(init_text_array[i % 3]);
        Thread::wait(500);
    }
    // Initalize player UI
    draw_UI();

    FILE *wave_file;
    printf("\r\n\nHello, wave world!\n\r");
    Thread::wait(1000);

    Thread progress_bar_thread(display_progress);
    printf("started progress bar thread...\r\n");

    wave_file=fopen("/sd/song.wav","r");
    if(wave_file==NULL) printf("file open error!\n\n\r");
    waver.play(wave_file);
    fclose(wave_file);

    progress_bar_thread.terminate();
}