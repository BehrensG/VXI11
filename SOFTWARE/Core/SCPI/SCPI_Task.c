/*
 * SCPI_Task.c
 *
 *  Created on: Jun 2, 2024
 *      Author: grzegorz
 */

#include "scpi/scpi.h"
#include "SCPI_Def.h"

#include "cmsis_os.h"
#include "vxi11.h"

xQueueHandle scpi_queue;

typedef struct {
	u32_t len;
	char val[VXI11_MAX_RECV_SIZE + 2]; // 2 is for the termination characters
} scpi_data_t;

static scpi_data_t	scpi_data_rx;


#define SCPI_THREAD_STACKSIZE	1024

extern vxi11_instr_t vxi11_instr;

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {

    return 0;
}

scpi_result_t SCPI_Flush(scpi_t * context) {

    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {

    return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context) {

    (void) context;

    return SCPI_RES_OK;
}


void SCPI_RequestControl(void) {

}

void SCPI_AddError(int16_t err) {
}



static void scpi_task(void *arg) {

    SCPI_Init(&scpi_context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
            scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
            scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);



    for(;;){

    	if(pdTRUE == xQueueReceive(scpi_queue, &scpi_data_rx, portMAX_DELAY))
    	{

    		SCPI_Input(&scpi_context, &scpi_data_rx.val, scpi_data_rx.len);
    	}

    }


}

TaskHandle_t scpi_handler;
uint32_t scpi_buffer[SCPI_THREAD_STACKSIZE];
StaticTask_t scpi_control_block;


void SCPI_CreateTask(void) {

	scpi_handler = xTaskCreateStatic(scpi_task,"scpi_task",
			SCPI_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY + 4,
			scpi_buffer, &scpi_control_block);

	scpi_queue = xQueueCreate(1, sizeof(scpi_data_rx));

}
