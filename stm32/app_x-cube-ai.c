/* app_x-cube-ai.c */
#include "main.h"
#include "app_x-cube-ai.h"
#include "network.h"
#include "network_data.h"
#include <stdio.h>
#include <string.h>

/* Defined in main.c */
extern uint8_t img_buf[];
extern uint32_t img_len;
extern int request_picture(void);
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN PD */
#define APP_IMG_W        96
#define APP_IMG_H        96
#define APP_NUM_CLASSES  3
/* USER CODE END PD */

/* USER CODE BEGIN PV */
static const char *kLabels[APP_NUM_CLASSES] = {
    "organic", "paper", "plastic"      
};

STAI_ALIGNED(8)  static uint8_t net_ctx[STAI_NETWORK_CONTEXT_SIZE];
STAI_ALIGNED(32) static uint8_t net_act[STAI_NETWORK_ACTIVATIONS_SIZE_BYTES];

static stai_network *g_net = (stai_network *)net_ctx;
static stai_ptr      g_in[STAI_NETWORK_IN_NUM];
static stai_ptr      g_out[STAI_NETWORK_OUT_NUM];
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
/* RGB565 (low byte first) -> int8 RGB, zero point -128 */
static void rgb565_to_input(const uint8_t *src, int8_t *dst, uint32_t npix)
{
    for (uint32_t i = 0; i < npix; i++) {
        uint16_t px = (uint16_t)((src[2 * i] << 8) | src[2 * i + 1]);

        uint8_t r5 = (px >> 11) & 0x1F;
        uint8_t g6 = (px >> 5)  & 0x3F;
        uint8_t b5 =  px        & 0x1F;

        uint8_t r = (r5 << 3) | (r5 >> 2);
        uint8_t g = (g6 << 2) | (g6 >> 4);
        uint8_t b = (b5 << 3) | (b5 >> 2);

        dst[3 * i + 0] = (int8_t)(r - 128);
        dst[3 * i + 1] = (int8_t)(g - 128);
        dst[3 * i + 2] = (int8_t)(b - 128);
    }
}

/*
 * Frame packet sent to the PC over USART3:
 *   "IMG0" | width u16 LE | height u16 LE | length u32 LE | raw RGB565 bytes | checksum u32 LE
 * checksum = sum of the raw bytes
 */
static void send_frame_to_pc(const uint8_t *data, uint32_t len)
{
    uint8_t hdr[12] = {'I', 'M', 'G', '0',
                       APP_IMG_W, 0, APP_IMG_H, 0,
                       0, 0, 0, 0};
    hdr[8]  = (uint8_t)(len);
    hdr[9]  = (uint8_t)(len >> 8);
    hdr[10] = (uint8_t)(len >> 16);
    hdr[11] = (uint8_t)(len >> 24);

    uint32_t sum = 0;
    for (uint32_t i = 0; i < len; i++) sum += data[i];

    uint8_t cs[4] = {(uint8_t)sum, (uint8_t)(sum >> 8),
                     (uint8_t)(sum >> 16), (uint8_t)(sum >> 24)};

    HAL_UART_Transmit(&huart3, hdr, sizeof(hdr), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)data, (uint16_t)len, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, cs, sizeof(cs), HAL_MAX_DELAY);
}

static void classify_and_print(void)
{
    uint32_t t0 = HAL_GetTick();
    if (stai_network_run(g_net, STAI_MODE_SYNC) != STAI_SUCCESS) {
        printf("X-CUBE-AI run error\r\n");
        return;
    }
    uint32_t dt = HAL_GetTick() - t0;

    const int8_t *o = (const int8_t *)g_out[0];
    int best = 0;
    for (int i = 1; i < APP_NUM_CLASSES; i++) {
        if (o[i] > o[best]) best = i;
    }

    printf("pred=%s  %lu ms\r\n", kLabels[best], (unsigned long)dt);
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
void MX_X_CUBE_AI_Init(void)
{
    stai_ptr act[STAI_NETWORK_ACTIVATIONS_NUM] = { (stai_ptr)net_act };
    stai_size n;

    if (stai_network_init(g_net) != STAI_SUCCESS) Error_Handler();

    if (stai_network_set_activations(g_net, act, STAI_NETWORK_ACTIVATIONS_NUM) != STAI_SUCCESS)
        Error_Handler();

    n = STAI_NETWORK_IN_NUM;
    if (stai_network_get_inputs(g_net, g_in, &n) != STAI_SUCCESS || n != STAI_NETWORK_IN_NUM)
        Error_Handler();

    n = STAI_NETWORK_OUT_NUM;
    if (stai_network_get_outputs(g_net, g_out, &n) != STAI_SUCCESS || n != STAI_NETWORK_OUT_NUM)
        Error_Handler();

    printf("X-CUBE-AI ready\r\n");
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
void MX_X_CUBE_AI_Process(void)
{
    int rc = request_picture();
    if (rc != 0) {
        printf("capture error %d\r\n", rc);
        HAL_Delay(1000);
        return;
    }

    /* Send the raw frame to the PC first, then run inference on it */
    send_frame_to_pc(img_buf, APP_IMG_W * APP_IMG_H * 2);

    rgb565_to_input(img_buf, (int8_t *)g_in[0], APP_IMG_W * APP_IMG_H);
    classify_and_print();

    HAL_Delay(1000);
}
/* USER CODE END 2 */
