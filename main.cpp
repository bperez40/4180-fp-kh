
#include "Thread.h"
#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include <cstdio>


SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card

AnalogOut DACout(p18);

wave_player waver(&DACout);

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