/********************************************************************************************* */
//    PI Calculator
//    Author: Svenja Ruoss
//    Juventus Technikerschule
//    Version: 1.0.0
//    
//   
//    
//   
/********************************************************************************************* */
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
    double time_taken1              = 0;
    clock_t start_time1             = clock(); // Startzeit erfassen

    while (1) {   

        if(!(xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            //Reset
            if(xEventGroupGetBits(xControlleventgroup) & RESET_BIT){
                sign        = 1;
                counter     = 0;
                k           = 0;
                LeibnizPi   = 0.0;
                time_taken1 = 0.0;
                gctime_takenL = time_taken1;
                xEventGroupClearBits(xControlleventgroup, RESET_BIT);
            }     
            if(xEventGroupGetBits(xControlleventgroup) & START_BIT) {
                LeibnizPi += sign / (2.0 * k + 1.0);
                //Wechseln des Vorzeichen für nächste Stelle
                sign = -sign;
                k++;
                counter++;
                //Abgleich für die Zeit
                if(Timefound == 0){
                    if (fabs((LeibnizPi * 4) - 3.14159) < 0.00001) {
                        clock_t end_time = clock(); // Endzeit erfassen
                        time_taken1 = (double)(end_time - start_time1) / CLOCKS_PER_SEC;
                        printf("Benötigte Zeit Leibnitz: %.6f Sekunden\n", time_taken1);
                        gctime_takenL = time_taken1;
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
    clock_t start_time2             = clock(); // Startzeit erfassen
    double time_taken2              = 0.0;

       while (1) {
        
        if((xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            //Reset
             if(xEventGroupGetBits(xControlleventgroup) & RESET_BIT){
                n = 1;
                NilakanthaPi = 3.0;
                time_taken2  = 0.0;
                gctime_takenN = time_taken2;
                xEventGroupClearBits(xControlleventgroup, RESET_BIT);
        }
            if(xEventGroupGetBits(xControlleventgroup) & START_BIT) {
                NilakanthaPi += (n % 2 == 0 ? -4.0 : 4.0) / ((2.0 * n) * (2.0 * n + 1.0) * (2.0 * n + 2.0));
                n++;
                //Abgleich für die Zeit
                if(Timefound == 0){
                    if (fabs(NilakanthaPi - 3.14159) < 0.00001) {
                        clock_t end_time = clock(); // Endzeit erfassen
                        time_taken2 = (double)(end_time - start_time2) / CLOCKS_PER_SEC;
                        printf("Benötigte Zeit Nilakantha: %.6f Sekunden\n", time_taken2);
                        gctime_takenN = time_taken2;
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

//----------------- Steuerung --------------------------------------------------------------------------
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

    char Leibniz[13];
    char Nilkantha[13];
    char TimeLeibniz[13];
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
        if(!(xEventGroupGetBits(xControlleventgroup) & SWITCH_ALGO_BIT)){
            lcdDrawString(fx32M, 10, 30, "PI-Calculator", WHITE);
            lcdDrawString(fx24M, 10, 100, "Leibnitz-Pi:", GREEN);
            lcdDrawString(fx24M, 10, 130, "Calculate-Time:", YELLOW);
            sprintf(Leibniz, "%.10f", LeibnizPi*4);
            lcdDrawString(fx24M, 200, 100, Leibniz, GREEN);
            sprintf(TimeLeibniz, "%f sek", gctime_takenL );
            lcdDrawString(fx24M, 200, 130, TimeLeibniz, YELLOW);
        }

        lcdDrawRect(0, 219, 119, 319,   GRAY);
        lcdDrawString(fx16M, 10, 280, "S0 = Ein", WHITE);
        lcdDrawRect(120, 219, 239, 319, GRAY);
        lcdDrawString(fx16M, 130, 280, "S1 = Aus", WHITE);
        lcdDrawRect(240, 219, 359, 319, GRAY);
        lcdDrawString(fx16M, 250, 280, "S2 = Reset", WHITE);
        lcdDrawRect(360, 219, 479, 319, GRAY);
        lcdDrawString(fx16M, 370, 280, "S3 = Switch", WHITE);

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