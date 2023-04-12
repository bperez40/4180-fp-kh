
#include "Thread.h"
#include "mbed.h"
#include "mbed2/238/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768/PinNames.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"
#include <cstdio>

// Just checking if commits and push works

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
uLCD_4DGL uLCD(p13, p14, p12);  // uLCD

AnalogOut DACout(p18);

wave_player waver(&DACout);

// uLCD setup text animation
char* init_text_array[] = {(char *) "Initializing.", (char *) "Initializing..", (char *) "Initializing..."};



void pause(const void *args) {
    Thread::wait(1000); // initial wait

    while(1)
    {   
        Thread::wait(2000);
        waver.set_stop(1);
        printf("stop set\n\r");
        Thread::wait(2000);
        waver.set_stop(0);
        printf("continue set\n\r");
    }
}

int main()
{
    FILE *wave_file;
    printf("\r\n\nHello, wave world!\n\r");
    Thread::wait(1000);


    Thread pause_thread(pause);
    printf("started pausing thread...\r\n");

    wave_file=fopen("/sd/song.wav","r");
    if(wave_file==NULL) printf("file open error!\n\n\r");
    waver.play(wave_file);
    fclose(wave_file);

    pause_thread.terminate();
}