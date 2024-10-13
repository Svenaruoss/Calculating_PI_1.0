#include "eduboard2.h"
#include "memon.h"

#include "math.h"

#define TAG "PI Calcolator"

#define UPDATETIME_MS 100

//----------------- EventBits --------------------------------------------------------------------------
#define START_BIT               (1 << 0)
#define STOP_BIT                (1 << 1)
#define RESET_BIT               (1 << 2)
#define SWITCH_ALGO_BIT         (1 << 3)
#define UPDATE_DISPLAY_BIT      (1 << 4)

EventGroupHandle_t xControlleventgroup = NULL;

//----------------- Global Variables -------------------------------------------------------------------
int Timefound           = 0;
int AlgoBit             = 0;
double LeibnizPi        = 0.0;
double NilakanthaPi     = 3.0;
double gctime_takenL    = 0.0;
double gctime_takenN    = 0.0;


//----------------- Leibniz Task -----------------------------------------------------------------------
void LeibnizTask(void *param) {

    double sign                     = 1;
    int counter                     = 0;
    int k                           = 0;
    clock_t start_time              = clock(); // Startzeit erfassen

    while (1) {
 
        if(!(xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            if(xEventGroupGetBits(xControlleventgroup) & START_BIT) {
                LeibnizPi += sign / (2.0 * k + 1.0);
                printf("LeibnizPi = %.15f\n", LeibnizPi * 4);
                //Wechseln des Vorzeichen für nächste Stelle
                sign = -sign;
                k++;
                counter++;
                //Abgleich für die Zeit
                if(Timefound == 0){
                    if (fabs(LeibnizPi - 3.14159) < 0.00001) {
                        clock_t end_time = clock(); // Endzeit erfassen
                        double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
                        printf("Benötigte Zeit: %.5f Sekunden\n", time_taken);
                        gctime_takenL = time_taken;
                        Timefound = 1;
                    }
                }
            }
        }
        else{
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        vTaskDelay(1);
    }  
}

//----------------- Nilakantha Task --------------------------------------------------------------------
void NilakanthaTask(void *param) {

    int n                           = 1;
    clock_t start_time              = clock(); // Startzeit erfassen


    TickType_t xstartTime = xTaskGetTickCount();

       while (1) {

        if(xEventGroupGetBits(xControlleventgroup) & RESET_BIT){
            n = 1;
            NilakanthaPi = 3.0;
            xEventGroupClearBits(xControlleventgroup, RESET_BIT);
        }
        
        if((xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            if(xEventGroupGetBits(xControlleventgroup) & START_BIT) {
                NilakanthaPi += (n % 2 == 0 ? -4.0 : 4.0) / ((2.0 * n) * (2.0 * n + 1.0) * (2.0 * n + 2.0));
                n++;
                if (n % 1000 == 0) {
                    printf("Nilakantha π: %.10f\n", NilakanthaPi);
                }
                //Abgleich für die Zeit
                if(Timefound == 0){
                    if (fabs(NilakanthaPi - 3.14159) < 0.00001) {
                        clock_t end_time = clock(); // Endzeit erfassen
                        double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
                        printf("Benötigte Zeit: %.6f Sekunden\n", time_taken);
                        gctime_takenN = time_taken;
                        Timefound = 1;
                    }
                }
            }
        }
        else{
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        vTaskDelay(1);
    }
}
void ControllingTask(void *param){
    for(;;) {
        if(button_get_state(SW0, true) == SHORT_PRESSED) {
            xEventGroupSetBits(xControlleventgroup, START_BIT);
            xEventGroupClearBits(xControlleventgroup, STOP_BIT);
        }
        if(button_get_state(SW1, true) == SHORT_PRESSED) {
            xEventGroupSetBits(xControlleventgroup, STOP_BIT);
            xEventGroupClearBits(xControlleventgroup, START_BIT);
        }
        if(button_get_state(SW2, true) == SHORT_PRESSED) {
            xEventGroupSetBits(xControlleventgroup, RESET_BIT);
            Timefound = 0;
        }
        if(button_get_state(SW3, true) == SHORT_PRESSED) {
            if(AlgoBit == 0){
                xEventGroupSetBits(xControlleventgroup, SWITCH_ALGO_BIT);
                AlgoBit = 1;
            }
            else{
                xEventGroupClearBits(xControlleventgroup, SWITCH_ALGO_BIT);
                AlgoBit = 0;
            }
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

//----------------- LCD --------------------------------------------------------------------------------
void InterfaceTask(void *param){

    char Nilkantha[13];
    char TimeNilkantha[13];

    while (1){
        lcdFillScreen(BLACK);
        if((xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            lcdDrawString(fx32M, 10, 30, "PI-Calculator", WHITE);
            lcdDrawString(fx24M, 10, 100, "Nilkantha-Pi:", GREEN);
            lcdDrawString(fx24M, 10, 130, "Calculate-Time:", YELLOW);
            sprintf(Nilkantha, "%f", NilakanthaPi );
            lcdDrawString(fx24M, 200, 100, Nilkantha, GREEN);
            sprintf(TimeNilkantha, "%f sek", gctime_takenN );
            lcdDrawString(fx24M, 200, 130, TimeNilkantha, YELLOW);
        }

        lcdUpdateVScreen();
        vTaskDelay(500/portTICK_PERIOD_MS);
    }    
}

void app_main()
{
    //Initialize Eduboard2 BSP
    eduboard2_init();
    
    xControlleventgroup = xEventGroupCreate();

    xTaskCreate(LeibnizTask,        //Subroutine
            "LeibnizTask",          //Name
            2*2048,                 //Stacksize
            NULL,                   //Parameters
            1,                      //Priority
            NULL);                  //Taskhandle

    xTaskCreate(NilakanthaTask,     //Subroutine
            "NilakanthaTask",       //Name
            2*2048,                 //Stacksize
            NULL,                   //Parameters
            1,                      //Priority
            NULL);                  //Taskhandle

    xTaskCreate(ControllingTask,    //Subroutine
            "ControllingTask",      //Name
            2*2048,                 //Stacksize
            NULL,                   //Parameters
            10,                     //Priority
            NULL);                  //Taskhandle

    xTaskCreate(InterfaceTask,      //Subroutine
            "ControllingTask",      //Name
            2*2048,                 //Stacksize
            NULL,                   //Parameters
            7,                      //Priority
            NULL);                  //Taskhandle

}