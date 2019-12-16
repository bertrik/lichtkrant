#include <Arduino.h>

#include "leddriver.h"

static TaskHandle_t ledtask_handle;


void setup(void)
{
    Serial.begin(115200);
    Serial.printf("\nESP32-lichtkrant\n");

    led_clear();
    xTaskCreatePinnedToCore(led_task,   /* Function to implement the task */
                            "leddriver",        /* Name of the task */
                            10000,      /* Stack size in words */
                            NULL,       /* Task input parameter */
                            0,  /* Priority of the task */
                            &ledtask_handle,    /* Task handle. */
                            0); /* Core where the task should run */
}

void loop(void)
{
}


