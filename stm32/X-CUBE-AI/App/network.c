/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-10-08T08:42:40+0000
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "ai_lite_inspect.h"
#include "ai_platform_interface.h"
#include "layers.h"
#include "core_convert.h"
#include "network.h"
#include "network_details.h"
#include "network_data.h"
#include "stai_events.h"

#include "lite_operators.h"

#include "ai_lite_inspect.h"
/*****************************************************************************/
#define STAI_INTERNAL_API_MAJOR               (1)
#define STAI_INTERNAL_API_MINOR               (0)
#define STAI_INTERNAL_API_MICRO               (0)

#define STAI_MAGIC                            (0xB1C00100)

/*****************************************************************************/
#define _STAI_CONCAT_ARG(a, b)     a ## b
#define STAI_CONCAT(a, b)         _STAI_CONCAT_ARG(a, b)

/*!  STAI_CAST SECTION                       *********************************/
#define STAI_CAST(type, expr) \
  ((type)(expr))


/*****************************************************************************/
#define STAI_SIZE(_size) \
  ((stai_size)(_size))

/*****************************************************************************/
#define STAI_INIT_BUFFER(_flags, _size, _address) \
  { \
    .size = (_size), \
    .address = (uintptr_t)(_address), \
    .flags = (_flags), \
  }

#define STAI_INIT_TENSOR(_name, _flags, _fmt, _size_bytes, _shape, _scale, _zeropoint) \
  { \
    .size_bytes = (_size_bytes), \
    .flags = (_flags), \
    .format = (stai_format)(_fmt), \
    .shape = STAI_PACK(_shape), \
    .scale = STAI_PACK(_scale), \
    .zeropoint = STAI_PACK(_zeropoint), \
    .name = (_name) \
  }

#define STAI_INIT_ARRAY(_size, _ptr) \
  { .size = STAI_SIZE(_size), .data = STAI_PACK(_ptr) }


#define STAI_CAST_ARRAY(_type, _size, _ptr) \
  { .size = STAI_SIZE(_size), .data = (_type)STAI_PACK(_ptr) }


#define STAI_DECLARE_ARRAY(_type, _size, ...) \
  { .size = STAI_SIZE(_size), .data = (_type[_size]) { STAI_PACK(__VA_ARGS__) } }


#define STAI_EMPTY_ARRAY() \
  { .size = 0, .data = NULL }


#define STAI_INIT_VERSION(_major, _minor, _micro) \
  { .major = (_major), .minor = (_minor), .micro = (_micro), .reserved = 0x0 }

/*****************************************************************************/
/**  Getters and setters  **/

#define STAI_GET_ARRAY_SIZE(nd_array) \
  (nd_array.size)


#define STAI_GET_ARRAY_ELEM(nd_array, pos) \
  (nd_array.data[(pos)])

#define _STAI_SET_ERROR(net_ctx, cond, value, exit) { \
  if (!(net_ctx)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE; } \
  if (((uintptr_t)net_ctx) & (_STAI_CONTEXT_ALIGNMENT-1)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_ALIGNMENT; } \
  if (((value) >= STAI_ERROR_GENERIC) && (cond)) { \
    if ((net_ctx)->_return_code == STAI_SUCCESS) { \
      (net_ctx)->_return_code = (value); \
    } \
    return (exit); \
  } \
}

/*****************************************************************************/
/* TODO REMOVE THESE TWO MACROS */
#define STAI_EVENT_NODE_START_CB
#define STAI_EVENT_NODE_STOP_CB

#ifdef STAI_EVENT_NODE_START_CB
#ifndef _STAI_NETWORK_EVENT_NODE_START_CB
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _start_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(const stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_START, (const void*)&_start_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_START_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_START_CB */

#ifdef STAI_EVENT_NODE_STOP_CB
#ifndef _STAI_NETWORK_EVENT_NODE_STOP_CB
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _stop_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_STOP, (const void*)&_stop_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_STOP_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_STOP_CB */


/*****************************************************************************/
#define _STAI_NETWORK_MODEL_SIGNATURE     "0xe1ccf3855be0a9b83f99d9d2b8af503a"
#define _STAI_NETWORK_DATETIME            "2026-10-08T08:42:40+0000"
#define _STAI_NETWORK_COMPILE_DATETIME    __DATE__ " " __TIME__

#define _STAI_CONTEXT_ALIGNMENT        STAI_NETWORK_CONTEXT_ALIGNMENT

/*****************************************************************************/
#define g_network_activations_1     (NULL)




#if defined(HAVE_NETWORK_INFO)
/*****************************************************************************/
static const stai_network_info g_network_info = {
  .model_signature = _STAI_NETWORK_MODEL_SIGNATURE,
  .c_compile_datetime = _STAI_NETWORK_COMPILE_DATETIME,
  .c_model_name = STAI_NETWORK_MODEL_NAME,
  .c_model_datetime = _STAI_NETWORK_DATETIME,
  .c_model_signature = 0x0,
  .runtime_version = STAI_INIT_VERSION(12, 0, 1),
  .tool_version = STAI_INIT_VERSION(4, 0, 1),
  .api_version = STAI_INIT_VERSION(1, 0, 0),
  .n_macc = STAI_NETWORK_MACC_NUM,
  .n_nodes = STAI_NETWORK_NODES_NUM,
  .flags = STAI_NETWORK_FLAGS,
  .n_inputs = STAI_NETWORK_IN_NUM,
  .n_outputs = STAI_NETWORK_OUT_NUM,
  .n_activations = STAI_NETWORK_ACTIVATIONS_NUM,
  .n_weights = STAI_NETWORK_WEIGHTS_NUM,
  .n_states = STAI_NETWORK_STATES_NUM,
  .inputs = (stai_tensor[STAI_NETWORK_IN_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_1_NAME,
      STAI_NETWORK_IN_1_FLAGS,
      STAI_NETWORK_IN_1_FORMAT,
      STAI_NETWORK_IN_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 4, 1, 96, 96, 3),
      STAI_DECLARE_ARRAY(float, 1, 1.0f),
      STAI_DECLARE_ARRAY(int16_t, 1, -128)),
    },
    .outputs = (stai_tensor[STAI_NETWORK_OUT_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_1_NAME,
      STAI_NETWORK_OUT_1_FLAGS,
      STAI_NETWORK_OUT_1_FORMAT,
      STAI_NETWORK_OUT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 3),
      STAI_DECLARE_ARRAY(float, 1, 0.00390625f),
      STAI_DECLARE_ARRAY(int16_t, 1, -128)),
    },
  .activations = (stai_tensor[STAI_NETWORK_ACTIVATIONS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_ACTIVATION_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_ACTIVATION_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 129536),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
  .weights = (stai_tensor[STAI_NETWORK_WEIGHTS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_WEIGHT_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_WEIGHT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 414068),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },

  .states = NULL
};
#endif

#define _STAI_CONTEXT_ACQUIRE(_net_ctx, _net_handle) \
  _stai_network_context* _net_ctx = (_stai_network_context*)(_net_handle); \
  STAI_ASSERT(_net_ctx != NULL) \
  _STAI_SET_ERROR(_net_ctx, _net_ctx->_magic != STAI_MAGIC, \
                  STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE, _net_ctx->_return_code)


/*****************************************************************************/
static
void _stai_network_check(_stai_network_context* net_ctx)
{
  stai_size idx;

// Check activations status
  for (idx=0; idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    if (net_ctx->_activations[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_ACTIVATIONS_NUM) ? STAI_FLAG_ACTIVATIONS : STAI_FLAG_NONE;
// Check inputs status
  for (idx=0; idx<STAI_NETWORK_IN_NUM; idx++) {
    if (net_ctx->_inputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_IN_NUM) ? STAI_FLAG_INPUTS : STAI_FLAG_NONE;

  // Check outputs status
  for (idx=0; idx<STAI_NETWORK_OUT_NUM; idx++) {
    if (net_ctx->_outputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_OUT_NUM) ? STAI_FLAG_OUTPUTS : STAI_FLAG_NONE;

// Check weights status
  for (idx=0; idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    if (net_ctx->_weights[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_WEIGHTS_NUM) ? STAI_FLAG_WEIGHTS : STAI_FLAG_NONE;
STAI_PRINT("  [_stai_network_check] flags: 0x%08x\n", net_ctx->_flags)
}


/*****************************************************************************/
STAI_API_ENTRY
stai_return_code stai_network_init(
  stai_network* network)
{
  /* Memory where to store internal context is provided by applications as a raw byte buffer */
  _stai_network_context* net_ctx = (_stai_network_context*)(network);
  net_ctx->_return_code = STAI_SUCCESS;
  STAI_PRINT("[Entering Network Init] network(%p) context_size(%d)\n", net_ctx, (int32_t)sizeof(_stai_network_context))

  _STAI_SET_ERROR(net_ctx, STAI_NETWORK_CONTEXT_SIZE != sizeof(_stai_network_context),
                 STAI_ERROR_NETWORK_INVALID_CONTEXT_SIZE, net_ctx->_return_code)

  {
    const _stai_network_context _network_context = {
      ._magic = STAI_MAGIC,
      ._signature = STAI_NETWORK_MODEL_SIGNATURE,
      ._flags = STAI_NETWORK_FLAGS,
      ._return_code = STAI_SUCCESS,
      ._callback = NULL,
      ._callback_cookie = NULL,
      ._activations = {
      (stai_ptr)g_network_activations_1
      },
      ._weights = {
      (stai_ptr)g_network_weights_array
      },
      ._inputs = {
    NULL},
      ._outputs = {
    NULL},
    };

    // Deep copy of internal context to opaque buffer provided by app
    *net_ctx = _network_context;

    _stai_network_check(net_ctx);
  }

  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_deinit(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /*  Reset flags to initial state  */
  net_ctx->_flags = STAI_NETWORK_FLAGS;
  return net_ctx->_return_code;
}

/*****************************************************************************/



/* Int quant #0 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(serving_default_keras_tensor_16180_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(1.0f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #1 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0117647061124444f),
    AI_PACK_INTQ_ZP(-43)))

/* Int quant #2 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(functional_1_1_rescaling_1_1_Cast_x_4D_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(3.075740460189991e-05f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #3 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_1_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007843137718737125f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #4 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(functional_1_1_rescaling_1_1_Cast_1_4D_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921568859368563f),
    AI_PACK_INTQ_ZP(127)))

/* Int quant #5 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_7_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.3196049630641937f),
    AI_PACK_INTQ_ZP(5)))

/* Int quant #6 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_10_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.5001610517501831f),
    AI_PACK_INTQ_ZP(-4)))

/* Int quant #7 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_11_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.48252204060554504f),
    AI_PACK_INTQ_ZP(1)))

/* Int quant #8 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_14_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2943727970123291f),
    AI_PACK_INTQ_ZP(20)))

/* Int quant #9 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_17_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.27556443214416504f),
    AI_PACK_INTQ_ZP(10)))

/* Int quant #10 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_18_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2676216959953308f),
    AI_PACK_INTQ_ZP(14)))

/* Int quant #11 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_21_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.3419592082500458f),
    AI_PACK_INTQ_ZP(8)))

/* Int quant #12 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_22_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.3419592082500458f),
    AI_PACK_INTQ_ZP(8)))

/* Int quant #13 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_25_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.25301802158355713f),
    AI_PACK_INTQ_ZP(-12)))

/* Int quant #14 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_28_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.26992267370224f),
    AI_PACK_INTQ_ZP(-7)))

/* Int quant #15 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_29_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.26992267370224f),
    AI_PACK_INTQ_ZP(-7)))

/* Int quant #16 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_32_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.27030789852142334f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #17 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_33_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.27030789852142334f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #18 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_36_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.3205251395702362f),
    AI_PACK_INTQ_ZP(19)))

/* Int quant #19 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_37_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.3205251395702362f),
    AI_PACK_INTQ_ZP(19)))

/* Int quant #20 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_40_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.19619816541671753f),
    AI_PACK_INTQ_ZP(7)))

/* Int quant #21 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_43_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.24855132400989532f),
    AI_PACK_INTQ_ZP(-13)))

/* Int quant #22 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_44_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.24855132400989532f),
    AI_PACK_INTQ_ZP(-13)))

/* Int quant #23 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_47_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2924808859825134f),
    AI_PACK_INTQ_ZP(3)))

/* Int quant #24 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_48_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2924808859825134f),
    AI_PACK_INTQ_ZP(3)))

/* Int quant #25 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_51_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.16510659456253052f),
    AI_PACK_INTQ_ZP(16)))

/* Int quant #26 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_54_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.1923304796218872f),
    AI_PACK_INTQ_ZP(-10)))

/* Int quant #27 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_55_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.1923304796218872f),
    AI_PACK_INTQ_ZP(-10)))

/* Int quant #28 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_58_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.22229193150997162f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #29 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(eltwise_59_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.22229193150997162f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #30 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_63_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0235294122248888f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #31 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(pool_64_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0235294122248888f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #32 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(gemm_65_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.07735281437635422f),
    AI_PACK_INTQ_ZP(-19)))

/* Int quant #33 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(gemm_65_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 3,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0011936594964936376f, 0.0014821439981460571f, 0.0012731109745800495f),
    AI_PACK_INTQ_ZP(0, 0, 0)))



/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  serving_default_keras_tensor_16180_output_array, AI_ARRAY_FORMAT_S8|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 27648, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 27648, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  functional_1_1_rescaling_1_1_Cast_x_4D_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_1_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 27649, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  functional_1_1_rescaling_1_1_Cast_1_4D_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_7_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 4608, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_10_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 4608, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_11_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 4608, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_14_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 2304, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_17_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 2304, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_18_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 2304, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_21_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 2304, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_22_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 2304, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_25_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_28_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_29_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_32_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_33_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_36_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_37_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 864, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_40_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1152, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_43_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1152, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_44_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1152, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_47_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1152, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_48_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1152, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_51_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 504, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_54_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 504, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_55_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 504, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_58_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 504, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  eltwise_59_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 504, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_63_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 11520, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  pool_64_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1280, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  gemm_65_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 3, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  gemm_65_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 3840, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  gemm_65_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 3, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  gemm_65_scratch0_array, AI_ARRAY_FORMAT_S16,
  NULL, NULL, 1295, AI_STATIC)



/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_0_output, AI_STATIC,
  225, 0x1,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 1, 1, 3, 288),
  1, &eltwise_0_output_array, &eltwise_0_output_array_intq)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  functional_1_1_rescaling_1_1_Cast_x_4D, AI_STATIC,
  238, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &functional_1_1_rescaling_1_1_Cast_x_4D_array, &functional_1_1_rescaling_1_1_Cast_x_4D_array_intq)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  serving_default_keras_tensor_16180_output, AI_STATIC,
  246, 0x1,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 1, 1, 3, 288),
  1, &serving_default_keras_tensor_16180_output_array, &serving_default_keras_tensor_16180_output_array_intq)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_1_output, AI_STATIC,
  228, 0x1,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 1, 1, 3, 288),
  1, &eltwise_1_output_array, &eltwise_1_output_array_intq)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  functional_1_1_rescaling_1_1_Cast_1_4D, AI_STATIC,
  237, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &functional_1_1_rescaling_1_1_Cast_1_4D_array, &functional_1_1_rescaling_1_1_Cast_1_4D_array_intq)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_10_output, AI_STATIC,
  1, 0x1,
  AI_SHAPE_INIT(4, 1, 8, 24, 24), AI_STRIDE_INIT(4, 1, 1, 8, 192),
  1, &conv2d_10_output_array, &conv2d_10_output_array_intq)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_7_output, AI_STATIC,
  213, 0x1,
  AI_SHAPE_INIT(4, 1, 8, 24, 24), AI_STRIDE_INIT(4, 1, 1, 8, 192),
  1, &conv2d_7_output_array, &conv2d_7_output_array_intq)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_11_output, AI_STATIC,
  226, 0x1,
  AI_SHAPE_INIT(4, 1, 8, 24, 24), AI_STRIDE_INIT(4, 1, 1, 8, 192),
  1, &eltwise_11_output_array, &eltwise_11_output_array_intq)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_14_output, AI_STATIC,
  14, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 12, 12), AI_STRIDE_INIT(4, 1, 1, 16, 192),
  1, &conv2d_14_output_array, &conv2d_14_output_array_intq)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_17_output, AI_STATIC,
  27, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 12, 12), AI_STRIDE_INIT(4, 1, 1, 16, 192),
  1, &conv2d_17_output_array, &conv2d_17_output_array_intq)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_18_output, AI_STATIC,
  227, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 12, 12), AI_STRIDE_INIT(4, 1, 1, 16, 192),
  1, &eltwise_18_output_array, &eltwise_18_output_array_intq)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_21_output, AI_STATIC,
  40, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 12, 12), AI_STRIDE_INIT(4, 1, 1, 16, 192),
  1, &conv2d_21_output_array, &conv2d_21_output_array_intq)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_22_output, AI_STATIC,
  229, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 12, 12), AI_STRIDE_INIT(4, 1, 1, 16, 192),
  1, &eltwise_22_output_array, &eltwise_22_output_array_intq)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_25_output, AI_STATIC,
  53, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &conv2d_25_output_array, &conv2d_25_output_array_intq)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_28_output, AI_STATIC,
  66, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &conv2d_28_output_array, &conv2d_28_output_array_intq)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_29_output, AI_STATIC,
  230, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &eltwise_29_output_array, &eltwise_29_output_array_intq)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_32_output, AI_STATIC,
  83, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &conv2d_32_output_array, &conv2d_32_output_array_intq)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_33_output, AI_STATIC,
  231, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &eltwise_33_output_array, &eltwise_33_output_array_intq)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_36_output, AI_STATIC,
  96, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &conv2d_36_output_array, &conv2d_36_output_array_intq)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_37_output, AI_STATIC,
  232, 0x1,
  AI_SHAPE_INIT(4, 1, 24, 6, 6), AI_STRIDE_INIT(4, 1, 1, 24, 144),
  1, &eltwise_37_output_array, &eltwise_37_output_array_intq)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_40_output, AI_STATIC,
  114, 0x1,
  AI_SHAPE_INIT(4, 1, 32, 6, 6), AI_STRIDE_INIT(4, 1, 1, 32, 192),
  1, &conv2d_40_output_array, &conv2d_40_output_array_intq)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_43_output, AI_STATIC,
  127, 0x1,
  AI_SHAPE_INIT(4, 1, 32, 6, 6), AI_STRIDE_INIT(4, 1, 1, 32, 192),
  1, &conv2d_43_output_array, &conv2d_43_output_array_intq)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_44_output, AI_STATIC,
  233, 0x1,
  AI_SHAPE_INIT(4, 1, 32, 6, 6), AI_STRIDE_INIT(4, 1, 1, 32, 192),
  1, &eltwise_44_output_array, &eltwise_44_output_array_intq)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_47_output, AI_STATIC,
  140, 0x1,
  AI_SHAPE_INIT(4, 1, 32, 6, 6), AI_STRIDE_INIT(4, 1, 1, 32, 192),
  1, &conv2d_47_output_array, &conv2d_47_output_array_intq)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_48_output, AI_STATIC,
  234, 0x1,
  AI_SHAPE_INIT(4, 1, 32, 6, 6), AI_STRIDE_INIT(4, 1, 1, 32, 192),
  1, &eltwise_48_output_array, &eltwise_48_output_array_intq)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_51_output, AI_STATIC,
  157, 0x1,
  AI_SHAPE_INIT(4, 1, 56, 3, 3), AI_STRIDE_INIT(4, 1, 1, 56, 168),
  1, &conv2d_51_output_array, &conv2d_51_output_array_intq)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_54_output, AI_STATIC,
  170, 0x1,
  AI_SHAPE_INIT(4, 1, 56, 3, 3), AI_STRIDE_INIT(4, 1, 1, 56, 168),
  1, &conv2d_54_output_array, &conv2d_54_output_array_intq)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_55_output, AI_STATIC,
  235, 0x1,
  AI_SHAPE_INIT(4, 1, 56, 3, 3), AI_STRIDE_INIT(4, 1, 1, 56, 168),
  1, &eltwise_55_output_array, &eltwise_55_output_array_intq)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_58_output, AI_STATIC,
  183, 0x1,
  AI_SHAPE_INIT(4, 1, 56, 3, 3), AI_STRIDE_INIT(4, 1, 1, 56, 168),
  1, &conv2d_58_output_array, &conv2d_58_output_array_intq)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  eltwise_59_output, AI_STATIC,
  236, 0x1,
  AI_SHAPE_INIT(4, 1, 56, 3, 3), AI_STRIDE_INIT(4, 1, 1, 56, 168),
  1, &eltwise_59_output_array, &eltwise_59_output_array_intq)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_63_output, AI_STATIC,
  204, 0x1,
  AI_SHAPE_INIT(4, 1, 1280, 3, 3), AI_STRIDE_INIT(4, 1, 1, 1280, 3840),
  1, &conv2d_63_output_array, &conv2d_63_output_array_intq)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  pool_64_output, AI_STATIC,
  245, 0x1,
  AI_SHAPE_INIT(4, 1, 1280, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1280, 1280),
  1, &pool_64_output_array, &pool_64_output_array_intq)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  gemm_65_bias, AI_STATIC,
  239, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 1, 1), AI_STRIDE_INIT(4, 4, 4, 12, 12),
  1, &gemm_65_bias_array, NULL)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  gemm_65_output, AI_STATIC,
  240, 0x1,
  AI_SHAPE_INIT(4, 1, 3, 1, 1), AI_STRIDE_INIT(4, 1, 1, 3, 3),
  1, &gemm_65_output_array, &gemm_65_output_array_intq)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  gemm_65_scratch0, AI_STATIC,
  241, 0x0,
  AI_SHAPE_INIT(4, 1, 1295, 1, 1), AI_STRIDE_INIT(4, 2, 2, 2590, 2590),
  1, &gemm_65_scratch0_array, NULL)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  gemm_65_weights, AI_STATIC,
  242, 0x1,
  AI_SHAPE_INIT(4, 1280, 3, 1, 1), AI_STRIDE_INIT(4, 1, 1280, 3840, 3840),
  1, &gemm_65_weights_array, &gemm_65_weights_array_intq)


AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &serving_default_keras_tensor_16180_output, &functional_1_1_rescaling_1_1_Cast_x_4D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_0_layer, 0,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_0_chain,
  NULL, &eltwise_0_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_1_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_0_output, &functional_1_1_rescaling_1_1_Cast_1_4D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_1_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_1_layer, 1,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_1_chain,
  NULL, &eltwise_1_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_11_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &conv2d_7_output, &conv2d_10_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_11_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_11_layer, 11,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_11_chain,
  NULL, &eltwise_11_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_18_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &conv2d_14_output, &conv2d_17_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_18_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_18_layer, 18,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_18_chain,
  NULL, &eltwise_18_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_22_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_18_output, &conv2d_21_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_22_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_22_layer, 22,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_22_chain,
  NULL, &eltwise_22_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_29_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &conv2d_25_output, &conv2d_28_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_29_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_29_layer, 29,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_29_chain,
  NULL, &eltwise_29_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_33_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_29_output, &conv2d_32_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_33_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_33_layer, 33,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_33_chain,
  NULL, &eltwise_33_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_37_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_33_output, &conv2d_36_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_37_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_37_layer, 37,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_37_chain,
  NULL, &eltwise_37_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_44_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &conv2d_40_output, &conv2d_43_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_44_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_44_layer, 44,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_44_chain,
  NULL, &eltwise_44_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_48_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_44_output, &conv2d_47_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_48_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_48_layer, 48,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_48_chain,
  NULL, &eltwise_48_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_55_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &conv2d_51_output, &conv2d_54_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_55_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_55_layer, 55,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_55_chain,
  NULL, &eltwise_55_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  eltwise_59_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &eltwise_55_output, &conv2d_58_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &eltwise_59_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  eltwise_59_layer, 59,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &eltwise_59_chain,
  NULL, &eltwise_59_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  pool_64_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_63_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &pool_64_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  pool_64_layer, 64,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap_integer_INT8,
  &pool_64_chain,
  NULL, &pool_64_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(3, 3), 
  .pool_stride = AI_SHAPE_2D_INIT(3, 3), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  gemm_65_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &pool_64_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &gemm_65_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &gemm_65_weights, &gemm_65_bias),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &gemm_65_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  gemm_65_layer, 65,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense_integer_SSSA_ch,
  &gemm_65_chain,
  NULL, &gemm_65_layer, AI_STATIC, 
)
/**  Hybrid layers declarations section  *************************************/
void forward_lite_eltwise_integer_INT8_eltwise_0(_stai_network_context* net_ctx)
{
  serving_default_keras_tensor_16180_output_array.data = AI_PTR(net_ctx->_inputs[0] + 0);
  serving_default_keras_tensor_16180_output_array.data_start = AI_PTR(net_ctx->_inputs[0] + 0);
  functional_1_1_rescaling_1_1_Cast_x_4D_array.data = AI_PTR(net_ctx->_weights[0] + 0);
  functional_1_1_rescaling_1_1_Cast_x_4D_array.data_start = AI_PTR(net_ctx->_weights[0] + 0);
  eltwise_0_output_array.data = AI_PTR(net_ctx->_activations[0] + 14292);
  eltwise_0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 14292);
  _STAI_NETWORK_EVENT_NODE_START_CB(0, 2, { serving_default_keras_tensor_16180_output.data->data,functional_1_1_rescaling_1_1_Cast_x_4D.data->data});
  forward_eltwise_integer_INT8(&eltwise_0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(0, 1, { eltwise_0_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_1(_stai_network_context* net_ctx)
{
  eltwise_0_output_array.data = AI_PTR(net_ctx->_activations[0] + 14292);
  eltwise_0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 14292);
  functional_1_1_rescaling_1_1_Cast_1_4D_array.data = AI_PTR(net_ctx->_weights[0] + 4);
  functional_1_1_rescaling_1_1_Cast_1_4D_array.data_start = AI_PTR(net_ctx->_weights[0] + 4);
  eltwise_1_output_array.data = AI_PTR(net_ctx->_activations[0] + 14288);
  eltwise_1_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 14288);
  _STAI_NETWORK_EVENT_NODE_START_CB(1, 2, { eltwise_0_output.data->data,functional_1_1_rescaling_1_1_Cast_1_4D.data->data});
  forward_eltwise_integer_INT8(&eltwise_1_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(1, 1, { eltwise_1_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_11(_stai_network_context* net_ctx)
{
  conv2d_7_output_array.data = AI_PTR(net_ctx->_activations[0] + 272);
  conv2d_7_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 272);
  conv2d_10_output_array.data = AI_PTR(net_ctx->_activations[0] + 4880);
  conv2d_10_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 4880);
  eltwise_11_output_array.data = AI_PTR(net_ctx->_activations[0] + 9488);
  eltwise_11_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 9488);
  _STAI_NETWORK_EVENT_NODE_START_CB(11, 2, { conv2d_7_output.data->data,conv2d_10_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_11_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(11, 1, { eltwise_11_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_18(_stai_network_context* net_ctx)
{
  conv2d_14_output_array.data = AI_PTR(net_ctx->_activations[0] + 8692);
  conv2d_14_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 8692);
  conv2d_17_output_array.data = AI_PTR(net_ctx->_activations[0] + 544);
  conv2d_17_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 544);
  eltwise_18_output_array.data = AI_PTR(net_ctx->_activations[0] + 2848);
  eltwise_18_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2848);
  _STAI_NETWORK_EVENT_NODE_START_CB(18, 2, { conv2d_14_output.data->data,conv2d_17_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_18_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(18, 1, { eltwise_18_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_22(_stai_network_context* net_ctx)
{
  eltwise_18_output_array.data = AI_PTR(net_ctx->_activations[0] + 2848);
  eltwise_18_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2848);
  conv2d_21_output_array.data = AI_PTR(net_ctx->_activations[0] + 544);
  conv2d_21_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 544);
  eltwise_22_output_array.data = AI_PTR(net_ctx->_activations[0] + 5152);
  eltwise_22_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 5152);
  _STAI_NETWORK_EVENT_NODE_START_CB(22, 2, { eltwise_18_output.data->data,conv2d_21_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_22_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(22, 1, { eltwise_22_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_29(_stai_network_context* net_ctx)
{
  conv2d_25_output_array.data = AI_PTR(net_ctx->_activations[0] + 624);
  conv2d_25_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 624);
  conv2d_28_output_array.data = AI_PTR(net_ctx->_activations[0] + 2304);
  conv2d_28_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2304);
  eltwise_29_output_array.data = AI_PTR(net_ctx->_activations[0] + 3168);
  eltwise_29_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 3168);
  _STAI_NETWORK_EVENT_NODE_START_CB(29, 2, { conv2d_25_output.data->data,conv2d_28_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_29_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(29, 1, { eltwise_29_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_33(_stai_network_context* net_ctx)
{
  eltwise_29_output_array.data = AI_PTR(net_ctx->_activations[0] + 3168);
  eltwise_29_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 3168);
  conv2d_32_output_array.data = AI_PTR(net_ctx->_activations[0] + 816);
  conv2d_32_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 816);
  eltwise_33_output_array.data = AI_PTR(net_ctx->_activations[0] + 1680);
  eltwise_33_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1680);
  _STAI_NETWORK_EVENT_NODE_START_CB(33, 2, { eltwise_29_output.data->data,conv2d_32_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_33_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(33, 1, { eltwise_33_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_37(_stai_network_context* net_ctx)
{
  eltwise_33_output_array.data = AI_PTR(net_ctx->_activations[0] + 1680);
  eltwise_33_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1680);
  conv2d_36_output_array.data = AI_PTR(net_ctx->_activations[0] + 816);
  conv2d_36_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 816);
  eltwise_37_output_array.data = AI_PTR(net_ctx->_activations[0] + 2544);
  eltwise_37_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2544);
  _STAI_NETWORK_EVENT_NODE_START_CB(37, 2, { eltwise_33_output.data->data,conv2d_36_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_37_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(37, 1, { eltwise_37_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_44(_stai_network_context* net_ctx)
{
  conv2d_40_output_array.data = AI_PTR(net_ctx->_activations[0] + 896);
  conv2d_40_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 896);
  conv2d_43_output_array.data = AI_PTR(net_ctx->_activations[0] + 3136);
  conv2d_43_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 3136);
  eltwise_44_output_array.data = AI_PTR(net_ctx->_activations[0] + 4288);
  eltwise_44_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 4288);
  _STAI_NETWORK_EVENT_NODE_START_CB(44, 2, { conv2d_40_output.data->data,conv2d_43_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_44_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(44, 1, { eltwise_44_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_48(_stai_network_context* net_ctx)
{
  eltwise_44_output_array.data = AI_PTR(net_ctx->_activations[0] + 4288);
  eltwise_44_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 4288);
  conv2d_47_output_array.data = AI_PTR(net_ctx->_activations[0] + 1088);
  conv2d_47_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1088);
  eltwise_48_output_array.data = AI_PTR(net_ctx->_activations[0] + 2240);
  eltwise_48_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2240);
  _STAI_NETWORK_EVENT_NODE_START_CB(48, 2, { eltwise_44_output.data->data,conv2d_47_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_48_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(48, 1, { eltwise_48_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_55(_stai_network_context* net_ctx)
{
  conv2d_51_output_array.data = AI_PTR(net_ctx->_activations[0] + 1328);
  conv2d_51_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1328);
  conv2d_54_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  conv2d_54_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  eltwise_55_output_array.data = AI_PTR(net_ctx->_activations[0] + 504);
  eltwise_55_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 504);
  _STAI_NETWORK_EVENT_NODE_START_CB(55, 2, { conv2d_51_output.data->data,conv2d_54_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_55_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(55, 1, { eltwise_55_output.data->data});
}
void forward_lite_eltwise_integer_INT8_eltwise_59(_stai_network_context* net_ctx)
{
  eltwise_55_output_array.data = AI_PTR(net_ctx->_activations[0] + 504);
  eltwise_55_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 504);
  conv2d_58_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  conv2d_58_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  eltwise_59_output_array.data = AI_PTR(net_ctx->_activations[0] + 1008);
  eltwise_59_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1008);
  _STAI_NETWORK_EVENT_NODE_START_CB(59, 2, { eltwise_55_output.data->data,conv2d_58_output.data->data});
  forward_eltwise_integer_INT8(&eltwise_59_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(59, 1, { eltwise_59_output.data->data});
}
void forward_lite_ap_integer_INT8_pool_64(_stai_network_context* net_ctx)
{
  conv2d_63_output_array.data = AI_PTR(net_ctx->_activations[0] + 19744);
  conv2d_63_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 19744);
  pool_64_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  pool_64_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  _STAI_NETWORK_EVENT_NODE_START_CB(64, 1, { conv2d_63_output.data->data});
  forward_ap_integer_INT8(&pool_64_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(64, 1, { pool_64_output.data->data});
}
void forward_lite_dense_integer_SSSA_ch_gemm_65(_stai_network_context* net_ctx)
{
  pool_64_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  pool_64_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  gemm_65_weights_array.data = AI_PTR(net_ctx->_weights[0] + 410216);
  gemm_65_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 410216);
  gemm_65_bias_array.data = AI_PTR(net_ctx->_weights[0] + 414056);
  gemm_65_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 414056);
  gemm_65_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 1280);
  gemm_65_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 1280);
  gemm_65_output_array.data = AI_PTR(net_ctx->_activations[0] + 3872);
  gemm_65_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 3872);
  _STAI_NETWORK_EVENT_NODE_START_CB(65, 1, { pool_64_output.data->data});
  forward_dense_integer_SSSA_ch(&gemm_65_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(65, 1, { gemm_65_output.data->data});
}

/*****************************************************************************/




static const ai_u16 conv2d_2_t_in_0_shape_w_const_u16 = 96;
static const ai_u16 conv2d_2_t_out_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_2_t_weight_0_shape_w_const_u16 = 3;
static const ai_i32 conv2d_2_l_pad_W_0_const_s32 = 0;
static const ai_u16 conv2d_2_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_2_t_in_0_fmt_zero_const_s8 = -1;
static const ai_i8 conv2d_2_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_2_t_in_0_fmt_scale_const_f32 = 0.007843137718737125f;
static const ai_float conv2d_2_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_2_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(1.67050970389937e-07f, 0.020020049065351486f, 0.020457018166780472f, 0.004432215820997953f, 0.0012904924806207418f, 0.020331308245658875f, 0.0038729673251509666f, 0.028989583253860474f, 0.000321097148116678f, 0.005294324830174446f, 0.007798223290592432f, 0.0013371178647503257f, 1.4705690887240053e-07f, 0.005538501776754856f, 0.0033088813070207834f, 1.58530273353108e-08f);
static const ai_layer_format_type conv2d_2_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;
static const ai_u16 conv2d_2_t_out_0_shape_w_const_u16 = 48;

static const ai_i8 conv2d_3_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_3_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_3_pad_before_t_in_0_shape_h_const_u32 = 48;

static const ai_u16 conv2d_3_t_in_0_shape_w_const_u16 = 50;
static const ai_u16 conv2d_3_t_in_0_shape_h_const_u16 = 50;
static const ai_u16 conv2d_3_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_3_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_3_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_3_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_3_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_3_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_3_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_3_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.07942783832550049f, 0.0047692651860415936f, 0.00734791299328208f, 0.010866953060030937f, 0.006442848592996597f, 0.006952024530619383f, 0.016511162742972374f, 0.009069700725376606f, 0.001307512866333127f, 0.010150759480893612f, 0.008227416314184666f, 0.032525744289159775f, 0.030907778069376945f, 0.01007858570665121f, 0.00808307621628046f, 0.1660681664943695f);
static const ai_u16 conv2d_3_t_out_0_shape_w_const_u16 = 48;
static const ai_u16 conv2d_3_t_out_0_shape_h_const_u16 = 48;

static const ai_u16 conv2d_4_t_in_0_shape_w_const_u16 = 48;
static const ai_u16 conv2d_4_t_in_0_shape_h_const_u16 = 48;
static const ai_u16 conv2d_4_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_4_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_4_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_4_t_out_0_shape_ch_const_u16 = 8;
static const ai_i8 conv2d_4_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_4_t_out_0_fmt_zero_const_s8 = 3;
static const ai_float conv2d_4_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_4_t_out_0_fmt_scale_const_f32 = 0.3810703456401825f;
static const ai_float conv2d_4_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.08344659209251404f, 0.08535084128379822f, 0.10206731408834457f, 0.04747951403260231f, 0.05371420830488205f, 0.09569042921066284f, 0.05823199823498726f, 0.04202578589320183f);
static const ai_layer_format_type conv2d_4_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_5_t_in_0_shape_w_const_u16 = 48;
static const ai_u16 conv2d_5_t_in_0_shape_h_const_u16 = 48;
static const ai_u16 conv2d_5_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_5_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_5_t_in_0_shape_ch_const_u16 = 8;
static const ai_u16 conv2d_5_t_out_0_shape_ch_const_u16 = 48;
static const ai_i8 conv2d_5_t_in_0_fmt_zero_const_s8 = 3;
static const ai_i8 conv2d_5_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_5_t_in_0_fmt_scale_const_f32 = 0.3810703456401825f;
static const ai_float conv2d_5_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_5_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0009442944428883493f, 0.0008819265058264136f, 0.000634416239336133f, 0.0005880667013116181f, 0.0005328754778020084f, 0.000638393044937402f, 0.0007114742184057832f, 0.0023692219983786345f, 0.0007385837379842997f, 0.004810930695384741f, 0.001163010485470295f, 0.0038401640485972166f, 0.00036209443351253867f, 3.937008052901092e-09f, 0.001879889634437859f, 0.0003274135524407029f, 3.937008052901092e-09f, 0.01264355331659317f, 0.0008123833104036748f, 0.0005908211460337043f, 0.000564271816983819f, 0.008824929594993591f, 0.0006118659512139857f, 0.003059039358049631f, 0.0021761483512818813f, 0.001283719902858138f, 0.004259760491549969f, 0.004223599564284086f, 0.0005627350765280426f, 0.003162824548780918f, 0.0024733636528253555f, 0.0003393071820028126f, 0.0003929729282390326f, 0.018245341256260872f, 0.0008635405101813376f, 0.0010651235934346914f, 0.007317767478525639f, 0.0007256458047777414f, 0.005316259805113077f, 0.0005110740312375128f, 0.0023833357263356447f, 0.0003610709682106972f, 0.0009355738875456154f, 0.017371080815792084f, 0.0007212291820906103f, 0.0010740740690380335f, 0.0005287231178954244f, 0.0003547391388565302f);
static const ai_layer_format_type conv2d_5_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_6_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_6_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_6_pad_before_t_in_0_shape_h_const_u32 = 48;

static const ai_u16 conv2d_6_t_in_0_shape_w_const_u16 = 50;
static const ai_u16 conv2d_6_t_in_0_shape_h_const_u16 = 50;
static const ai_u16 conv2d_6_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_6_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_6_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_6_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_6_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_6_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_6_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_6_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.00459868973121047f, 0.0030829247552901506f, 0.007233722601085901f, 0.0050545730628073215f, 0.00569942407310009f, 0.0068049682304263115f, 0.0053602950647473335f, 0.0024333300534635782f, 0.0037144972011446953f, 0.0013556275516748428f, 0.0033788387663662434f, 0.0012788695748895407f, 0.007493840530514717f, 0.23551659286022186f, 0.0028321384452283382f, 0.00944758951663971f, 0.1168077290058136f, 0.0008903977577574551f, 0.0039820135571062565f, 0.007123882882297039f, 0.004878988489508629f, 0.0062552024610340595f, 0.005590423941612244f, 0.0014703992055729032f, 0.0037665951531380415f, 0.003729436080902815f, 0.0014245398342609406f, 0.002022996312007308f, 0.012412821874022484f, 0.0024607637897133827f, 0.002112720860168338f, 0.007757976185530424f, 0.008985541760921478f, 0.0004612184129655361f, 0.003685558680444956f, 0.009281540289521217f, 0.0007337765418924391f, 0.018105072900652885f, 0.0014377038460224867f, 0.004057862795889378f, 0.0030589003581553698f, 0.009788176976144314f, 0.00494390819221735f, 0.00047133592306636274f, 0.006481362506747246f, 0.003036129754036665f, 0.009674615226686f, 0.005337184760719538f);
static const ai_u16 conv2d_6_t_out_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_6_t_out_0_shape_h_const_u16 = 24;

static const ai_u16 conv2d_7_t_in_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_7_t_in_0_shape_h_const_u16 = 24;
static const ai_u16 conv2d_7_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_7_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_7_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_7_t_out_0_shape_ch_const_u16 = 8;
static const ai_i8 conv2d_7_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_7_t_out_0_fmt_zero_const_s8 = 5;
static const ai_float conv2d_7_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_7_t_out_0_fmt_scale_const_f32 = 0.3196049630641937f;
static const ai_float conv2d_7_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.05558214709162712f, 0.028268953785300255f, 0.056880999356508255f, 0.054720181971788406f, 0.04849627614021301f, 0.04898638650774956f, 0.03630875423550606f, 0.03257575258612633f);
static const ai_layer_format_type conv2d_7_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_8_t_in_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_8_t_in_0_shape_h_const_u16 = 24;
static const ai_u16 conv2d_8_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_8_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_8_t_in_0_shape_ch_const_u16 = 8;
static const ai_u16 conv2d_8_t_out_0_shape_ch_const_u16 = 48;
static const ai_i8 conv2d_8_t_in_0_fmt_zero_const_s8 = 5;
static const ai_i8 conv2d_8_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_8_t_in_0_fmt_scale_const_f32 = 0.3196049630641937f;
static const ai_float conv2d_8_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_8_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.00019171890744473785f, 0.0007587759173475206f, 0.0005793729214929044f, 0.0010129715083166957f, 0.0033828739542514086f, 0.0012564145727083087f, 0.0022991274017840624f, 0.0028063992504030466f, 0.0015147895319387317f, 0.0014493368798866868f, 0.001676068757660687f, 0.0011880374513566494f, 0.0013159648515284061f, 0.001307573402300477f, 0.0013875187141820788f, 0.0043866257183253765f, 0.0007367674261331558f, 0.0008609220385551453f, 0.004734290298074484f, 0.00025562511291354895f, 0.0006831393693573773f, 0.00089532311540097f, 0.0011386539554223418f, 0.0023743947967886925f, 0.0008215017733164132f, 0.0005588950007222593f, 0.0003844610182568431f, 0.0009443958988413215f, 0.0019507395336404443f, 0.0023923174012452364f, 0.0013209432363510132f, 0.0009739386150613427f, 0.002268561627715826f, 0.0005956030217930675f, 0.0008728577522560954f, 0.0006818235851824284f, 0.001185453264042735f, 0.0007202081615105271f, 0.00027480421704240143f, 0.00599231431260705f, 0.0015146903460845351f, 0.002633887343108654f, 0.0010638629319146276f, 0.0009910175576806068f, 0.0006654133903793991f, 0.0011831182055175304f, 0.0019489165861159563f, 0.002490102080628276f);
static const ai_layer_format_type conv2d_8_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_9_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_9_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_9_pad_before_t_in_0_shape_h_const_u32 = 24;

static const ai_u16 conv2d_9_t_in_0_shape_w_const_u16 = 26;
static const ai_u16 conv2d_9_t_in_0_shape_h_const_u16 = 26;
static const ai_u16 conv2d_9_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_9_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_9_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_9_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_9_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_9_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_9_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_9_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.013508240692317486f, 0.011090355925261974f, 0.00802635122090578f, 0.008949597366154194f, 0.005032342858612537f, 0.0041360631585121155f, 0.02542714774608612f, 0.005958362016826868f, 0.004890752024948597f, 0.005586664658039808f, 0.003357101697474718f, 0.006629768293350935f, 0.010337773710489273f, 0.009708060882985592f, 0.0038485489785671234f, 0.008162337355315685f, 0.008884303271770477f, 0.009537409991025925f, 0.0028188959695398808f, 0.0067856949754059315f, 0.008124764077365398f, 0.008245046250522137f, 0.006306362338364124f, 0.005973680876195431f, 0.007826984860002995f, 0.005230524577200413f, 0.026257436722517014f, 0.00405440479516983f, 0.007613898254930973f, 0.032677218317985535f, 0.004469930659979582f, 0.007473159581422806f, 0.030430005863308907f, 0.006992929615080357f, 0.007364572957158089f, 0.00606859615072608f, 0.01405875850468874f, 0.006529963109642267f, 0.011300724931061268f, 0.000875731697306037f, 0.005347954109311104f, 0.0022055679000914097f, 0.005520549602806568f, 0.005650978535413742f, 0.006540614180266857f, 0.00430343858897686f, 0.008538600988686085f, 0.004060589708387852f);
static const ai_u16 conv2d_9_t_out_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_9_t_out_0_shape_h_const_u16 = 24;

static const ai_u16 conv2d_10_t_in_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_10_t_in_0_shape_h_const_u16 = 24;
static const ai_u16 conv2d_10_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_10_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_10_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_10_t_out_0_shape_ch_const_u16 = 8;
static const ai_i8 conv2d_10_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_10_t_out_0_fmt_zero_const_s8 = -4;
static const ai_float conv2d_10_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_10_t_out_0_fmt_scale_const_f32 = 0.5001610517501831f;
static const ai_float conv2d_10_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.018520992249250412f, 0.03416561335325241f, 0.06615716964006424f, 0.06764303147792816f, 0.03389111906290054f, 0.027341404929757118f, 0.03296781703829765f, 0.04524937644600868f);
static const ai_layer_format_type conv2d_10_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_12_t_in_0_shape_w_const_u16 = 24;
static const ai_u16 conv2d_12_t_in_0_shape_h_const_u16 = 24;
static const ai_u16 conv2d_12_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_12_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_12_t_in_0_shape_ch_const_u16 = 8;
static const ai_u16 conv2d_12_t_out_0_shape_ch_const_u16 = 48;
static const ai_i8 conv2d_12_t_in_0_fmt_zero_const_s8 = 1;
static const ai_i8 conv2d_12_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_12_t_in_0_fmt_scale_const_f32 = 0.48252204060554504f;
static const ai_float conv2d_12_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_12_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0006891956436447799f, 0.001546741696074605f, 0.0005128099001012743f, 0.0005376163753680885f, 0.00044001269270665944f, 0.0004855556762777269f, 0.0008647683425806463f, 0.0004337075515650213f, 0.0006895062979310751f, 0.0014797518961131573f, 0.0017435504123568535f, 0.0005535919917747378f, 0.0007391844410449266f, 0.0012820218689739704f, 0.00026651256484910846f, 0.0007963749812915921f, 0.0005057746893726289f, 0.0006963034975342453f, 0.0004398964229039848f, 0.0005322280339896679f, 0.0006334332865662873f, 0.00043875823030248284f, 0.0006937128491699696f, 0.0002996389812324196f, 0.00053552066674456f, 0.0005509749171324074f, 0.0029521335382014513f, 0.0007193360361270607f, 0.0003825557650998235f, 0.0008186434279195964f, 0.0005213878466747701f, 0.00038796511944383383f, 0.0013571707531809807f, 0.00048077237443067133f, 0.0015106851933524013f, 0.0009749517776072025f, 0.0014020298840478063f, 0.0006281137466430664f, 0.0004401799524202943f, 0.0006599054322578013f, 0.0003177317266818136f, 0.00045298822806216776f, 0.0006045378395356238f, 0.0004912314470857382f, 0.003388816025108099f, 0.0007029440021142364f, 0.0005954458611086011f, 0.002154281595721841f);
static const ai_layer_format_type conv2d_12_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_13_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_13_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_13_pad_before_t_in_0_shape_h_const_u32 = 24;

static const ai_u16 conv2d_13_t_in_0_shape_w_const_u16 = 26;
static const ai_u16 conv2d_13_t_in_0_shape_h_const_u16 = 26;
static const ai_u16 conv2d_13_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_13_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_13_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_13_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_13_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_13_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_13_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_13_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.003352802712470293f, 0.0015321880346164107f, 0.00727679580450058f, 0.002406555460765958f, 0.005974102299660444f, 0.002835160121321678f, 0.0066683050245046616f, 0.0051594809629023075f, 0.003433921607211232f, 0.001726370770484209f, 0.0021221074275672436f, 0.004206449259072542f, 0.001803661696612835f, 0.0016243018908426166f, 0.006785002537071705f, 0.008317183703184128f, 0.007439208682626486f, 0.0033173186238855124f, 0.005642091855406761f, 0.005917993374168873f, 0.003769529750570655f, 0.007896573282778263f, 0.004505987279117107f, 0.005404331721365452f, 0.004298162646591663f, 0.00454195961356163f, 0.0009350859327241778f, 0.0027053533121943474f, 0.005036240443587303f, 0.0039693997241556644f, 0.004712109919637442f, 0.00599507475271821f, 0.0021900709252804518f, 0.0058271917514503f, 0.0012179708573967218f, 0.0023399877827614546f, 0.0043229637667536736f, 0.003983653150498867f, 0.005380181595683098f, 0.0035867441911250353f, 0.00444027641788125f, 0.0042332373559474945f, 0.0037281373515725136f, 0.0038635795935988426f, 0.0013408625964075327f, 0.007014094851911068f, 0.0043941386975348f, 0.001351169659756124f);
static const ai_u16 conv2d_13_t_out_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_13_t_out_0_shape_h_const_u16 = 12;

static const ai_u16 conv2d_14_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_14_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_14_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_14_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_14_t_in_0_shape_ch_const_u16 = 48;
static const ai_u16 conv2d_14_t_out_0_shape_ch_const_u16 = 16;
static const ai_i8 conv2d_14_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_14_t_out_0_fmt_zero_const_s8 = 20;
static const ai_float conv2d_14_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_14_t_out_0_fmt_scale_const_f32 = 0.2943727970123291f;
static const ai_float conv2d_14_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.03151122108101845f, 0.03651324287056923f, 0.048204198479652405f, 0.043731387704610825f, 0.03980230540037155f, 0.04398127272725105f, 0.05034370347857475f, 0.025586780160665512f, 0.02406741864979267f, 0.026678116992115974f, 0.03836695849895477f, 0.026469986885786057f, 0.04019840806722641f, 0.05521858111023903f, 0.025718802586197853f, 0.027281811460852623f);
static const ai_layer_format_type conv2d_14_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_15_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_15_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_15_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_15_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_15_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_15_t_out_0_shape_ch_const_u16 = 96;
static const ai_i8 conv2d_15_t_in_0_fmt_zero_const_s8 = 20;
static const ai_i8 conv2d_15_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_15_t_in_0_fmt_scale_const_f32 = 0.2943727970123291f;
static const ai_float conv2d_15_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_15_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0004728764179162681f, 0.0008669632370583713f, 0.0005063030985184014f, 0.002470332197844982f, 0.0009180589695461094f, 0.0005337905022315681f, 0.0006590557750314474f, 0.001846314175054431f, 0.001086791162379086f, 0.0003140437474939972f, 0.0006471480592153966f, 0.0017387217376381159f, 0.0013244120636954904f, 0.000407751533202827f, 0.0006853395607322454f, 0.001019586925394833f, 0.0007344571058638394f, 0.0005958097754046321f, 0.0007642462151125073f, 0.001959613524377346f, 0.0011841499945148826f, 0.0005432261968962848f, 0.00108536914922297f, 0.0007553391624242067f, 0.0006717090727761388f, 0.0012394109508022666f, 0.0008069370524026453f, 0.0006777256494387984f, 0.001970558427274227f, 0.0013102047378197312f, 0.0007084024255163968f, 0.0006361785344779491f, 0.00039292892324738204f, 0.0020115389488637447f, 0.0007043668883852661f, 0.0016188578447327018f, 0.00037216456257738173f, 0.0012136775767430663f, 0.0020831110887229443f, 0.0006112630944699049f, 0.0021500135771930218f, 0.0004896636237390339f, 0.0005568898050114512f, 0.0005654781707562506f, 0.0011758351465687156f, 0.0009188187541440129f, 0.0006841318099759519f, 0.0007351988460868597f, 0.0010885405354201794f, 0.0008320604683831334f, 0.001347302459180355f, 0.0029934332706034184f, 0.0004992639296688139f, 0.0007578232325613499f, 0.0009730218444019556f, 0.00037636313936673105f, 0.0012569268001243472f, 0.0007529895519837737f, 0.0007358959410339594f, 0.0013305683387443423f, 0.00043469356023706496f, 0.0006129315006546676f, 0.0011823342647403479f, 0.0013633634662255645f, 0.0005751358112320304f, 0.0008127573528327048f, 0.0009871459333226085f, 0.0013481219066306949f, 0.0005156896077096462f, 0.0004835447471123189f, 0.0006281259702518582f, 0.0011968311155214906f, 0.000797365908510983f, 0.0012640119530260563f, 0.0007721278234384954f, 0.00132003054022789f, 0.0003171963326167315f, 0.001061860122717917f, 0.00042316934559494257f, 0.0015550313983112574f, 0.0005943224532529712f, 0.0011456792708486319f, 0.0005652710096910596f, 0.0013036588206887245f, 0.0007327512139454484f, 0.0009251676383428276f, 0.0007880451739765704f, 0.0008363450760953128f, 0.00045564238098450005f, 0.0012750871246680617f, 0.0005371871520765126f, 0.0013828978408128023f, 0.0004706845502369106f, 0.0004108184075448662f, 0.0009829220362007618f, 0.0007360979216173291f);
static const ai_layer_format_type conv2d_15_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_16_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_16_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_16_pad_before_t_in_0_shape_h_const_u32 = 12;

static const ai_u16 conv2d_16_t_in_0_shape_w_const_u16 = 14;
static const ai_u16 conv2d_16_t_in_0_shape_h_const_u16 = 14;
static const ai_u16 conv2d_16_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_16_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_16_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_16_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_16_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_16_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_16_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_16_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.007675510831177235f, 0.004894535522907972f, 0.011772518046200275f, 0.0017391829751431942f, 0.007568170316517353f, 0.006772669963538647f, 0.012384052388370037f, 0.003648669458925724f, 0.00605191383510828f, 0.017389196902513504f, 0.0067879920825362206f, 0.014660602435469627f, 0.0038860286585986614f, 0.016749994829297066f, 0.008121456950902939f, 0.006188297178596258f, 0.004347573034465313f, 0.012583251111209393f, 0.0024540796875953674f, 0.01333777979016304f, 0.002237799344584346f, 0.007952224463224411f, 0.005265198182314634f, 0.016051916405558586f, 0.00805461872369051f, 0.01011959370225668f, 0.006732170935720205f, 0.007326913066208363f, 0.005356543231755495f, 0.004520663060247898f, 0.006297197658568621f, 0.007414822466671467f, 0.005369005724787712f, 0.0067449514754116535f, 0.009030979126691818f, 0.0074731227941811085f, 0.016696028411388397f, 0.005163194611668587f, 0.0018215542659163475f, 0.005844664294272661f, 0.01027483120560646f, 0.01413598284125328f, 0.014254805631935596f, 0.006122211925685406f, 0.008582817390561104f, 0.001974592451006174f, 0.008909773081541061f, 0.011531494557857513f, 0.003931547049432993f, 0.007964135147631168f, 0.008989794179797173f, 0.006577661260962486f, 0.010069635696709156f, 0.012012499384582043f, 0.012187927030026913f, 0.01829538494348526f, 0.012873650528490543f, 0.009026449173688889f, 0.004767565056681633f, 0.003443714464083314f, 0.01269149873405695f, 0.005936308763921261f, 0.011496231891214848f, 0.005759131629019976f, 0.008242051117122173f, 0.007528131362050772f, 0.003490957897156477f, 0.0027935761027038097f, 0.01393416803330183f, 0.010305145755410194f, 0.009451396763324738f, 0.004486724268645048f, 0.004640166647732258f, 0.005411367863416672f, 0.006941715721040964f, 0.011336845345795155f, 0.022812161594629288f, 0.002250255551189184f, 0.011348633095622063f, 0.010677034966647625f, 0.006836083717644215f, 0.01129932887852192f, 0.0045815762132406235f, 0.006857870612293482f, 0.005514933727681637f, 0.003944784868508577f, 0.00577422883361578f, 0.00845413375645876f, 0.008131644688546658f, 0.004582147113978863f, 0.008567949756979942f, 0.005144513212144375f, 0.006206835154443979f, 0.018365157768130302f, 0.11392004787921906f, 0.003274135524407029f);
static const ai_u16 conv2d_16_t_out_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_16_t_out_0_shape_h_const_u16 = 12;

static const ai_u16 conv2d_17_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_17_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_17_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_17_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_17_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_17_t_out_0_shape_ch_const_u16 = 16;
static const ai_i8 conv2d_17_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_17_t_out_0_fmt_zero_const_s8 = 10;
static const ai_float conv2d_17_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_17_t_out_0_fmt_scale_const_f32 = 0.27556443214416504f;
static const ai_float conv2d_17_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.019105684012174606f, 0.011663926765322685f, 0.011883620172739029f, 0.010008459910750389f, 0.016209332272410393f, 0.013997303321957588f, 0.01338382251560688f, 0.016166990622878075f, 0.008683268912136555f, 0.011265154927968979f, 0.015825578942894936f, 0.02904241345822811f, 0.02024584636092186f, 0.020618390291929245f, 0.01982865482568741f, 0.012746676802635193f);
static const ai_layer_format_type conv2d_17_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_19_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_19_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_19_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_19_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_19_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_19_t_out_0_shape_ch_const_u16 = 96;
static const ai_i8 conv2d_19_t_in_0_fmt_zero_const_s8 = 14;
static const ai_i8 conv2d_19_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_19_t_in_0_fmt_scale_const_f32 = 0.2676216959953308f;
static const ai_float conv2d_19_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_19_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0007288073538802564f, 0.0006765750586055219f, 0.00045547433546744287f, 0.00024795366334728897f, 0.0006032894598320127f, 0.0009887180058285594f, 0.0015110699459910393f, 0.00098827900364995f, 0.00022799741418566555f, 0.0009780173422768712f, 0.000791700673289597f, 0.0006352845230139792f, 0.000489005004055798f, 0.000638839032035321f, 0.0002241969050373882f, 0.0012074231635779142f, 0.0005727438838221133f, 0.000405836763093248f, 0.0005081937997601926f, 0.0006130767287686467f, 0.0010123471729457378f, 0.0018592752749100327f, 0.0005260008038021624f, 0.001987465424463153f, 0.0017726414371281862f, 0.0007644735160283744f, 0.0005401092930696905f, 0.0005990745266899467f, 0.0007636031950823963f, 0.00029766932129859924f, 0.00017514849605504423f, 0.00044982408871874213f, 0.0006969808600842953f, 0.001086488482542336f, 0.00064132857369259f, 0.00072118901880458f, 0.0012024891329929233f, 0.0014178636483848095f, 0.0003956290311180055f, 0.0019101750804111362f, 0.0004932289011776447f, 0.0005332418950274587f, 0.00021523107716348022f, 0.0009806851157918572f, 0.0003997310413978994f, 0.0005842431564815342f, 0.0008980989805422723f, 0.0005634794943034649f, 0.00021859657135792077f, 0.0005970502970740199f, 0.0007467274554073811f, 0.0006450603832490742f, 0.0009477504645474255f, 0.0007334260735660791f, 0.0013656807132065296f, 0.0005104737356305122f, 0.000605622073635459f, 0.0003866296319756657f, 0.0011076402151957154f, 0.0005410478916019201f, 0.0009587070089764893f, 0.0002673157723620534f, 0.0006908978684805334f, 0.0005692702252417803f, 0.0007078145863488317f, 0.0006993412971496582f, 0.0007433522259816527f, 0.0006648545386269689f, 0.0007260038983076811f, 0.0007311386289075017f, 0.000686689221765846f, 0.0005027955048717558f, 0.0003356745874043554f, 0.0005547431646846235f, 0.0010468416148796678f, 0.0008358050836250186f, 0.0007653596112504601f, 0.0010467793326824903f, 0.0009744408307597041f, 0.0010858294554054737f, 0.00042993115494027734f, 0.00041531346505507827f, 0.000483046896988526f, 0.0003952608967665583f, 0.0011086689773947f, 0.0009561597253195941f, 0.0006813963409513235f, 0.0001796116412151605f, 0.0004485814133659005f, 0.0006577958702109754f, 0.0006567177479155362f, 0.001149475108832121f, 0.0006890049553476274f, 0.0006094432319514453f, 0.0007253278745338321f, 0.00046274042688310146f);
static const ai_layer_format_type conv2d_19_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_20_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_20_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_20_pad_before_t_in_0_shape_h_const_u32 = 12;

static const ai_u16 conv2d_20_t_in_0_shape_w_const_u16 = 14;
static const ai_u16 conv2d_20_t_in_0_shape_h_const_u16 = 14;
static const ai_u16 conv2d_20_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_20_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_20_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_20_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_20_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_20_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_20_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_20_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.005487134680151939f, 0.003864634782075882f, 0.004984789993613958f, 0.009062841534614563f, 0.00983414612710476f, 0.006961171515285969f, 0.002294729230925441f, 0.004123533610254526f, 0.01018216460943222f, 0.007283613085746765f, 0.011796480044722557f, 0.005081272684037685f, 0.010955660603940487f, 0.0086629344150424f, 0.01667976938188076f, 0.007089724764227867f, 0.00898110680282116f, 0.007336339447647333f, 0.008129539899528027f, 0.003277447773143649f, 0.003461601911112666f, 0.0019989013671875f, 0.006069748662412167f, 0.0034368231426924467f, 0.0031023123301565647f, 0.004278908018022776f, 0.007691041100770235f, 0.008418399840593338f, 0.00310354121029377f, 0.019829146564006805f, 0.015797017142176628f, 0.010121037252247334f, 0.005292906425893307f, 0.007488972041755915f, 0.00945443008095026f, 0.019273381680250168f, 0.01649618335068226f, 0.010312680155038834f, 0.010398021899163723f, 0.004868768155574799f, 0.009259175509214401f, 0.008493490517139435f, 0.0144590362906456f, 0.004239535890519619f, 0.014925941824913025f, 0.004080887883901596f, 0.004953600931912661f, 0.011732409708201885f, 0.02924926020205021f, 0.004328269511461258f, 0.01777680031955242f, 0.00669539999216795f, 0.007800604682415724f, 0.007382872514426708f, 0.009230008348822594f, 0.00526805967092514f, 0.006079740822315216f, 0.01715400442481041f, 0.0018363919807597995f, 0.007539032027125359f, 0.014115806668996811f, 0.01404547318816185f, 0.005092548672109842f, 0.006686693523079157f, 0.0056474595330655575f, 0.006051500793546438f, 0.011618860997259617f, 0.011776219122111797f, 0.00795291643589735f, 0.004396701231598854f, 0.008525548502802849f, 0.004291756544262171f, 0.012126607820391655f, 0.006028397474437952f, 0.016206247732043266f, 0.013193672522902489f, 0.0052355509251356125f, 0.003573839785531163f, 0.0024481972213834524f, 0.00787422340363264f, 0.01055700983852148f, 0.009814972057938576f, 0.0074489233084023f, 0.01233111135661602f, 0.002453564666211605f, 0.005156754050403833f, 0.003305016551166773f, 0.02043393813073635f, 0.010396745055913925f, 0.005234804004430771f, 0.016069598495960236f, 0.0013660079566761851f, 0.006671222858130932f, 0.004625129513442516f, 0.015343747101724148f, 0.01160416379570961f);
static const ai_u16 conv2d_20_t_out_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_20_t_out_0_shape_h_const_u16 = 12;

static const ai_u16 conv2d_21_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_21_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_21_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_21_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_21_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_21_t_out_0_shape_ch_const_u16 = 16;
static const ai_i8 conv2d_21_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_21_t_out_0_fmt_zero_const_s8 = 8;
static const ai_float conv2d_21_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_21_t_out_0_fmt_scale_const_f32 = 0.3419592082500458f;
static const ai_float conv2d_21_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.035470928996801376f, 0.018024016171693802f, 0.009993650019168854f, 0.009313035756349564f, 0.017832057550549507f, 0.017203597351908684f, 0.015728551894426346f, 0.011893454007804394f, 0.014203717932105064f, 0.01158085372298956f, 0.031362708657979965f, 0.02506968006491661f, 0.019167114049196243f, 0.010118394158780575f, 0.022174213081598282f, 0.02400459162890911f);
static const ai_layer_format_type conv2d_21_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_23_t_in_0_shape_w_const_u16 = 12;
static const ai_u16 conv2d_23_t_in_0_shape_h_const_u16 = 12;
static const ai_u16 conv2d_23_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_23_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_23_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_23_t_out_0_shape_ch_const_u16 = 96;
static const ai_i8 conv2d_23_t_in_0_fmt_zero_const_s8 = 8;
static const ai_i8 conv2d_23_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_23_t_in_0_fmt_scale_const_f32 = 0.3419592082500458f;
static const ai_float conv2d_23_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_23_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0007583129336126149f, 0.0010996217606589198f, 0.0007107303244993091f, 0.000857255479786545f, 0.0009232622687704861f, 0.0008746072999201715f, 0.00045854912605136633f, 0.0009150476544164121f, 0.0005093796062283218f, 0.0011048561427742243f, 0.0004999871598556638f, 0.0007651139749214053f, 0.0007341380696743727f, 0.003378639230504632f, 0.0005709775723516941f, 0.00047007499961182475f, 0.00034083821810781956f, 0.000696494709700346f, 0.0013020116603001952f, 0.0007092339801602066f, 0.0003475207195151597f, 0.00023936967772897333f, 0.000404855702072382f, 0.00024264899548143148f, 0.0005136797553859651f, 0.0012544502969831228f, 0.0019083605147898197f, 0.0009466976043768227f, 0.0006380795384757221f, 0.0010551040759310126f, 0.000558942963834852f, 0.0008100866107270122f, 0.0005727456300519407f, 0.0006768816383555532f, 0.0008491943590342999f, 0.0006479209405370057f, 0.0004573545011226088f, 0.0005607083439826965f, 0.0009515749989077449f, 0.000997890718281269f, 0.00035803066566586494f, 0.0012793971691280603f, 0.0008028376032598317f, 0.0005461139953695238f, 0.0011951674241572618f, 0.0005350140272639692f, 0.0006517810979858041f, 0.0007380768656730652f, 0.0007674467633478343f, 0.0006227600970305502f, 0.0006486743222922087f, 0.0004645954177249223f, 0.001485674176365137f, 0.0006239307695068419f, 0.000895200646482408f, 0.0009165743831545115f, 0.001186817535199225f, 0.0006707760621793568f, 0.0010372002143412828f, 0.0004605232970789075f, 0.002200994174927473f, 0.0011984551092609763f, 0.0005718509200960398f, 0.00123487354721874f, 0.001416780985891819f, 0.000676519877742976f, 0.00026642211014404893f, 0.0005698362365365028f, 0.001301776384934783f, 0.0010640268446877599f, 0.0006587590905837715f, 0.0006597649771720171f, 0.0023004296235740185f, 0.0008829361177049577f, 0.0007166748982854187f, 0.000517551030497998f, 0.0007785243797115982f, 0.00027585396310314536f, 0.0003027147613465786f, 0.0004882663779426366f, 0.00038046258850954473f, 0.00046316045336425304f, 0.0005165290203876793f, 0.0004333461110945791f, 0.00032430814462713897f, 0.0008097667014226317f, 0.0013419018359854817f, 0.000496739346999675f, 0.00031570112332701683f, 0.001312992419116199f, 0.0007543665706180036f, 0.0006842075963504612f, 0.0008517360547557473f, 0.0014087731251493096f, 0.0010039323242381215f, 0.0005356105393730104f);
static const ai_layer_format_type conv2d_23_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_24_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_24_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_24_pad_before_t_in_0_shape_h_const_u32 = 12;

static const ai_u16 conv2d_24_t_in_0_shape_w_const_u16 = 14;
static const ai_u16 conv2d_24_t_in_0_shape_h_const_u16 = 14;
static const ai_u16 conv2d_24_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_24_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_24_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_24_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_24_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_24_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_24_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_24_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0024740330409258604f, 0.0016069727716967463f, 0.002921242732554674f, 0.001884845900349319f, 0.0036578169092535973f, 0.0032885584514588118f, 0.002296352991834283f, 0.001827794243581593f, 0.0026382606010884047f, 0.002780957380309701f, 0.005395918618887663f, 0.002886332105845213f, 0.0036897456739097834f, 0.0012527378275990486f, 0.002085271058604121f, 0.0049611302092671394f, 0.002802601084113121f, 0.002176978625357151f, 0.004470813553780317f, 0.0033156012650579214f, 0.005591030232608318f, 0.007579897530376911f, 0.006406087428331375f, 0.00754853431135416f, 0.004458635579794645f, 0.0017157907132059336f, 0.0008369420538656414f, 0.002104101935401559f, 0.002790433121845126f, 0.007180633023381233f, 0.002453054301440716f, 0.0036772587336599827f, 0.003189151408150792f, 0.0023798218462616205f, 0.002582005225121975f, 0.0024749659933149815f, 0.003627307014539838f, 0.0058581833727657795f, 0.002228332683444023f, 0.0015926441410556436f, 0.005629328079521656f, 0.0019162468379363418f, 0.0023780222982168198f, 0.00328410929068923f, 0.0012175955343991518f, 0.004866583272814751f, 0.0026581112761050463f, 0.0024957628920674324f, 0.0021365478169173002f, 0.004955349024385214f, 0.005510328803211451f, 0.003766934620216489f, 0.0019607774447649717f, 0.00401507830247283f, 0.0017226964700967073f, 0.001826631254516542f, 0.0024218643084168434f, 0.0026712219696491957f, 0.003457756247371435f, 0.00264487462118268f, 0.0016008615493774414f, 0.002334324410185218f, 0.0037476939614862204f, 0.0026355625595897436f, 0.004205184057354927f, 0.0024396220687776804f, 0.009524141438305378f, 0.0024792891927063465f, 0.002534290077164769f, 0.00179191492497921f, 0.0027253373991698027f, 0.0027620061300694942f, 0.0009729095036163926f, 0.0023877606727182865f, 0.0027508228085935116f, 0.0040643890388309956f, 0.004387727938592434f, 0.00953813549131155f, 0.007894759066402912f, 0.003957964479923248f, 0.005312509369105101f, 0.0032793974969536066f, 0.003115937812253833f, 0.0026097185909748077f, 0.007396560162305832f, 0.00171054364182055f, 0.0016664821887388825f, 0.00368979899212718f, 0.005188638810068369f, 0.0010784742189571261f, 0.002287291456013918f, 0.007892890833318233f, 0.0033690566197037697f, 0.0025963918305933475f, 0.00318911368958652f, 0.0030998315196484327f);
static const ai_u16 conv2d_24_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_24_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_25_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_25_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_25_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_25_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_25_t_in_0_shape_ch_const_u16 = 96;
static const ai_u16 conv2d_25_t_out_0_shape_ch_const_u16 = 24;
static const ai_i8 conv2d_25_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_25_t_out_0_fmt_zero_const_s8 = -12;
static const ai_float conv2d_25_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_25_t_out_0_fmt_scale_const_f32 = 0.25301802158355713f;
static const ai_float conv2d_25_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.01697898469865322f, 0.01642937958240509f, 0.021923452615737915f, 0.022389018908143044f, 0.018476614728569984f, 0.013705160468816757f, 0.018800746649503708f, 0.024413933977484703f, 0.02056027017533779f, 0.010773087851703167f, 0.016263868659734726f, 0.019040187820792198f, 0.019173068925738335f, 0.013877687975764275f, 0.02156938426196575f, 0.018701566383242607f, 0.01988391764461994f, 0.017239348962903023f, 0.012527058832347393f, 0.020287852734327316f, 0.023916438221931458f, 0.016288738697767258f, 0.0167124904692173f, 0.024221064522862434f);
static const ai_layer_format_type conv2d_25_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_26_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_26_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_26_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_26_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_26_t_in_0_shape_ch_const_u16 = 24;
static const ai_u16 conv2d_26_t_out_0_shape_ch_const_u16 = 144;
static const ai_i8 conv2d_26_t_in_0_fmt_zero_const_s8 = -12;
static const ai_i8 conv2d_26_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_26_t_in_0_fmt_scale_const_f32 = 0.25301802158355713f;
static const ai_float conv2d_26_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_26_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0006038288120180368f, 0.001835122937336564f, 0.0003771467600017786f, 0.0005287527455948293f, 0.0008553970255888999f, 0.0005968823679722846f, 0.0006298993248492479f, 0.0009747629519551992f, 0.00026019939105026424f, 0.0006629979470744729f, 0.0005016959039494395f, 0.0008827537531033158f, 0.0007355574634857476f, 0.0004347275535110384f, 0.0014178585261106491f, 0.0003827873442787677f, 0.0005781687214039266f, 0.0006292589823715389f, 0.0006212212610989809f, 0.0007400311878882349f, 0.0005262488266453147f, 0.0011530418414622545f, 0.0007848710520192981f, 0.0008188288775272667f, 0.0005797459743916988f, 0.0006744178826920688f, 0.0003157851460855454f, 0.0005791497533209622f, 0.0007320218719542027f, 0.00035955538623966277f, 0.0012206498067826033f, 0.001338439411483705f, 0.0008149605127982795f, 0.00039224245119839907f, 0.0007372519467025995f, 0.0006056647398509085f, 0.00032958583324216306f, 0.0005576784024015069f, 0.00034847823553718626f, 0.0004539984802249819f, 0.0005334390443749726f, 0.0007230337942019105f, 0.0006016191327944398f, 0.00030857676756568253f, 0.001351254410110414f, 0.0005639562732540071f, 0.0005422048852778971f, 0.0003149219264741987f, 0.0005378320347517729f, 0.0008204634068533778f, 0.0005355377215892076f, 0.0006179483607411385f, 0.0005368288839235902f, 0.001034929184243083f, 0.00041912205051630735f, 0.0014392024604603648f, 0.0008557143737562001f, 0.00026959393289871514f, 0.00029502715915441513f, 0.00028937263414263725f, 0.002284745220094919f, 0.0008064466528594494f, 0.0006472755339927971f, 0.0005826426204293966f, 0.0005407368298619986f, 0.0004902580985799432f, 0.0006882973248139024f, 0.0005510167684406042f, 0.00024290356668643653f, 0.001008407212793827f, 0.00045977678382769227f, 0.0015264061512425542f, 0.00048134129610843956f, 0.0004976544296368957f, 0.0006009304197505116f, 0.000620451697614044f, 0.00044968092697672546f, 0.00043241409002803266f, 0.0006452653324231505f, 0.0005290064145810902f, 0.0007862200727686286f, 0.0008249352686107159f, 0.0006763218552805483f, 0.0009750031749717891f, 0.000416259397752583f, 0.0005725790979340672f, 0.0006529446691274643f, 0.00044222825090400875f, 0.0012597056338563561f, 0.0007906275568529963f, 0.002269625198096037f, 0.0009698601788841188f, 0.0005190121009945869f, 0.0005214014672674239f, 0.00037121461355127394f, 0.00013097989722155035f, 0.0007093531312420964f, 0.0005549220950342715f, 0.0006145347142592072f, 0.0006189402192831039f, 0.00061779998941347f, 0.0008503777789883316f, 0.0004545547708403319f, 0.0007423718925565481f, 0.0005174973630346358f, 0.000768922851420939f, 0.0016782486345618963f, 0.0004532713792286813f, 0.0007988554425537586f, 0.0007507281843572855f, 0.0012895938707515597f, 0.0009233497548848391f, 0.0008114996016956866f, 0.0009666074765846133f, 0.0006482666358351707f, 0.00024736500927247107f, 0.0005108627374283969f, 0.0005671202088706195f, 0.0006877499399706721f, 0.0008595949038863182f, 0.00044074770994484425f, 0.0013665087753906846f, 0.0007955356268212199f, 0.0008137256954796612f, 0.0007208187016658485f, 0.0005015693022869527f, 0.0006268827710300684f, 0.0013797642895951867f, 0.0023035539779812098f, 0.0008278213790617883f, 0.0009447316988371313f, 0.0005302085191942751f, 0.0004779154260177165f, 0.0007970918668434024f, 0.00048680079635232687f, 0.0005576998228207231f, 0.0005221093888394535f, 0.0011307214153930545f, 0.0006540108588524163f, 0.0013442328199744225f, 0.0003819804114755243f, 0.000832313031423837f, 0.00032079749507829547f, 0.0005955136730335653f);
static const ai_layer_format_type conv2d_26_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_27_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_27_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_27_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_27_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_27_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_27_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_27_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_27_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_27_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_27_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_27_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_27_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_27_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.009750316850841045f, 0.0025677261874079704f, 0.015494633466005325f, 0.006490285042673349f, 0.014941072091460228f, 0.007720919791609049f, 0.009722087532281876f, 0.0011591301299631596f, 0.01638946868479252f, 0.01864449307322502f, 0.012944292277097702f, 0.007448940072208643f, 0.01763816550374031f, 0.023724349215626717f, 0.0062343706376850605f, 0.011030510999262333f, 0.01435779221355915f, 0.008110545575618744f, 0.009241594932973385f, 0.012905015610158443f, 0.005403685849159956f, 0.01105248648673296f, 0.014343502931296825f, 0.003726034425199032f, 0.007044261787086725f, 0.008908015675842762f, 0.02046187035739422f, 0.008048591203987598f, 0.01380949467420578f, 0.00806418713182211f, 0.010699220933020115f, 0.022238248959183693f, 0.016323382034897804f, 0.016593530774116516f, 0.007433074526488781f, 0.01706617884337902f, 0.01179467886686325f, 0.011597790755331516f, 0.012215866707265377f, 0.011912226676940918f, 0.006904685404151678f, 0.00914918165653944f, 0.010686349123716354f, 0.012568965554237366f, 0.015661554411053658f, 0.007029333151876926f, 0.008621059358119965f, 0.009500963613390923f, 0.016275841742753983f, 0.010185553692281246f, 0.008771301247179508f, 0.006841204594820738f, 0.012294689193367958f, 0.003205368062481284f, 0.01238559652119875f, 0.0025485679507255554f, 0.024726590141654015f, 0.013892915099859238f, 0.015888720750808716f, 0.03535569831728935f, 0.00395884457975626f, 0.010067921131849289f, 0.008923247456550598f, 0.00846290122717619f, 0.00458059785887599f, 0.01965062879025936f, 0.010627951472997665f, 0.017961421981453896f, 0.01641453243792057f, 0.004222342744469643f, 0.00798060279339552f, 0.010533583350479603f, 0.012345789931714535f, 0.016664408147335052f, 0.01286058034747839f, 0.009513896889984608f, 0.009736892767250538f, 0.020649215206503868f, 0.0052773719653487206f, 0.008564361371099949f, 0.007876355201005936f, 0.007837994024157524f, 0.009054617956280708f, 0.015670420601963997f, 0.01112121157348156f, 0.009665134362876415f, 0.004374166950583458f, 0.005651999730616808f, 0.00972159393131733f, 0.005562160164117813f, 0.0028639656957238913f, 0.0033498990815132856f, 0.010388722643256187f, 0.009005105122923851f, 0.016425788402557373f, 0.05439748987555504f, 0.007905171252787113f, 0.01645049825310707f, 0.010480829514563084f, 0.015612903982400894f, 0.010759741999208927f, 0.013372375629842281f, 0.00836480688303709f, 0.008255255408585072f, 0.004155515693128109f, 0.009443426504731178f, 0.002892640884965658f, 0.014814391732215881f, 0.006607873365283012f, 0.005625339690595865f, 0.011357770301401615f, 0.022486867383122444f, 0.009012872353196144f, 0.0038824507500976324f, 0.006789783481508493f, 0.013518503867089748f, 0.01071695052087307f, 0.014820361509919167f, 0.007628890685737133f, 0.010053840465843678f, 0.019537661224603653f, 0.006700377911329269f, 0.008996340446174145f, 0.009186699986457825f, 0.009884791448712349f, 0.016949232667684555f, 0.019040385261178017f, 0.003289648564532399f, 0.005007272586226463f, 0.004631573334336281f, 0.004352974239736795f, 0.01172921247780323f, 0.007977304048836231f, 0.006252921652048826f, 0.009856819175183773f, 0.017017651349306107f, 0.0030060717836022377f, 0.011068060994148254f, 0.019434452056884766f, 0.01636687107384205f, 0.016947196796536446f, 0.00954277254641056f, 0.01285947673022747f, 0.006416617892682552f);
static const ai_u16 conv2d_27_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_27_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_28_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_28_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_28_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_28_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_28_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_28_t_out_0_shape_ch_const_u16 = 24;
static const ai_i8 conv2d_28_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_28_t_out_0_fmt_zero_const_s8 = -7;
static const ai_float conv2d_28_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_28_t_out_0_fmt_scale_const_f32 = 0.26992267370224f;
static const ai_float conv2d_28_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.010672490112483501f, 0.020433949306607246f, 0.006406000815331936f, 0.005924483761191368f, 0.0062991357408463955f, 0.004419335629791021f, 0.005386918317526579f, 0.009094391018152237f, 0.004728293977677822f, 0.021155592054128647f, 0.007889353670179844f, 0.003630181308835745f, 0.0036054484080523252f, 0.0037178369238972664f, 0.005319204181432724f, 0.004253870341926813f, 0.01028247456997633f, 0.007643591612577438f, 0.012827579863369465f, 0.004929892253130674f, 0.003855785122141242f, 0.00476417550817132f, 0.013610705733299255f, 0.005241783801466227f);
static const ai_layer_format_type conv2d_28_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_30_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_30_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_30_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_30_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_30_t_in_0_shape_ch_const_u16 = 24;
static const ai_u16 conv2d_30_t_out_0_shape_ch_const_u16 = 144;
static const ai_i8 conv2d_30_t_in_0_fmt_zero_const_s8 = -7;
static const ai_i8 conv2d_30_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_30_t_in_0_fmt_scale_const_f32 = 0.26992267370224f;
static const ai_float conv2d_30_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_30_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0009339344687759876f, 0.0003974858846049756f, 0.0006232002633623779f, 0.00043777693645097315f, 0.00032259555882774293f, 0.0009881863370537758f, 0.0005714788567274809f, 0.0007220653933472931f, 0.0005114819505251944f, 0.0006589184631593525f, 0.0007853467832319438f, 0.0005971869104541838f, 0.0006463967729359865f, 0.0008768744883127511f, 0.0002672064583748579f, 0.00033325591357424855f, 0.0009574893047101796f, 0.001047624391503632f, 0.0005272451671771705f, 0.0005334782181307673f, 0.0004991134046576917f, 0.00032382551580667496f, 0.0004399078316055238f, 0.0007451230776496232f, 0.0007855044095776975f, 0.00040309628820978105f, 0.0009084290941245854f, 0.0008525785524398088f, 0.0005311587010510266f, 0.0007368286605924368f, 0.00041364572825841606f, 0.0003091592516284436f, 0.0005017311777919531f, 0.000756921130232513f, 0.00024230044800788164f, 0.00023158763360697776f, 0.0005569584318436682f, 0.0005293902941048145f, 0.00033079253626056015f, 0.0004950009752064943f, 0.0007973304018378258f, 0.0006374070071615279f, 0.0006400173297151923f, 0.0010906242532655597f, 0.0010029469849541783f, 0.00037789507769048214f, 0.0006514682318083942f, 0.00048397373757325113f, 0.0005114589584991336f, 0.0005981012363918126f, 0.0003643619129434228f, 0.000570614414755255f, 0.0008722824859432876f, 0.0007662606658414006f, 0.0002673489216249436f, 0.000519102846737951f, 0.0003637118497863412f, 0.0004713597008958459f, 0.0009405008750036359f, 0.00018237113545183092f, 0.0003610379935707897f, 0.0003178099577780813f, 0.0003347986494190991f, 0.0007640859112143517f, 0.0007980019436217844f, 0.0004279094282537699f, 0.00016314249660354108f, 0.00044340279418975115f, 0.0003099163877777755f, 0.00042507442412897944f, 0.0010987314162775874f, 0.0003914434346370399f, 0.00032869295682758093f, 0.0005409291479736567f, 0.001122374553233385f, 0.0003396000247448683f, 0.0006176462629809976f, 0.0007180966786108911f, 0.0007511894800700247f, 0.00038893771125003695f, 0.0004755942209158093f, 0.0006429410423152149f, 0.0011052667396143079f, 0.0005358890048228204f, 0.0003769925970118493f, 0.000672691036015749f, 0.0008065568399615586f, 0.0010356465354561806f, 0.00030387501465156674f, 0.0004989614826627076f, 0.0007411256665363908f, 0.0008276752196252346f, 0.0005005842540413141f, 0.0003755817306227982f, 0.00034884901833720505f, 0.0004846299416385591f, 0.0006892952951602638f, 0.0005091686616651714f, 0.0004872585996054113f, 0.00038024981040507555f, 0.0003169943520333618f, 0.0004910543211735785f, 0.0007152716862037778f, 0.0003315703943371773f, 0.0004682331345975399f, 0.00026023530517704785f, 0.0005377397174015641f, 0.000999901443719864f, 0.001103467890061438f, 0.0009316967916674912f, 0.0004501546500250697f, 0.0006203720113262534f, 0.00038956693606451154f, 0.00039073106017895043f, 0.0007316154078580439f, 0.00026714245905168355f, 0.0005027351435273886f, 0.0011933664791285992f, 0.0005385542172007263f, 0.0004133068141527474f, 0.00023200479336082935f, 0.00016535696340724826f, 0.0007803423213772476f, 0.0005850069574080408f, 0.0004546793061308563f, 0.0004570376477204263f, 0.0007025134400464594f, 0.0008466544095426798f, 0.0010192112531512976f, 0.00018030292994808406f, 0.0009194659069180489f, 0.0005153673118911684f, 0.0005622530588880181f, 0.0005660486058332026f, 0.0002801857772283256f, 0.0006952730473130941f, 0.0005432440084405243f, 0.00038387099630199373f, 0.0007274095551110804f, 0.0008356604957953095f, 0.0005368785350583494f, 0.0008167593623511493f, 0.0011033479822799563f, 0.00030832496122457087f);
static const ai_layer_format_type conv2d_30_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_31_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_31_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_31_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_31_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_31_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_31_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_31_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_31_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_31_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_31_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_31_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_31_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_31_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.003916901536285877f, 0.007712789345532656f, 0.008580847643315792f, 0.009469223208725452f, 0.008853263221681118f, 0.004145586397498846f, 0.009394309483468533f, 0.007052290253341198f, 0.0029662763699889183f, 0.01456443965435028f, 0.004379799589514732f, 0.0024605444632470608f, 0.0033327906858175993f, 0.005501285661011934f, 0.015144076198339462f, 0.02261980250477791f, 0.0033323580864816904f, 0.007202710956335068f, 0.015245935879647732f, 0.008218970149755478f, 0.02163141965866089f, 0.012969285249710083f, 0.02028466947376728f, 0.007368267513811588f, 0.009093365631997585f, 0.011579654179513454f, 0.0039809588342905045f, 0.007940994575619698f, 0.009070965461432934f, 0.0017792246071621776f, 0.012893331237137318f, 0.01277993619441986f, 0.005780593026429415f, 0.01510198786854744f, 0.016048377379775047f, 0.004094607196748257f, 0.007457823026925325f, 0.011269682087004185f, 0.014111137017607689f, 0.007679870817810297f, 0.005937924142926931f, 0.005503036547452211f, 0.00396596547216177f, 0.00940177496522665f, 0.0012548989616334438f, 0.005010219756513834f, 0.008932815864682198f, 0.013193691149353981f, 0.008294573985040188f, 0.01097896322607994f, 0.004914786200970411f, 0.0037283485289663076f, 0.007367673330008984f, 0.011018182151019573f, 0.018980691209435463f, 0.008457088842988014f, 0.010206029750406742f, 0.006764691788703203f, 0.002101026475429535f, 0.01951448991894722f, 0.012486375868320465f, 0.017287801951169968f, 0.008077736012637615f, 0.007079691626131535f, 0.0043909307569265366f, 0.010362936183810234f, 0.007021444849669933f, 0.0074198427610099316f, 0.0360647551715374f, 0.005211751442402601f, 0.005462322384119034f, 0.013279949314892292f, 0.016597874462604523f, 0.008399983868002892f, 0.007610293570905924f, 0.014417590573430061f, 0.0062268562614917755f, 0.01825072057545185f, 0.009973064064979553f, 0.007364471908658743f, 0.004358899779617786f, 0.013632496818900108f, 0.0049480777233839035f, 0.016585340723395348f, 0.016034875065088272f, 0.004628978669643402f, 0.0057410565204918385f, 0.013786057941615582f, 0.01425163820385933f, 0.009244458749890327f, 0.01845558173954487f, 0.009622587822377682f, 0.007361513562500477f, 0.005621056072413921f, 0.022838277742266655f, 0.006105801090598106f, 0.012100963853299618f, 0.009527322836220264f, 0.01575596071779728f, 0.019534185528755188f, 0.00865352712571621f, 0.007121720816940069f, 0.01112268678843975f, 0.015136105008423328f, 0.01911160536110401f, 0.01720385253429413f, 0.00808396190404892f, 0.008750020526349545f, 0.008447024039924145f, 0.008183404803276062f, 0.005533572286367416f, 0.005714260507375002f, 0.007672037463635206f, 0.011235405690968037f, 0.012457077391445637f, 0.02583598718047142f, 0.008654073812067509f, 0.006109153851866722f, 0.009251762181520462f, 0.0129772974178195f, 0.020724404603242874f, 0.02908020094037056f, 0.006086140405386686f, 0.008018122054636478f, 0.0055319443345069885f, 0.010506870225071907f, 0.006460089702159166f, 0.007717172149568796f, 0.009585043415427208f, 0.01919219270348549f, 0.0074727279134094715f, 0.01099787000566721f, 0.015367777086794376f, 0.010111072100698948f, 0.012405975721776485f, 0.004056978039443493f, 0.006969389971345663f, 0.009186635725200176f, 0.012652728706598282f, 0.01530700083822012f, 0.008415032178163528f, 0.005712426733225584f, 0.00463198171928525f, 0.019878463819622993f);
static const ai_u16 conv2d_31_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_31_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_32_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_32_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_32_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_32_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_32_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_32_t_out_0_shape_ch_const_u16 = 24;
static const ai_i8 conv2d_32_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_32_t_out_0_fmt_zero_const_s8 = 0;
static const ai_float conv2d_32_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_32_t_out_0_fmt_scale_const_f32 = 0.27030789852142334f;
static const ai_float conv2d_32_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.012444213964045048f, 0.01710882969200611f, 0.004636131227016449f, 0.007364918012171984f, 0.0063006691634655f, 0.005956957582384348f, 0.00427525257691741f, 0.0073442403227090836f, 0.005400306545197964f, 0.0363607183098793f, 0.008060166612267494f, 0.0049956124275922775f, 0.005255335476249456f, 0.00595398573204875f, 0.007791544310748577f, 0.0062292939983308315f, 0.005590442568063736f, 0.007026020437479019f, 0.02895507402718067f, 0.007975981570780277f, 0.006402856670320034f, 0.010685767978429794f, 0.013162591494619846f, 0.008388878777623177f);
static const ai_layer_format_type conv2d_32_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_34_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_34_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_34_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_34_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_34_t_in_0_shape_ch_const_u16 = 24;
static const ai_u16 conv2d_34_t_out_0_shape_ch_const_u16 = 144;
static const ai_i8 conv2d_34_t_in_0_fmt_zero_const_s8 = 0;
static const ai_i8 conv2d_34_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_34_t_in_0_fmt_scale_const_f32 = 0.27030789852142334f;
static const ai_float conv2d_34_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_34_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0004547183052636683f, 0.0006770940963178873f, 0.0005404258263297379f, 0.000424100348027423f, 0.0008173948153853416f, 0.00025865857605822384f, 0.00048651473480276763f, 0.00048790962318889797f, 0.0004456205933820456f, 0.0007027728715911508f, 0.0005774158053100109f, 0.0003998999309260398f, 0.0004235394299030304f, 0.0006467531784437597f, 0.000901418155990541f, 0.0006909518851898611f, 0.0006807437748648226f, 0.0018689039861783385f, 0.0006323538837023079f, 0.0006949130329303443f, 0.0003832589718513191f, 0.0005342602380551398f, 0.0004164500569459051f, 0.0009089752566069365f, 0.00043537223245948553f, 0.000230951642151922f, 0.0012473464012145996f, 0.0005599413998425007f, 0.0006926823407411575f, 0.00015023168816696852f, 0.0007648415630683303f, 0.00037622934905812144f, 0.000691103923600167f, 0.0010651140473783016f, 0.0009573957649990916f, 0.0005859002121724188f, 0.0003221496008336544f, 0.0003914930275641382f, 0.0007636223454028368f, 0.001072471495717764f, 0.00020817050244659185f, 6.633203156525269e-05f, 0.0004934587050229311f, 0.0008529964252375066f, 0.0007194687495939434f, 0.0005966922035440803f, 0.0008576987311244011f, 0.0006182857323437929f, 0.0005194804980419576f, 0.0008159923017956316f, 0.0004528742574620992f, 0.00028139789355918765f, 0.00035710426163859665f, 0.000982841127552092f, 0.0007845661602914333f, 0.0003339398535899818f, 0.0009025969193316996f, 0.0008069335017353296f, 0.0015243212692439556f, 0.000757581670768559f, 0.00032882814412005246f, 0.0005665957578457892f, 0.000615809578448534f, 0.0006012699450366199f, 0.0007933643064461648f, 0.0005719175678677857f, 0.000649997207801789f, 0.0005813445313833654f, 0.0011480143293738365f, 0.0005383208626881242f, 0.0006064175977371633f, 0.00042495696106925607f, 0.0004015225567854941f, 0.0006474546389654279f, 0.0007023060461506248f, 0.0006501810858026147f, 0.0007632546476088464f, 0.0006113564013503492f, 0.0008680649916641414f, 0.0006571426056325436f, 0.0005222225445322692f, 0.00040544845978729427f, 0.0005097867106087506f, 0.0007586874417029321f, 0.0004868315299972892f, 0.0010930349817499518f, 0.0005572060472331941f, 0.0006517627043649554f, 0.0005650102393701673f, 0.00047281206934712827f, 0.0005536142853088677f, 0.0003811973729170859f, 0.000839388812892139f, 0.0009723035036586225f, 0.0004173691268078983f, 0.0007905131787993014f, 0.0014713373966515064f, 0.0008899196982383728f, 0.0008647044887766242f, 0.0006011375808157027f, 0.000264118833001703f, 0.0005849661538377404f, 0.0007222695276141167f, 0.00027221732307225466f, 0.00029485856066457927f, 0.0006443587481044233f, 0.0007232281495817006f, 0.00022085069213062525f, 0.0004690336063504219f, 0.0006534990388900042f, 0.0008983866428025067f, 0.000805756775662303f, 0.0005064535071142018f, 0.0008911601617000997f, 0.0009963767370209098f, 0.0007609627209603786f, 0.0004767506034113467f, 0.0004272106452845037f, 0.0006859198329038918f, 0.0007527071284130216f, 0.0005497311940416694f, 0.0004890150157734752f, 0.0008099835249595344f, 0.0005823447718285024f, 0.0003144386864732951f, 0.0005009426386095583f, 0.00018944904149975628f, 0.0005963018629699945f, 0.0005075648659840226f, 0.0008814986213110387f, 0.0005850003799423575f, 0.0005929601029492915f, 0.0006548888050019741f, 0.0007141280802898109f, 0.0012839046539738774f, 0.0005700399633497f, 0.00038998774834908545f, 0.0010395009303465486f, 0.0010785748017951846f, 0.00019074766896665096f, 0.0004425077931955457f, 0.0005496519734151661f, 0.0005839015357196331f, 0.0005329149425961077f);
static const ai_layer_format_type conv2d_34_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_35_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_35_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_35_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_35_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_35_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_35_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_35_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_35_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_35_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_35_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_35_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_35_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_35_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.011277416720986366f, 0.007962881587445736f, 0.011733082123100758f, 0.0038514097686856985f, 0.010541487485170364f, 0.02548799477517605f, 0.007895248010754585f, 0.015484495088458061f, 0.010959742590785027f, 0.002449797233566642f, 0.013584050349891186f, 0.011900180019438267f, 0.013046840205788612f, 0.010558766312897205f, 0.011862283572554588f, 0.009095408022403717f, 0.008156866766512394f, 0.0024585614446550608f, 0.00471902173012495f, 0.006444268394261599f, 0.00885104015469551f, 0.003263151738792658f, 0.006366767454892397f, 0.0072700525633990765f, 0.0036447891034185886f, 0.02480657584965229f, 0.0027001160196959972f, 0.02259606495499611f, 0.008730539120733738f, 0.02270556055009365f, 0.008060966618359089f, 0.019851794466376305f, 0.03518297150731087f, 0.008784661069512367f, 0.014256310649216175f, 0.004261049907654524f, 0.01612120307981968f, 0.009895192459225655f, 0.010985531844198704f, 0.004618968814611435f, 0.020469320937991142f, 0.10821682214736938f, 0.01785227470099926f, 0.01288128923624754f, 0.007877731695771217f, 0.004322556778788567f, 0.012198629789054394f, 0.007156898267567158f, 0.009302948601543903f, 0.00820994097739458f, 0.0050468603149056435f, 0.019508302211761475f, 0.01417382713407278f, 0.022180939093232155f, 0.009290006943047047f, 0.012151849456131458f, 0.006527335848659277f, 0.0019314195960760117f, 0.004567280411720276f, 0.00611708452925086f, 0.02390027418732643f, 0.003730833763256669f, 0.011717884801328182f, 0.007567497435957193f, 0.006137196905910969f, 0.006884305272251368f, 0.008677741512656212f, 0.010856855660676956f, 0.004964766092598438f, 0.008129322901368141f, 0.02144145779311657f, 0.007640061900019646f, 0.008321530185639858f, 0.013696219772100449f, 0.0069615719839930534f, 0.006785405799746513f, 0.007992235943675041f, 0.01738511025905609f, 0.004952538758516312f, 0.0027415205258876085f, 0.015924649313092232f, 0.010769291780889034f, 0.008020973764359951f, 0.013264293782413006f, 0.004977245349436998f, 0.0035987133160233498f, 0.0045272065326571465f, 0.005651205312460661f, 0.012229383923113346f, 0.007641049101948738f, 0.022363759577274323f, 0.007366941776126623f, 0.006114894524216652f, 0.0033085145987570286f, 0.008253841660916805f, 0.006363028660416603f, 0.010739178396761417f, 0.013993939384818077f, 0.02167135663330555f, 0.007646556943655014f, 0.01475672796368599f, 0.00849191751331091f, 0.0026283487677574158f, 0.01800842024385929f, 0.006552973762154579f, 0.006507664453238249f, 0.0049669197760522366f, 0.011939155869185925f, 0.005666786339133978f, 0.0068388874642550945f, 0.0037881536409258842f, 0.010063299909234047f, 0.002975223585963249f, 0.006216595880687237f, 0.0056912279687821865f, 0.005477247294038534f, 0.004244484473019838f, 0.017699366435408592f, 0.0039980169385671616f, 0.007694073487073183f, 0.012396039441227913f, 0.017735572531819344f, 0.006614477373659611f, 0.006786347832530737f, 0.014278349466621876f, 0.004060918465256691f, 0.011952060274779797f, 0.005772444419562817f, 0.0074408818036317825f, 0.004150792025029659f, 0.003241980914026499f, 0.003958282992243767f, 0.008777609094977379f, 0.0054639242589473724f, 0.010148223489522934f, 0.010190965607762337f, 0.011769692413508892f, 0.003878229996189475f, 0.003723511938005686f, 0.02395414561033249f, 0.0070866369642317295f, 0.012183206155896187f, 0.0071031018160283566f, 0.015598616562783718f);
static const ai_u16 conv2d_35_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_35_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_36_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_36_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_36_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_36_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_36_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_36_t_out_0_shape_ch_const_u16 = 24;
static const ai_i8 conv2d_36_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_36_t_out_0_fmt_zero_const_s8 = 19;
static const ai_float conv2d_36_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_36_t_out_0_fmt_scale_const_f32 = 0.3205251395702362f;
static const ai_float conv2d_36_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.006489901803433895f, 0.018333854153752327f, 0.005648056045174599f, 0.006957442499697208f, 0.005558012519031763f, 0.005579558201134205f, 0.004523839335888624f, 0.005121980793774128f, 0.007495270110666752f, 0.023400528356432915f, 0.007219892926514149f, 0.005379209294915199f, 0.005666147917509079f, 0.005916021764278412f, 0.004316434264183044f, 0.005469371564686298f, 0.005731774494051933f, 0.005490840412676334f, 0.041190747171640396f, 0.0058919331058859825f, 0.005167905706912279f, 0.005836696829646826f, 0.02297246642410755f, 0.006305191665887833f);
static const ai_layer_format_type conv2d_36_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_38_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_38_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_38_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_38_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_38_t_in_0_shape_ch_const_u16 = 24;
static const ai_u16 conv2d_38_t_out_0_shape_ch_const_u16 = 144;
static const ai_i8 conv2d_38_t_in_0_fmt_zero_const_s8 = 19;
static const ai_i8 conv2d_38_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_38_t_in_0_fmt_scale_const_f32 = 0.3205251395702362f;
static const ai_float conv2d_38_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_38_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0007192282355390489f, 0.0006863282178528607f, 0.00047501848894171417f, 0.0007453057332895696f, 0.0007113569299690425f, 0.0006185777019709349f, 0.00043968961108475924f, 0.0005077756359241903f, 0.0019850246608257294f, 0.001145017216913402f, 0.000746099220123142f, 0.0007478190818801522f, 0.0002703422214835882f, 0.0009247337584383786f, 0.0003229677095077932f, 0.00047830655239522457f, 0.0003425328468438238f, 0.0008998810080811381f, 0.0007459715707227588f, 0.0008379661594517529f, 0.0009459736756980419f, 0.0004202102718409151f, 0.000599734194111079f, 0.0009053791873157024f, 0.0009596179588697851f, 0.0007066830876283348f, 0.00021514436230063438f, 0.00047131950850598514f, 0.0007078854250721633f, 0.000836414925288409f, 0.0006187427206896245f, 0.0007574299233965576f, 0.0006714742630720139f, 0.0005327799008227885f, 0.0004812848928850144f, 0.0008652678225189447f, 0.0007536000339314342f, 0.0007145779673010111f, 0.00040011797682382166f, 0.0007425638614222407f, 0.0007045280653983355f, 0.0006738832453265786f, 0.0009179477929137647f, 0.0005221285973675549f, 0.0005587858613580465f, 0.0012288768775761127f, 0.0006115042488090694f, 0.000651189882773906f, 0.00114330998621881f, 0.000604353379458189f, 0.0009216725593432784f, 0.0008755860617384315f, 0.0010002620983868837f, 0.0004577295621857047f, 0.0007225382141768932f, 0.0005805984837934375f, 0.0007771510863676667f, 0.001114915474317968f, 0.0007830418762750924f, 0.0009684266988188028f, 0.0010532484157010913f, 0.000730165047571063f, 0.0007826433866284788f, 0.0007553264731541276f, 0.0008644201443530619f, 0.0007008312386460602f, 0.0005516207893379033f, 0.00047077532508410513f, 0.0006839589332230389f, 0.00037396838888525963f, 0.000606750778388232f, 0.0006410650676116347f, 0.0010781600140035152f, 0.0009625185048207641f, 0.0008157015545293689f, 0.0007062283693812788f, 0.0007694661035202444f, 0.0005117437103763223f, 0.0008371614385396242f, 0.0006189191481098533f, 0.00024729210417717695f, 0.0006231313454918563f, 0.0003209527349099517f, 0.0004186579317320138f, 0.000998332048766315f, 0.00143740966450423f, 0.0006964984349906445f, 0.001342951669357717f, 0.000721544143743813f, 0.00038238544948399067f, 0.001220833626575768f, 0.000896945595741272f, 0.0005201241001486778f, 0.00042697874596342444f, 0.00043565110536292195f, 0.0009425312746316195f, 0.0006069241208024323f, 0.00037129028351046145f, 0.0004742485180031508f, 0.0004724299651570618f, 0.00107076286803931f, 0.0007714703097008169f, 0.00024832107010297477f, 0.0005663778283633292f, 0.0004913364537060261f, 0.0004844407085329294f, 0.0004221355775371194f, 0.0007969485595822334f, 0.0005330413696356118f, 0.0005185148329474032f, 0.000911576091311872f, 0.0006484079058282077f, 0.0005347567494027317f, 0.0005296305171214044f, 0.0007662366260774434f, 0.000910076079890132f, 0.0010815788991749287f, 0.0003966983640566468f, 0.000593260454479605f, 0.000741015886887908f, 0.0004274933598935604f, 0.0006038901628926396f, 0.0007164016715250909f, 0.0005842465325258672f, 0.0011149412021040916f, 0.0009394166409038007f, 0.0005469564930535853f, 0.0005502952262759209f, 0.0014447616413235664f, 0.0007750546792522073f, 0.0005729377153329551f, 0.0005775325698778033f, 0.0008473618654534221f, 0.0007319706492125988f, 0.0008902066037990153f, 0.0009229672141373158f, 0.00085790129378438f, 0.0008052516495808959f, 0.0007414675201289356f, 0.0003851178626064211f, 0.0007163583650253713f, 0.0005507015739567578f, 0.0006030195509083569f, 0.00037088451790623367f);
static const ai_layer_format_type conv2d_38_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_39_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_39_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_39_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_39_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_39_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_39_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_39_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_39_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_39_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_39_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_39_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_39_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_39_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.006366346031427383f, 0.004140995908528566f, 0.00607386464253068f, 0.007504812441766262f, 0.00358308176510036f, 0.007353939116001129f, 0.014076096005737782f, 0.0021108861546963453f, 0.0033230490516871214f, 0.005223189480602741f, 0.004512353800237179f, 0.0029013934545218945f, 0.011521847918629646f, 0.005119762383401394f, 0.00850839726626873f, 0.003920283168554306f, 0.013483978807926178f, 0.00587120046839118f, 0.005241069942712784f, 0.005827543791383505f, 0.006337272003293037f, 0.007380849681794643f, 0.004731331020593643f, 0.0069304462522268295f, 0.005155858118087053f, 0.0020289430394768715f, 0.0045206621289253235f, 0.006642106920480728f, 0.011610857211053371f, 0.009068133309483528f, 0.007713416591286659f, 0.00561139453202486f, 0.006677169352769852f, 0.006654503755271435f, 0.014882739633321762f, 0.012733744457364082f, 0.009100847877562046f, 0.0060542975552380085f, 0.0056564705446362495f, 0.005150826182216406f, 0.005393765866756439f, 0.004316257778555155f, 0.002200949704274535f, 0.006122350227087736f, 0.007693050894886255f, 0.005967233795672655f, 0.0029961694963276386f, 0.009136022068560123f, 0.002219644607976079f, 0.006987632717937231f, 0.0020725405775010586f, 0.005141471046954393f, 0.004196941386908293f, 0.004136472474783659f, 0.0071432278491556644f, 0.007684823591262102f, 0.003916535992175341f, 0.0018889143830165267f, 0.006066604517400265f, 0.003593760309740901f, 0.0027857907116413116f, 0.004691200330853462f, 0.007150228135287762f, 0.008729408495128155f, 0.005451718810945749f, 0.004096520598977804f, 0.007833876647055149f, 0.012304509989917278f, 0.003927485551685095f, 0.016444681212306023f, 0.00689633609727025f, 0.005149764474481344f, 0.005507966037839651f, 0.003409256460145116f, 0.009741450659930706f, 0.006062996108084917f, 0.0062059978954494f, 0.004116102121770382f, 0.0032995641231536865f, 0.00571000250056386f, 0.01176040805876255f, 0.0047898069024086f, 0.005564526189118624f, 0.009128598496317863f, 0.0016973994206637144f, 0.0041914633475244045f, 0.004775139968842268f, 0.009519632905721664f, 0.0016919855261221528f, 0.006160364020615816f, 0.0036397145595401525f, 0.004940451588481665f, 0.00828850269317627f, 0.012008565478026867f, 0.007336319424211979f, 0.0056435163132846355f, 0.00707771023735404f, 0.003998225089162588f, 0.005540468730032444f, 0.01613655500113964f, 0.002506893826648593f, 0.0061943307518959045f, 0.019291725009679794f, 0.005183876492083073f, 0.008784414269030094f, 0.007608065381646156f, 0.00704627949744463f, 0.00793171301484108f, 0.006515487562865019f, 0.009296263568103313f, 0.004801188129931688f, 0.005561082623898983f, 0.006401455495506525f, 0.005566731560975313f, 0.003129076911136508f, 0.005034229252487421f, 0.00599141838029027f, 0.012184031307697296f, 0.0019652429036796093f, 0.0072666555643081665f, 0.005862073041498661f, 0.006396840326488018f, 0.006889762356877327f, 0.0048738145269453526f, 0.007046950981020927f, 0.0026247836649417877f, 0.008139070123434067f, 0.006839380599558353f, 0.0023517441004514694f, 0.003424333408474922f, 0.007978401146829128f, 0.004187813028693199f, 0.004468902945518494f, 0.005779583007097244f, 0.0061093103140592575f, 0.0035649207420647144f, 0.0031487392261624336f, 0.0016549502033740282f, 0.006955316755920649f, 0.004979877732694149f, 0.009762564674019814f, 0.006480646785348654f, 0.0030771372839808464f, 0.008087895810604095f);
static const ai_u16 conv2d_39_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_39_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_40_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_40_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_40_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_40_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_40_t_in_0_shape_ch_const_u16 = 144;
static const ai_u16 conv2d_40_t_out_0_shape_ch_const_u16 = 32;
static const ai_i8 conv2d_40_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_40_t_out_0_fmt_zero_const_s8 = 7;
static const ai_float conv2d_40_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_40_t_out_0_fmt_scale_const_f32 = 0.19619816541671753f;
static const ai_float conv2d_40_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.015758754685521126f, 0.013613006100058556f, 0.010914292186498642f, 0.013036380521953106f, 0.012331625446677208f, 0.010281873866915703f, 0.01311393454670906f, 0.011329054832458496f, 0.014505189843475819f, 0.011565872468054295f, 0.010889011435210705f, 0.012632052414119244f, 0.013275805860757828f, 0.012556598521769047f, 0.012793050147593021f, 0.012751332484185696f, 0.011647061444818974f, 0.01647571288049221f, 0.01626795344054699f, 0.012343681417405605f, 0.011444862000644207f, 0.010856285691261292f, 0.015352481976151466f, 0.0148445600643754f, 0.0157476756721735f, 0.01578538492321968f, 0.01695815660059452f, 0.011716071516275406f, 0.016161303967237473f, 0.016205012798309326f, 0.01220774743705988f, 0.01095335278660059f);
static const ai_layer_format_type conv2d_40_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_41_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_41_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_41_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_41_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_41_t_in_0_shape_ch_const_u16 = 32;
static const ai_u16 conv2d_41_t_out_0_shape_ch_const_u16 = 192;
static const ai_i8 conv2d_41_t_in_0_fmt_zero_const_s8 = 7;
static const ai_i8 conv2d_41_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_41_t_in_0_fmt_scale_const_f32 = 0.19619816541671753f;
static const ai_float conv2d_41_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_41_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0005892918561585248f, 0.0009683967800810933f, 0.001108720083720982f, 0.00034835259430110455f, 0.0003862175508402288f, 0.0005723033100366592f, 0.0005397147033363581f, 0.0009904535254463553f, 0.001017713570035994f, 0.00042191610555164516f, 0.0009210840798914433f, 0.0011893145274370909f, 0.0009460324654355645f, 0.0008210642845369875f, 0.0002801434020511806f, 0.00025458066374994814f, 0.0011442804243415594f, 0.0008156619151122868f, 0.0007725965115241706f, 0.0006959082093089819f, 0.0008227254147641361f, 0.00037565833190456033f, 0.0011825403198599815f, 0.0007953959866426885f, 0.0010720755672082305f, 0.0008645032648928463f, 0.00020675822452176362f, 0.0008671425748616457f, 0.000875558820553124f, 0.0004250133933965117f, 0.001043289084918797f, 0.0011095543159171939f, 0.000619462865870446f, 0.000532100850250572f, 0.0007093495805747807f, 0.0006134069990366697f, 0.0005827537388540804f, 0.001106854877434671f, 0.00025316138635389507f, 0.00035235474933870137f, 0.0005718391621485353f, 0.0005201855674386024f, 0.0005587554187513888f, 0.000693822861649096f, 0.0007884940132498741f, 0.0012714933836832643f, 0.0003382253635209054f, 0.0007652314961887896f, 0.0004781647876370698f, 0.00090799416648224f, 0.000633291550911963f, 0.0004018914478365332f, 0.000598752754740417f, 0.0006995906587690115f, 0.000389512802939862f, 0.0007261449354700744f, 0.00035491312155500054f, 0.001199429971165955f, 0.0007886979728937149f, 0.0009811964118853211f, 0.0011589450296014547f, 0.001012805849313736f, 0.00027466207393445075f, 0.0006484981277026236f, 0.0005600795266218483f, 0.0006148746469989419f, 0.000505014497321099f, 0.0007423936040140688f, 0.00035595466033555567f, 0.0007214059005491436f, 0.00040278269443660975f, 0.0005954725202172995f, 0.0007867785170674324f, 0.0006869337521493435f, 0.0005156674887984991f, 0.0017056838842108846f, 0.001072314684279263f, 0.0007571976748295128f, 0.000509737292304635f, 0.000486537377582863f, 0.0009742641123011708f, 0.0009235692559741437f, 0.0013286641333252192f, 0.0007038101321086287f, 0.00047295764670707285f, 0.0007406876538880169f, 0.0014446447603404522f, 0.0007821051985956728f, 0.0010660376865416765f, 0.0005215482087805867f, 0.0007744556642137468f, 0.0007326668710447848f, 0.0010930729331448674f, 0.0013218836393207312f, 0.0005936336820013821f, 0.00037155181053094566f, 0.00027475578826852143f, 0.00036354470648802817f, 0.0015605953522026539f, 0.0009129714453592896f, 0.0008837210480123758f, 0.0006952734547667205f, 0.0005988064804114401f, 0.000330130395013839f, 0.0009713011677376926f, 0.00039388795266859233f, 0.0008859067456796765f, 0.0011449895100668073f, 0.0009134848951362073f, 0.0005191897507756948f, 0.00039754671161063015f, 0.0004454287700355053f, 0.0005256888689473271f, 0.0009731650352478027f, 0.0013054515002295375f, 0.0006491614622063935f, 0.00033341883681714535f, 0.0005720271146856248f, 0.0009971533436328173f, 0.001312262611463666f, 0.001158667029812932f, 0.0008436979842372239f, 0.0010910556884482503f, 0.0008721678168512881f, 0.0005146440234966576f, 0.0007054415764287114f, 0.0006356107187457383f, 0.0011225418420508504f, 0.0003970133839175105f, 0.0007679937989450991f, 0.00028735955129377544f, 0.0007074169116094708f, 0.0011660681338980794f, 0.0005463691777549684f, 0.0005170811782591045f, 0.0006051211385056376f, 0.0008089145994745195f, 0.0007404725183732808f, 0.00053682632278651f, 0.0005445442511700094f, 0.0008372835582122207f, 0.0005981082795187831f, 0.0003103117924183607f, 0.00042014213977381587f, 0.0005311365239322186f, 0.0004271373327355832f, 0.00041838359902612865f, 0.0003171478456351906f, 0.00043902243487536907f, 0.00040242503746412694f, 0.0007973145111463964f, 0.0012473781825974584f, 0.0006701555685140193f, 0.0004875347949564457f, 0.0006032664678059518f, 0.0006279282388277352f, 0.0005920394905842841f, 0.0008978000842034817f, 0.0011904836865141988f, 0.0004718830168712884f, 0.0007479428313672543f, 0.0008415274205617607f, 0.0008152136579155922f, 0.0007234751828946173f, 0.0011170358629897237f, 0.0010498873889446259f, 0.0010855798609554768f, 0.0009093813714571297f, 0.000746839854400605f, 0.0005690801772288978f, 0.0025202452670782804f, 0.0008362609660252929f, 0.0015330449678003788f, 0.0002877727383747697f, 0.0006215701578184962f, 0.0008523307042196393f, 0.0014217273565009236f, 0.0009520520688965917f, 0.000886155292391777f, 0.0005388984573073685f, 0.0006687966524623334f, 0.0008065128931775689f, 0.0002705589577089995f, 0.000262446264969185f, 0.0015489754732698202f, 0.0006664033862762153f, 0.001180569059215486f, 0.000452576408861205f, 0.0006793987704440951f, 0.001078875153325498f, 0.000977821066044271f, 0.0012112107360735536f);
static const ai_layer_format_type conv2d_41_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_42_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_42_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_42_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_42_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_42_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_42_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_42_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_42_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_42_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_42_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_42_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_42_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_42_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.012598899193108082f, 0.013018568977713585f, 0.0038332000840455294f, 0.01359264925122261f, 0.011541658081114292f, 0.010504662059247494f, 0.007937607355415821f, 0.0044432953000068665f, 0.006222437135875225f, 0.01469497475773096f, 0.02632220648229122f, 0.004379025660455227f, 0.00433115242049098f, 0.01254522055387497f, 0.018956689164042473f, 0.018313586711883545f, 0.008598508313298225f, 0.01259143278002739f, 0.00950422789901495f, 0.006008756347000599f, 0.005091791041195393f, 0.018121765926480293f, 0.011426732875406742f, 0.003989367745816708f, 0.005130044184625149f, 0.005170475225895643f, 0.030019620433449745f, 0.01108565367758274f, 0.006007963325828314f, 0.010728788562119007f, 0.009972534142434597f, 0.01783912256360054f, 0.012030440382659435f, 0.007978139445185661f, 0.013416188769042492f, 0.009758309461176395f, 0.00981863308697939f, 0.00493814330548048f, 0.015570401214063168f, 0.010382497683167458f, 0.010653176344931126f, 0.007163263391703367f, 0.007341192103922367f, 0.008849451318383217f, 0.015358435921370983f, 0.004989429377019405f, 0.011198832653462887f, 0.007574290968477726f, 0.026064150035381317f, 0.0034899639431387186f, 0.005736708175390959f, 0.016286054626107216f, 0.009710906073451042f, 0.003209783462807536f, 0.013445976190268993f, 0.00804058089852333f, 0.020324179902672768f, 0.0057218954898417f, 0.003881764831021428f, 0.002296383725479245f, 0.005499690305441618f, 0.008154571987688541f, 0.01935950480401516f, 0.014970297925174236f, 0.0243682824075222f, 0.01548729371279478f, 0.009928599931299686f, 0.014305289834737778f, 0.019049828872084618f, 0.003627292113378644f, 0.00981360487639904f, 0.00979794654995203f, 0.006257489789277315f, 0.006743886042386293f, 0.002045532688498497f, 0.00433005578815937f, 0.003819460282102227f, 0.0026070778258144855f, 0.017923183739185333f, 0.008953254669904709f, 0.009015372022986412f, 0.011283093132078648f, 0.002778890309855342f, 0.005701960995793343f, 0.019049452617764473f, 0.006607987452298403f, 0.00983341597020626f, 0.003068830817937851f, 0.005754286888986826f, 0.008301613852381706f, 0.011752575635910034f, 0.011033624410629272f, 0.012705186381936073f, 0.008847368881106377f, 0.008791875094175339f, 0.011309263296425343f, 0.018736306577920914f, 0.008512571454048157f, 0.0036503123119473457f, 0.009718462824821472f, 0.003594281617552042f, 0.0021473232191056013f, 0.007517555728554726f, 0.013417737558484077f, 0.00937279500067234f, 0.011983645148575306f, 0.004037622828036547f, 0.0068901111371815205f, 0.005896448623389006f, 0.014484742656350136f, 0.011395280249416828f, 0.010326218791306019f, 0.009508829563856125f, 0.004304531496018171f, 0.006634665187448263f, 0.007728080730885267f, 0.015625229105353355f, 0.01630607806146145f, 0.01126885786652565f, 0.00524738198146224f, 0.007703233510255814f, 0.004701266065239906f, 0.014220776036381721f, 0.0028896948788315058f, 0.0047655124217271805f, 0.0075355516746640205f, 0.01241874136030674f, 0.004395657684653997f, 0.015553913079202175f, 0.015086092054843903f, 0.016139350831508636f, 0.005339293275028467f, 0.004102746956050396f, 0.007728589698672295f, 0.007651062682271004f, 0.0073153474368155f, 0.006621229462325573f, 0.0063705164939165115f, 0.019703054800629616f, 0.01326711755245924f, 0.0018275147303938866f, 0.014896760694682598f, 0.020154166966676712f, 0.013095039874315262f, 0.012353098019957542f, 0.006621864158660173f, 0.012454141862690449f, 0.01740364357829094f, 0.01037099864333868f, 0.013402465730905533f, 0.007733162958174944f, 0.005238109268248081f, 0.010651661083102226f, 0.016261285170912743f, 0.011045562103390694f, 0.008831155486404896f, 0.008823503740131855f, 0.00758372712880373f, 0.010898767970502377f, 0.010881914757192135f, 0.008347460068762302f, 0.011286339722573757f, 0.002331372583284974f, 0.009030395187437534f, 0.006775959394872189f, 0.0061079650186002254f, 0.003424199065193534f, 0.009703032672405243f, 0.003708623582497239f, 0.010845416225492954f, 0.006740873213857412f, 0.00443822517991066f, 0.002417769515886903f, 0.016828687861561775f, 0.009936172515153885f, 0.010071673430502415f, 0.011517293751239777f, 0.010408257134258747f, 0.009940237738192081f, 0.009664918296039104f, 0.008661720901727676f, 0.0017265829956158996f, 0.017837077379226685f, 0.01905546337366104f, 0.007170300930738449f, 0.009746533818542957f, 0.004987458232790232f, 0.01352695468813181f, 0.004843190312385559f, 0.011003589257597923f, 0.0038492295425385237f, 0.014315804466605186f);
static const ai_u16 conv2d_42_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_42_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_43_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_43_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_43_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_43_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_43_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_43_t_out_0_shape_ch_const_u16 = 32;
static const ai_i8 conv2d_43_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_43_t_out_0_fmt_zero_const_s8 = -13;
static const ai_float conv2d_43_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_43_t_out_0_fmt_scale_const_f32 = 0.24855132400989532f;
static const ai_float conv2d_43_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.004366690292954445f, 0.008977836929261684f, 0.003640740644186735f, 0.011482148431241512f, 0.004018910229206085f, 0.014715811237692833f, 0.0056792874820530415f, 0.011143699288368225f, 0.003791472176089883f, 0.003832623129710555f, 0.018678627908229828f, 0.0026173724327236414f, 0.011444718576967716f, 0.010800645686686039f, 0.0067934333346784115f, 0.007879924960434437f, 0.005047662649303675f, 0.012545671314001083f, 0.0070317527279257774f, 0.005731257144361734f, 0.0031017703004181385f, 0.0026065472047775984f, 0.0028860236052423716f, 0.011709538288414478f, 0.006328983698040247f, 0.009165334515273571f, 0.007324435282498598f, 0.00471313064917922f, 0.0040941983461380005f, 0.009883164428174496f, 0.003714079037308693f, 0.005309118423610926f);
static const ai_layer_format_type conv2d_43_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_45_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_45_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_45_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_45_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_45_t_in_0_shape_ch_const_u16 = 32;
static const ai_u16 conv2d_45_t_out_0_shape_ch_const_u16 = 192;
static const ai_i8 conv2d_45_t_in_0_fmt_zero_const_s8 = -13;
static const ai_i8 conv2d_45_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_45_t_in_0_fmt_scale_const_f32 = 0.24855132400989532f;
static const ai_float conv2d_45_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_45_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0010950000723823905f, 0.0002883437555283308f, 0.000669780361931771f, 0.0006410444038920105f, 0.0007379797170870006f, 0.0003762177366297692f, 0.0004624905704986304f, 0.0005133188096806407f, 0.0004980898811481893f, 0.0007635537767782807f, 0.0006730598979629576f, 0.00025990756694227457f, 0.0015174588188529015f, 0.000319119164487347f, 0.00031492288690060377f, 0.000377675547497347f, 0.0008216160931624472f, 0.0006881013396196067f, 0.0005218152655288577f, 0.0004016710154246539f, 0.0003425625618547201f, 0.00023252671235240996f, 0.0005543255829252303f, 0.0009478963329456747f, 0.0001837502495618537f, 0.0006089242524467409f, 0.0005303237703628838f, 0.0005781287327408791f, 0.0006604458321817219f, 0.00036429549800232053f, 0.0005073566571809351f, 0.0002969824126921594f, 0.0005998452543281019f, 0.0008614121470600367f, 0.0007625419530086219f, 0.00023282402253244072f, 0.0006247613928280771f, 0.0006707378779537976f, 0.0005095512024126947f, 0.00032867558184079826f, 0.0008004896226339042f, 0.0007152683683671057f, 0.0014514164067804813f, 0.0005418018554337323f, 0.0008284205687232316f, 0.0007301923469640315f, 0.000374662660760805f, 0.0007644596043974161f, 0.0006131004774942994f, 0.000783709401730448f, 0.0005664419732056558f, 0.0004773270047735423f, 0.0010482969228178263f, 0.0003846696636173874f, 0.001049449434503913f, 0.0006328434683382511f, 0.0007186495349742472f, 0.0004680989950429648f, 0.00038080537342466414f, 0.00038633818621747196f, 0.0007343089091591537f, 0.0005716260056942701f, 0.00048614011029712856f, 0.0003215296019334346f, 0.00047000491758808494f, 0.0006981574115343392f, 0.0006150258122943342f, 0.0009140167385339737f, 0.000694693299010396f, 0.0007198170060291886f, 0.0005734678125008941f, 0.0003820087295025587f, 0.0006036179256625473f, 0.00024332301109097898f, 0.0003388366603758186f, 0.0011692355619743466f, 0.000816576648503542f, 0.00022085526143200696f, 0.0005725748487748206f, 0.0006664743414148688f, 0.000602293643169105f, 0.000622570572886616f, 0.0008646778296679258f, 0.0005675628199242055f, 0.0015478171408176422f, 0.0006833006045781076f, 0.0006539232563227415f, 0.0008512578206136823f, 0.0002862952824216336f, 0.0006947654765099287f, 0.0017581423744559288f, 0.0005783562082797289f, 0.0011470038443803787f, 0.0005537004326470196f, 0.000518397893756628f, 0.000467199191916734f, 0.0008920709369704127f, 0.0005704063223674893f, 0.0004029099363833666f, 0.0005797733901999891f, 0.000885385787114501f, 0.0004376738506834954f, 0.0005052426131442189f, 0.0006650451687164605f, 0.0008997433469630778f, 0.0007810115348547697f, 0.0010174582712352276f, 0.0003953860141336918f, 0.00033200051984749734f, 0.0006723218248225749f, 0.0006419840501621366f, 0.0012530668172985315f, 0.0005035928916186094f, 0.0007400074391625822f, 0.0017170264618471265f, 0.0010262246942147613f, 0.0015062816673889756f, 0.0008545605232939124f, 0.0010381686734035611f, 0.0009818162070587277f, 0.0009058065479621291f, 0.001002501230686903f, 0.0006431681686080992f, 0.00037584902020171285f, 0.0011400573421269655f, 0.0004816140572074801f, 0.0007244065054692328f, 0.0007105766562744975f, 0.0008497887756675482f, 0.0008511882624588907f, 0.0006171768764033914f, 0.0005568891647271812f, 0.00035323045449331403f, 0.00035674910759553313f, 0.0007749642245471478f, 0.00028067745734006166f, 0.0006870838114991784f, 0.0008177682175301015f, 0.000749703380279243f, 0.0006871543009765446f, 0.0009408397600054741f, 0.00034377441625110805f, 0.0006508889491669834f, 0.0007057171314954758f, 0.0011639453005045652f, 0.0006605532253161073f, 0.000535084749571979f, 0.0008137887925840914f, 0.0006467366474680603f, 0.002460616175085306f, 0.00026250857627019286f, 0.0006209989660419524f, 0.0008770408458076417f, 0.0007338356226682663f, 0.0007998683722689748f, 0.0005813098978251219f, 0.0008569305646233261f, 0.001078990986570716f, 0.0009804023429751396f, 0.0007598613738082349f, 0.00042578743887133896f, 0.00042390250018797815f, 0.0006094484124332666f, 0.0007681518909521401f, 0.0004775717097800225f, 0.0006858944543637335f, 0.00029348742100410163f, 0.0011102547869086266f, 0.0006007013726048172f, 0.0006806289311498404f, 0.0007610084721818566f, 0.0007166552823036909f, 0.0006070384988561273f, 0.0010245757875964046f, 0.0007806514040566981f, 0.00021117026335559785f, 0.0022323508746922016f, 0.00022048632672522217f, 0.0005914243520237505f, 0.0005959945847280324f, 0.00035319558810442686f, 0.002039107494056225f, 0.00034271195181645453f, 0.0008244906202889979f, 0.0010717856930568814f, 0.0008414614130742848f, 0.0007108167628757656f, 0.0004888998810201883f, 0.000781363109126687f, 0.0012132112169638276f, 0.0008042450062930584f, 0.00020576544920913875f);
static const ai_layer_format_type conv2d_45_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_46_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_46_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_46_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_46_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_46_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_46_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_46_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_46_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_46_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_46_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_46_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_46_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_46_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.008536569774150848f, 0.011833516880869865f, 0.0025878706946969032f, 0.008230053819715977f, 0.007216011174023151f, 0.01946309208869934f, 0.0020330322440713644f, 0.010304674506187439f, 0.014574306085705757f, 0.018040552735328674f, 0.007719508372247219f, 0.007700106594711542f, 0.013596558943390846f, 0.02038864605128765f, 0.012603356502950191f, 0.015772784128785133f, 0.004708948079496622f, 0.006397800054401159f, 0.006110745947808027f, 0.009509657509624958f, 0.0064245229586958885f, 0.02050694078207016f, 0.005997419357299805f, 0.012666602618992329f, 0.014866259880363941f, 0.004620115272700787f, 0.009726526215672493f, 0.004515443928539753f, 0.004863657057285309f, 0.006661632098257542f, 0.01008912455290556f, 0.010077019222080708f, 0.005811777897179127f, 0.006048545707017183f, 0.005495124030858278f, 0.014921368099749088f, 0.006537527311593294f, 0.007477456238120794f, 0.006429539527744055f, 0.00548831932246685f, 0.006398004014045f, 0.00693505397066474f, 0.0035368441604077816f, 0.005184870678931475f, 0.005441412795335054f, 0.006100259721279144f, 0.020213533192873f, 0.0049822027795016766f, 0.005588679574429989f, 0.002680000150576234f, 0.003529561683535576f, 0.011236815713346004f, 0.009313750080764294f, 0.011656062677502632f, 0.006596735678613186f, 0.010445541702210903f, 0.002423053840175271f, 0.006744074635207653f, 0.005627892445772886f, 0.004703086335211992f, 0.011324319057166576f, 0.009527349844574928f, 0.005452142562717199f, 0.008584774099290371f, 0.00543594267219305f, 0.011994162574410439f, 0.006480109412223101f, 0.006696694064885378f, 0.009763055481016636f, 0.007396816276013851f, 0.00838120374828577f, 0.005208444781601429f, 0.0029320584144443274f, 0.012423811480402946f, 0.013579069636762142f, 0.005229179747402668f, 0.014062901958823204f, 0.011844752356410027f, 0.006005069240927696f, 0.011424878612160683f, 0.007586252875626087f, 0.003795999800786376f, 0.0050492119044065475f, 0.008863571099936962f, 0.004007123876363039f, 0.017373038455843925f, 0.0019414059352129698f, 0.011043405160307884f, 0.005545082036405802f, 0.005802216939628124f, 0.007553722243756056f, 0.005173240322619677f, 0.011541653424501419f, 0.003128018928691745f, 0.010600044392049313f, 0.011649328283965588f, 0.012860477901995182f, 0.013870641589164734f, 0.007456631399691105f, 0.011181059293448925f, 0.01589067280292511f, 0.003573250025510788f, 0.0067552984692156315f, 0.008471224457025528f, 0.0038475610781461f, 0.006528270430862904f, 0.012769452296197414f, 0.004477945622056723f, 0.012559381313621998f, 0.0051593538373708725f, 0.0097417077049613f, 0.004502647556364536f, 0.008543726056814194f, 0.004126339685171843f, 0.0026721034664660692f, 0.011180861853063107f, 0.006053200922906399f, 0.010347343981266022f, 0.006755258422344923f, 0.011133728548884392f, 0.005988774821162224f, 0.0015904823085293174f, 0.01662532426416874f, 0.013015643693506718f, 0.005360718350857496f, 0.006771155633032322f, 0.012587834149599075f, 0.005581877194344997f, 0.0160862747579813f, 0.0037780064158141613f, 0.008843201212584972f, 0.012834640219807625f, 0.013821031898260117f, 0.009253042750060558f, 0.004788199905306101f, 0.012508953921496868f, 0.005885442718863487f, 0.004136791452765465f, 0.006948693655431271f, 0.004679353907704353f, 0.019607145339250565f, 0.013598362915217876f, 0.006542635150253773f, 0.009584042243659496f, 0.012733528390526772f, 0.005331696011126041f, 0.00897842925041914f, 0.01019369252026081f, 0.0054224394261837006f, 0.003039997536689043f, 0.013853496871888638f, 0.002144497586414218f, 0.007971071638166904f, 0.008836157619953156f, 0.006582142319530249f, 0.006209039129316807f, 0.0017395252361893654f, 0.009594769217073917f, 0.003719848580658436f, 0.00574724143370986f, 0.012370326556265354f, 0.0052269455045461655f, 0.005791368428617716f, 0.010217751376330853f, 0.011737114749848843f, 0.004387473221868277f, 0.011684892699122429f, 0.015025446191430092f, 0.006718405522406101f, 0.0054953014478087425f, 0.0050214496441185474f, 0.0013941630022600293f, 0.011714302934706211f, 0.0034196332562714815f, 0.0074303545989096165f, 0.005582501646131277f, 0.002825177274644375f, 0.011147859506309032f, 0.010078571736812592f, 0.006648059468716383f, 0.01087688934057951f, 0.0023801422212272882f, 0.014786993153393269f, 0.008390135131776333f, 0.005546118598431349f, 0.014832600951194763f, 0.004065976943820715f, 0.021776126697659492f, 0.016837412491440773f, 0.013584892265498638f, 0.003506118431687355f, 0.02243982069194317f);
static const ai_u16 conv2d_46_t_out_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_46_t_out_0_shape_h_const_u16 = 6;

static const ai_u16 conv2d_47_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_47_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_47_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_47_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_47_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_47_t_out_0_shape_ch_const_u16 = 32;
static const ai_i8 conv2d_47_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_47_t_out_0_fmt_zero_const_s8 = 3;
static const ai_float conv2d_47_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_47_t_out_0_fmt_scale_const_f32 = 0.2924808859825134f;
static const ai_float conv2d_47_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.005229023285210133f, 0.015207793563604355f, 0.007969745434820652f, 0.011187364347279072f, 0.0043243844993412495f, 0.024079712107777596f, 0.004070993047207594f, 0.011278333142399788f, 0.0067995102144777775f, 0.00807518232613802f, 0.023209797218441963f, 0.003881785785779357f, 0.010819921270012856f, 0.0165390744805336f, 0.006323304492980242f, 0.008637872524559498f, 0.006162106990814209f, 0.01869414560496807f, 0.011264346539974213f, 0.00587077159434557f, 0.0029492201283574104f, 0.005704990588128567f, 0.004879707004874945f, 0.018797293305397034f, 0.008164447732269764f, 0.01422465592622757f, 0.006831960286945105f, 0.007316012866795063f, 0.0030199335888028145f, 0.010394496843218803f, 0.003950090147554874f, 0.004047404043376446f);
static const ai_layer_format_type conv2d_47_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_49_t_in_0_shape_w_const_u16 = 6;
static const ai_u16 conv2d_49_t_in_0_shape_h_const_u16 = 6;
static const ai_u16 conv2d_49_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_49_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_49_t_in_0_shape_ch_const_u16 = 32;
static const ai_u16 conv2d_49_t_out_0_shape_ch_const_u16 = 192;
static const ai_i8 conv2d_49_t_in_0_fmt_zero_const_s8 = 3;
static const ai_i8 conv2d_49_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_49_t_in_0_fmt_scale_const_f32 = 0.2924808859825134f;
static const ai_float conv2d_49_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_49_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0006621638895012438f, 0.00048585646436549723f, 0.0011488695163279772f, 0.0006039620493538678f, 0.0007438521715812385f, 0.0005553956725634634f, 0.0008625088958069682f, 0.0001985510316444561f, 0.0006836594548076391f, 0.0007582198013551533f, 0.0005336416070349514f, 0.0005105784512124956f, 0.0008068528841249645f, 0.0007749020005576313f, 0.0008254173444584012f, 0.0004776434216182679f, 0.001215760363265872f, 0.0005563361919485033f, 0.000736261485144496f, 0.0013131186133250594f, 0.0008316417224705219f, 0.0008471091859973967f, 0.0006823737639933825f, 0.000744311953894794f, 0.0006616652826778591f, 0.0008010502788238227f, 0.0006923518376424909f, 0.001115844352170825f, 0.000905103050172329f, 0.0008148867054842412f, 0.0005663312622345984f, 0.0009416872053407133f, 0.0005851288442499936f, 0.0008841066737659276f, 0.0005689890822395682f, 0.000942370155826211f, 0.0006016891566105187f, 0.0007396901492029428f, 0.0006612308789044619f, 0.0008362980443052948f, 0.0004200282273814082f, 0.0007173399208113551f, 0.0005324539379216731f, 0.0007694593514315784f, 0.0008451095782220364f, 0.0010267505422234535f, 0.0006254917825572193f, 0.0008813496679067612f, 0.000651064096018672f, 0.0004491154686547816f, 0.0011917324736714363f, 0.000876919599249959f, 0.0004927844856865704f, 0.0004559208173304796f, 0.0005576641415245831f, 0.0006863398011773825f, 0.0006502729374915361f, 0.00119566370267421f, 0.0004485122626647353f, 0.0004290000069886446f, 0.0008586817421019077f, 0.0007313211681321263f, 0.0009057972929440439f, 0.0008327309042215347f, 0.0007000152254477143f, 0.0010839031310752034f, 0.0007761791930533946f, 0.0010867201490327716f, 0.0005853354814462364f, 0.0006859630229882896f, 0.0010238962713629007f, 0.0007034476730041206f, 0.0006137971649877727f, 0.0010958214988932014f, 0.0006673509487882257f, 0.0008150083594955504f, 0.0005920631811022758f, 0.0008374533499591053f, 0.0007333561079576612f, 0.0009970281971618533f, 0.0005798691418021917f, 0.0005796677432954311f, 0.0008320576744154096f, 0.0007370651583187282f, 0.0006828262703493237f, 0.000944126455578953f, 0.0007150977035053074f, 0.0007666199235245585f, 0.000701543758623302f, 0.0003440077416598797f, 0.0008447899599559605f, 0.0008501902339048684f, 0.0008061022381298244f, 0.0005870426539331675f, 0.0006932527176104486f, 0.0009006313630379736f, 0.0006012683152221143f, 0.0006952633848413825f, 0.0006756643415428698f, 0.000895133416634053f, 0.0008415938937105238f, 0.0005298870382830501f, 0.0007268032641150057f, 0.0008652692777104676f, 0.0006965132197365165f, 0.0006440577562898397f, 0.0007904756348580122f, 0.000888388545718044f, 0.0008375822799280286f, 0.0005523317377083004f, 0.0005637090653181076f, 0.0004964476102031767f, 0.000496499240398407f, 0.0007315713446587324f, 0.0009588819229975343f, 0.0007766162743791938f, 0.0007942275842651725f, 0.0005703697097487748f, 0.0005231201648712158f, 0.0007153235492296517f, 0.0005528730107471347f, 0.0008834149339236319f, 0.001136208651587367f, 0.0002946637978311628f, 0.0007293852977454662f, 0.0007346792262978852f, 0.0007952527957968414f, 0.0005383323295973241f, 0.0003030574880540371f, 0.000649069610517472f, 0.0007293493254110217f, 0.0007525052060373127f, 0.0008430078742094338f, 0.0005954733933322132f, 0.00042574104736559093f, 0.0009045878541655838f, 0.0008739958284422755f, 0.0008420985541306436f, 0.00038746013888157904f, 0.0009094717097468674f, 0.0005805331165902317f, 0.0009993923595175147f, 0.0006112839910201728f, 0.0006003000307828188f, 0.0007429851684719324f, 0.0006504218908958137f, 0.0005749199190177023f, 0.0018254221649840474f, 0.0009704306139610708f, 0.0006813910440541804f, 0.0008220843155868351f, 0.0006236766930669546f, 0.000640659302007407f, 0.0007644634461030364f, 0.000814207480289042f, 0.0007281149737536907f, 0.001030716230161488f, 0.0007618883973918855f, 0.0007616323418915272f, 0.0007107106503099203f, 0.0009523372864350677f, 0.0006227564881555736f, 0.0006515232380479574f, 0.0005738710169680417f, 0.00045601814053952694f, 0.0007220035186037421f, 0.0005033363122493029f, 0.0010545996483415365f, 0.0010832733241841197f, 0.0007474671001546085f, 0.0006231794250197709f, 0.0005501781706698239f, 0.0006948669324629009f, 0.0006936754216440022f, 0.000907848181668669f, 0.0005522168939933181f, 0.0006672024028375745f, 0.0009061250020749867f, 0.0005330644780769944f, 0.00020012666936963797f, 0.0025857079308480024f, 0.0005543581210076809f, 0.0004393677518237382f, 0.0007732871454209089f, 0.0005335030145943165f, 0.000536816252861172f, 0.0008827437413856387f, 0.0006385974120348692f, 0.0007448250544257462f, 0.0006413822993636131f, 0.0006953579722903669f, 0.0008414798649027944f);
static const ai_layer_format_type conv2d_49_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_50_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_50_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_50_pad_before_t_in_0_shape_h_const_u32 = 6;

static const ai_u16 conv2d_50_t_in_0_shape_w_const_u16 = 8;
static const ai_u16 conv2d_50_t_in_0_shape_h_const_u16 = 8;
static const ai_u16 conv2d_50_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_50_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_50_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_50_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_50_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_50_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_50_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_50_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.003248484106734395f, 0.00545393256470561f, 0.004160788841545582f, 0.0026619115378707647f, 0.004062734544277191f, 0.006342827808111906f, 0.004453369881957769f, 0.012781351804733276f, 0.005271493457257748f, 0.00430153077468276f, 0.003082867246121168f, 0.005378466099500656f, 0.00343522010371089f, 0.004801366478204727f, 0.006328654941171408f, 0.005357156042009592f, 0.0023971409536898136f, 0.003601732896640897f, 0.004373755771666765f, 0.004173543304204941f, 0.005116766318678856f, 0.004293735604733229f, 0.0047302995808422565f, 0.006010788958519697f, 0.0062576537020504475f, 0.0036567950155586004f, 0.004135108087211847f, 0.0029623417649418116f, 0.003779827384278178f, 0.0022651965264230967f, 0.0035111880861222744f, 0.0035175487864762545f, 0.006227145437151194f, 0.004417590796947479f, 0.0029256532434374094f, 0.0025206811260432005f, 0.0036005352158099413f, 0.005567023530602455f, 0.0028489648830145597f, 0.0029646276962012053f, 0.004137654788792133f, 0.0033383469562977552f, 0.0029825828969478607f, 0.004596407059580088f, 0.00657189404591918f, 0.0029471188317984343f, 0.005251074675470591f, 0.005850616842508316f, 0.004335543140769005f, 0.004575116094201803f, 0.0028376569971442223f, 0.0040811034850776196f, 0.004320797510445118f, 0.0031892929691821337f, 0.005495596211403608f, 0.004511676728725433f, 0.0036816454958170652f, 0.003225291846320033f, 0.006006500218063593f, 0.006898972205817699f, 0.003782428102567792f, 0.005016724579036236f, 0.004608592949807644f, 0.0033961827866733074f, 0.003160266438499093f, 0.004276646301150322f, 0.0039535569958388805f, 0.004950270988047123f, 0.0042587993666529655f, 0.00523222703486681f, 0.005167779978364706f, 0.00439635431393981f, 0.003219255479052663f, 0.004961490165442228f, 0.00364371994510293f, 0.004691770765930414f, 0.0051920972764492035f, 0.004485008306801319f, 0.003912129905074835f, 0.004619088489562273f, 0.006271816790103912f, 0.00464413408190012f, 0.0037640586961060762f, 0.003971255850046873f, 0.0033310886938124895f, 0.0043124305084347725f, 0.004206565674394369f, 0.005902495235204697f, 0.0038612806238234043f, 0.005754603538662195f, 0.004837095271795988f, 0.002428246894851327f, 0.003597953589633107f, 0.004170358646661043f, 0.0031198477372527122f, 0.006228884216398001f, 0.003196172881871462f, 0.0032595773227512836f, 0.004930213093757629f, 0.003781509818509221f, 0.00730907591059804f, 0.005950569175183773f, 0.00346837448887527f, 0.005970476195216179f, 0.004274014849215746f, 0.005166972987353802f, 0.00548718823119998f, 0.003561733989045024f, 0.0037520634941756725f, 0.008366341702640057f, 0.004154914058744907f, 0.003331762505695224f, 0.0038331698160618544f, 0.00438885111361742f, 0.0044934977777302265f, 0.005467900540679693f, 0.003428841708227992f, 0.003775050863623619f, 0.003143486101180315f, 0.0045195999555289745f, 0.0034993092995136976f, 0.003276081755757332f, 0.004246360156685114f, 0.005079871043562889f, 0.005496826022863388f, 0.00480304379016161f, 0.0038658580742776394f, 0.005165028851479292f, 0.0074379839934408665f, 0.003271921304985881f, 0.00437164818868041f, 0.004776583053171635f, 0.005937427747994661f, 0.0066504040732979774f, 0.005186200607568026f, 0.005065805744379759f, 0.0020934620406478643f, 0.0036433867644518614f, 0.005416580475866795f, 0.003690629033371806f, 0.004004668444395065f, 0.003043540520593524f, 0.004652706906199455f, 0.0038213676307350397f, 0.0040863556787371635f, 0.003391672857105732f, 0.005741281900554895f, 0.0023138069082051516f, 0.003459337167441845f, 0.004831189289689064f, 0.0033824124839156866f, 0.004296891391277313f, 0.004985596053302288f, 0.003900996409356594f, 0.005433619488030672f, 0.002516113920137286f, 0.004388708155602217f, 0.00547461211681366f, 0.004213531501591206f, 0.004364100284874439f, 0.005762773100286722f, 0.005010399501770735f, 0.005088418256491423f, 0.003568492829799652f, 0.003959103021770716f, 0.005584482103586197f, 0.00439771544188261f, 0.004424995742738247f, 0.003185266861692071f, 0.004050577059388161f, 0.0036214753054082394f, 0.00423084944486618f, 0.003820717101916671f, 0.00257291691377759f, 0.003358034649863839f, 0.00461305258795619f, 0.00553685799241066f, 0.004271761514246464f, 0.004070404917001724f, 0.011703827418386936f, 0.0028374281246215105f, 0.006927064154297113f, 0.004843422211706638f, 0.0029020283836871386f, 0.003820233978331089f, 0.003481103340163827f, 0.003582834964618087f, 0.004789788741618395f, 0.003624565899372101f, 0.005472707096487284f, 0.0041809589602053165f, 0.006532546132802963f);
static const ai_u16 conv2d_50_t_out_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_50_t_out_0_shape_h_const_u16 = 3;

static const ai_u16 conv2d_51_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_51_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_51_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_51_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_51_t_in_0_shape_ch_const_u16 = 192;
static const ai_u16 conv2d_51_t_out_0_shape_ch_const_u16 = 56;
static const ai_i8 conv2d_51_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_51_t_out_0_fmt_zero_const_s8 = 16;
static const ai_float conv2d_51_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_51_t_out_0_fmt_scale_const_f32 = 0.16510659456253052f;
static const ai_float conv2d_51_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.007431203965097666f, 0.006505809724330902f, 0.0073395464569330215f, 0.00935128889977932f, 0.009498890489339828f, 0.008327445946633816f, 0.008943159133195877f, 0.010587221011519432f, 0.00747388880699873f, 0.0070425677113235f, 0.008018959313631058f, 0.010930605232715607f, 0.010824726894497871f, 0.007397542241960764f, 0.008343573659658432f, 0.008981170132756233f, 0.0110764279961586f, 0.010412345640361309f, 0.007968478836119175f, 0.010838859714567661f, 0.008032983168959618f, 0.011183548718690872f, 0.008135637268424034f, 0.008451550267636776f, 0.008903412148356438f, 0.008429395034909248f, 0.009268376044929028f, 0.00674918107688427f, 0.010822012089192867f, 0.0065508633852005005f, 0.006342693697661161f, 0.011622493155300617f, 0.008233429864048958f, 0.008216822519898415f, 0.008501543663442135f, 0.011423000134527683f, 0.009104675613343716f, 0.007476974278688431f, 0.014403213746845722f, 0.00921575352549553f, 0.010563391260802746f, 0.009160570800304413f, 0.010031729005277157f, 0.007922825403511524f, 0.009606173262000084f, 0.009864764288067818f, 0.007131360471248627f, 0.007547419983893633f, 0.009802061133086681f, 0.007355070207268f, 0.007742985151708126f, 0.010853122919797897f, 0.009750965051352978f, 0.010956872254610062f, 0.007266490254551172f, 0.010070743970572948f);
static const ai_layer_format_type conv2d_51_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_52_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_52_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_52_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_52_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_52_t_in_0_shape_ch_const_u16 = 56;
static const ai_u16 conv2d_52_t_out_0_shape_ch_const_u16 = 336;
static const ai_i8 conv2d_52_t_in_0_fmt_zero_const_s8 = 16;
static const ai_i8 conv2d_52_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_52_t_in_0_fmt_scale_const_f32 = 0.16510659456253052f;
static const ai_float conv2d_52_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_52_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0007149997400119901f, 0.0004977763164788485f, 0.0009246476111002266f, 0.0006982502527534962f, 0.0008605257025919855f, 0.0004652834904845804f, 0.00029408346745185554f, 0.0005005619605071843f, 0.0008572349906899035f, 0.0003824701125267893f, 0.0006767170270904899f, 0.00031990878051146865f, 0.000708394218236208f, 0.0006470541702583432f, 0.0006426926120184362f, 0.000601405743509531f, 0.000605628767516464f, 0.0005287174135446548f, 0.0004549068689811975f, 0.0008489811443723738f, 0.00044821694609709084f, 0.0017964658327400684f, 0.0014467312721535563f, 0.0005726322997361422f, 0.0009526800713501871f, 0.0012926619965583086f, 0.0005070603801868856f, 0.0005282579804770648f, 0.0003992436686530709f, 0.0005663479678332806f, 0.0007576183415949345f, 0.0004663089057430625f, 0.0005090058548375964f, 0.0003784661821555346f, 0.0003728072333615273f, 0.00033550645457580686f, 0.0012110717361792922f, 0.00039756670594215393f, 0.0002757713955361396f, 0.0008497004746459424f, 0.0008506960002705455f, 0.0007465500384569168f, 0.0005062647396698594f, 0.0009489352232776582f, 0.0003427700139582157f, 0.0006843047449365258f, 0.0006092761177569628f, 0.00040564584196545184f, 0.0004004569782409817f, 0.0005492183263413608f, 0.000793080369476229f, 0.0006364249857142568f, 0.0004172340559307486f, 0.00034145021345466375f, 0.0012054132530465722f, 0.0005872460315003991f, 0.0008499946561641991f, 0.0006742568220943213f, 0.0006565115181729198f, 0.00104196195024997f, 0.000210102298296988f, 0.0008735705050639808f, 0.0004954409669153392f, 0.0006587841198779643f, 0.0004929443821310997f, 0.0004904314992018044f, 0.0008204599726013839f, 0.00045732216676697135f, 0.0006902614259161055f, 0.0005684005445800722f, 0.0003822855360340327f, 0.0002507614844944328f, 0.000739330833312124f, 0.0005205550696700811f, 0.0008458513766527176f, 0.0006377986283041537f, 0.00036612755502574146f, 0.00047300811274908483f, 0.0005712934653274715f, 0.001008415943942964f, 0.0007910270942375064f, 0.0008177031995728612f, 0.0009259540238417685f, 0.0003434909740462899f, 0.0001770902017597109f, 0.0010856877779588103f, 0.00033415888901799917f, 0.00036397940129972994f, 0.0004774330591317266f, 0.0005830152658745646f, 0.0005169230280444026f, 0.0005359914503060281f, 0.0007962195668369532f, 0.0008367514819838107f, 0.000947149470448494f, 0.0004775255802087486f, 0.00044052311568520963f, 0.0008661479223519564f, 0.0007260347483679652f, 0.0008964836597442627f, 0.00119415030349046f, 0.0015295870834961534f, 0.0007549462025053799f, 0.0011790210846811533f, 0.0007183460984379053f, 0.0005014748312532902f, 0.0011808463605120778f, 0.0007396260625682771f, 0.00048397138016298413f, 0.0004142817051615566f, 0.0010295235551893711f, 0.0009117267327383161f, 0.0004796617722604424f, 0.0012237197952345014f, 0.0021415145602077246f, 0.000799747824203223f, 0.00020871576271019876f, 0.0006009249482303858f, 0.0008288614917546511f, 0.000873187615070492f, 0.0007582995458506048f, 0.0002476224326528609f, 0.0014311112463474274f, 0.0002665164938662201f, 0.0009115234133787453f, 0.00025637244107201695f, 0.00046114937867969275f, 0.0005342784570530057f, 0.001017174101434648f, 0.00042774894973263144f, 0.00033746680128388107f, 0.0008851275779306889f, 0.0005359581555239856f, 0.0008044233545660973f, 0.0006021019071340561f, 0.0008147290791384876f, 0.0003997886669822037f, 0.0005318685434758663f, 0.00046627764822915196f, 0.0008333321893587708f, 0.00047882256330922246f, 0.000742438598535955f, 0.0008881991961970925f, 0.0007828930392861366f, 0.0007090524304658175f, 0.0005242417682893574f, 0.0007849278044886887f, 0.00032873591408133507f, 0.0008097753161564469f, 0.0009064109763130546f, 0.000959063065238297f, 0.0005295056034810841f, 0.0007504529785364866f, 0.0003903133037965745f, 0.0007301475270651281f, 0.00036376219941303134f, 0.0004252411308698356f, 0.0008199100266210735f, 0.001045980374328792f, 0.00019146611157339066f, 0.0005473853088915348f, 0.0010133996838703752f, 0.0014768089167773724f, 0.0002728350227698684f, 0.0007123294053599238f, 0.00046820740681141615f, 0.00045392944593913853f, 0.0007238221005536616f, 0.0007060440257191658f, 0.0008963625296019018f, 0.0009338643867522478f, 0.0006660142680630088f, 0.0010688812471926212f, 0.00046124649816192687f, 0.0004579467058647424f, 0.0003083030751440674f, 0.0004170280590187758f, 0.0005031626205891371f, 0.0006193934241309762f, 0.0005231969989836216f, 0.0003755579818971455f, 0.0005565286846831441f, 0.00030101422453299165f, 0.0004367584188003093f, 0.0007340654847212136f, 0.00043869114597328007f, 0.0006530261598527431f, 0.0006101409671828151f, 0.0008457224466837943f, 0.0007066717953421175f, 0.00029984646243974566f, 0.002529095858335495f, 0.0004313309327699244f, 0.0006933650583960116f, 0.0007126241689547896f, 0.00045489007607102394f, 0.0009280765079893172f, 0.00048477682867087424f, 0.000793672283180058f, 0.0005235499702394009f, 0.0004018004110548645f, 0.00027576705906540155f, 0.00048714331933297217f, 0.0003137424646411091f, 0.0005400125519372523f, 0.0004994050832465291f, 0.0005747586255893111f, 0.0006012105732224882f, 0.0003882498131133616f, 0.0007306497427634895f, 0.0005465508438646793f, 0.0005197263672016561f, 0.0006101636099629104f, 0.0007157178479246795f, 0.000816439394839108f, 0.0006846953183412552f, 0.0011863649124279618f, 0.0008025072747841477f, 0.0012120106257498264f, 0.000638168363366276f, 0.0006776282098144293f, 0.0005440387758426368f, 0.0007762561435811222f, 0.0007625120924785733f, 0.00047083571553230286f, 0.00036225508665665984f, 0.0006218498456291854f, 0.0006009887438267469f, 0.0008094263030216098f, 0.0009084809571504593f, 0.0015157281886786222f, 0.0004032473370898515f, 0.0006078475271351635f, 0.000820512417703867f, 0.0005063702701590955f, 0.0003883673343807459f, 0.0007800688035786152f, 0.0006691224407404661f, 0.0007362322648987174f, 0.0006320811226032674f, 0.0006023174501024187f, 0.0005078957765363157f, 0.0005286829546093941f, 0.0008285552612505853f, 0.0010303343879058957f, 0.0008741041528992355f, 0.0007702631410211325f, 0.0006472881650552154f, 0.0003144891234114766f, 0.0006112640840001404f, 0.000674082082696259f, 0.0003785946173593402f, 0.0006631151190958917f, 0.00023852719459682703f, 0.0008563557057641447f, 0.0006989657995291054f, 0.00034039063029922545f, 0.0007288774359039962f, 0.0005461293621920049f, 0.0009782998822629452f, 0.0004418825847096741f, 0.00040339885163120925f, 0.0007700748974457383f, 0.0003152716381009668f, 0.000510058889631182f, 0.001027702703140676f, 0.0007444563671015203f, 0.0005894028581678867f, 0.0005779630737379193f, 0.0006243193638511002f, 0.00045934811350889504f, 0.0010299863060936332f, 0.0004642060666810721f, 0.0005867650033906102f, 0.0004683976876549423f, 0.0008069294271990657f, 0.0007789131486788392f, 0.0004827357770409435f, 0.0010221527190878987f, 0.0004938914207741618f, 0.0004608203307725489f, 0.00043237145291641355f, 0.0010949525749310851f, 0.0016478401375934482f, 0.0005137711414135993f, 0.00020819522615056485f, 0.0005154208629392087f, 0.0005986707983538508f, 0.0005613059620372951f, 0.0010511042783036828f, 0.0004746906051877886f, 0.0005132652586326003f, 0.0004442172357812524f, 0.0005841873935423791f, 0.0007377265719696879f, 0.0001722760935081169f, 0.0003902313474100083f, 0.00048203070764429867f, 0.00030188378877937794f, 0.0006509418017230928f, 0.0005874456837773323f, 0.0006412194925360382f, 0.00023567194875795394f, 0.00044642447028309107f, 0.0006788730970583856f, 0.00019721902208402753f, 0.0007932181470096111f, 0.0008742348873056471f, 0.0006178789772093296f, 0.0003670678415801376f, 0.0009096950525417924f, 0.0011170662473887205f, 0.0004995507770217955f, 0.0008317698375321925f, 0.000648157496470958f, 0.0005848967120982707f, 0.0005680128233507276f, 0.0003595060552470386f, 0.00022588387946598232f, 0.0006111327675171196f, 0.0006034555844962597f, 0.0005158347194083035f, 0.0004865670343860984f, 0.0005206151399761438f, 0.0005202835891395807f, 0.00038425441016443074f, 0.00042599483276717365f, 0.000795157509855926f, 0.00043096658191643655f, 0.00037220155354589224f, 0.0013741719303652644f, 0.0004367081564851105f, 0.0004779240698553622f, 0.0004760767624247819f, 0.0004503381496760994f, 0.0008098940015770495f);
static const ai_layer_format_type conv2d_52_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_53_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_53_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_53_pad_before_t_in_0_shape_h_const_u32 = 3;

static const ai_u16 conv2d_53_t_in_0_shape_w_const_u16 = 5;
static const ai_u16 conv2d_53_t_in_0_shape_h_const_u16 = 5;
static const ai_u16 conv2d_53_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_53_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_53_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_53_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_53_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_53_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_53_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_53_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.004985629115253687f, 0.007049527019262314f, 0.008144812658429146f, 0.011169036850333214f, 0.011892310343682766f, 0.0056976438499987125f, 0.006722073070704937f, 0.010046345181763172f, 0.011634951457381248f, 0.010822836309671402f, 0.006196300964802504f, 0.008826294913887978f, 0.007739557884633541f, 0.007730822078883648f, 0.006722180172801018f, 0.006481509655714035f, 0.008137925527989864f, 0.00706252409145236f, 0.016230888664722443f, 0.006173203699290752f, 0.008096898905932903f, 0.005578517448157072f, 0.006871099583804607f, 0.002802913775667548f, 0.0057151890359818935f, 0.0053331283852458f, 0.0067436532117426395f, 0.011862554587423801f, 0.013558886013925076f, 0.011328211985528469f, 0.0076373424381017685f, 0.0059839216992259026f, 0.0037320859264582396f, 0.008490403182804585f, 0.008371541276574135f, 0.008980151265859604f, 0.0018178229220211506f, 0.011025970801711082f, 0.0055535766296088696f, 0.0018453975208103657f, 0.0031942862551659346f, 0.004310308024287224f, 0.004197231028228998f, 0.0018800859106704593f, 0.005657366942614317f, 0.004114443901926279f, 0.008592775091528893f, 0.013802745379507542f, 0.009964842349290848f, 0.00836924184113741f, 0.007186522241681814f, 0.002605935325846076f, 0.011588234454393387f, 0.013337573036551476f, 0.0029408184345811605f, 0.0034739235416054726f, 0.007374769542366266f, 0.0028564080130308867f, 0.00846325233578682f, 0.004003521054983139f, 0.01407158188521862f, 0.007761452812701464f, 0.007169226184487343f, 0.00785868801176548f, 0.01464233547449112f, 0.009381771087646484f, 0.0017025816487148404f, 0.009255674667656422f, 0.006245034281164408f, 0.00630280701443553f, 0.008303488604724407f, 0.015698591247200966f, 0.006187449675053358f, 0.005557654425501823f, 0.0030089926440268755f, 0.006794549059122801f, 0.03028763085603714f, 0.014124609529972076f, 0.004456313792616129f, 0.0069274078123271465f, 0.009174929931759834f, 0.004749088082462549f, 0.00447411322966218f, 0.014284840784966946f, 0.015667762607336044f, 0.010997922159731388f, 0.008890027180314064f, 0.006232018582522869f, 0.00872577540576458f, 0.0040137022733688354f, 0.0029945194255560637f, 0.006007905583828688f, 0.004749606363475323f, 0.012394462712109089f, 0.006906148511916399f, 0.009463125839829445f, 0.008053913712501526f, 0.006237138994038105f, 0.007970966398715973f, 0.003160637803375721f, 0.009954545646905899f, 0.0066687255166471004f, 0.007484191097319126f, 0.004391073249280453f, 0.006064123474061489f, 0.008314416743814945f, 0.0030915772076696157f, 0.003938654903322458f, 0.008627095259726048f, 0.0027528812643140554f, 0.009700862690806389f, 0.008385448716580868f, 0.007962982170283794f, 0.0031925381626933813f, 0.003342372365295887f, 0.007951418869197369f, 0.01591266319155693f, 0.010728472843766212f, 0.0016761106671765447f, 0.005882428493350744f, 0.012651517055928707f, 0.007609428372234106f, 0.004161313641816378f, 0.015468363650143147f, 0.0056295874528586864f, 0.013351128436625004f, 0.004835047759115696f, 0.008517040871083736f, 0.004347461741417646f, 0.008451932109892368f, 0.007330859545618296f, 0.0023631807416677475f, 0.009613506495952606f, 0.006109954323619604f, 0.00459222262725234f, 0.007614151109009981f, 0.014296825975179672f, 0.008826936595141888f, 0.017183735966682434f, 0.005531738046556711f, 0.016343241557478905f, 0.005806202534586191f, 0.009999201633036137f, 0.01733068749308586f, 0.0036949629429727793f, 0.009853348135948181f, 0.009020619094371796f, 0.00768277607858181f, 0.010092443786561489f, 0.007895384915173054f, 0.0033415057696402073f, 0.008746442385017872f, 0.006297416519373655f, 0.015060102567076683f, 0.008206810802221298f, 0.011173296719789505f, 0.013275542296469212f, 0.004220299422740936f, 0.007856507785618305f, 0.026930466294288635f, 0.008102493360638618f, 0.004488468170166016f, 0.010175512172281742f, 0.01415948010981083f, 0.011039096862077713f, 0.009727085940539837f, 0.007189895026385784f, 0.006218745373189449f, 0.008729948662221432f, 0.007893653586506844f, 0.0074039483442902565f, 0.0029026386328041553f, 0.004974531941115856f, 0.008184266276657581f, 0.009964114986360073f, 0.011715035885572433f, 0.007189026568084955f, 0.009929521009325981f, 0.009603646583855152f, 0.006187740713357925f, 0.005541462451219559f, 0.007706393022090197f, 0.008186697959899902f, 0.10176831483840942f, 0.007857737131416798f, 0.010723685845732689f, 0.006766393780708313f, 0.004936752840876579f, 0.011958887800574303f, 0.005106229335069656f, 0.014426263980567455f, 0.007287683431059122f, 0.013426706194877625f, 0.004613527096807957f, 0.015846382826566696f, 0.006275924388319254f, 0.004730154760181904f, 0.008453186601400375f, 0.002877740189433098f, 0.009853919968008995f, 0.0060850768350064754f, 0.01742202788591385f, 0.016164109110832214f, 0.027776572853326797f, 0.007895110175013542f, 0.010870029218494892f, 0.007225857116281986f, 0.0041315509006381035f, 0.009923188015818596f, 0.011210810393095016f, 0.00730275409296155f, 0.009163737297058105f, 0.003869804786518216f, 0.013825645670294762f, 0.0057664853520691395f, 0.009592132642865181f, 0.0070371781475842f, 0.014107625000178814f, 0.005806532688438892f, 0.004430691245943308f, 0.006667625159025192f, 0.017639677971601486f, 0.005268455948680639f, 0.0038270335644483566f, 0.008665897883474827f, 0.020180964842438698f, 0.005745360627770424f, 0.006645678076893091f, 0.0067773256450891495f, 0.002793334424495697f, 0.01130608469247818f, 0.01172797754406929f, 0.014215076342225075f, 0.0066521428525447845f, 0.0109322564676404f, 0.012767698615789413f, 0.010626055300235748f, 0.007057931274175644f, 0.014314482919871807f, 0.012807399034500122f, 0.006612950935959816f, 0.008283623494207859f, 0.0071477023884654045f, 0.012431478127837181f, 0.012582250870764256f, 0.006325938273221254f, 0.011706729419529438f, 0.005977930501103401f, 0.009024007245898247f, 0.009774056263267994f, 0.005392040591686964f, 0.007776227314025164f, 0.008188372477889061f, 0.00836185086518526f, 0.003885917365550995f, 0.013672218658030033f, 0.012444112449884415f, 0.009834804572165012f, 0.007723135873675346f, 0.006648201495409012f, 0.007646746002137661f, 0.008531739003956318f, 0.005626422818750143f, 0.005707320291548967f, 0.0043015265837311745f, 0.02185545302927494f, 0.012672323733568192f, 0.007180613931268454f, 0.006796627771109343f, 0.012876549735665321f, 0.008425884880125523f, 0.0054604532197117805f, 0.009344677440822124f, 0.007824046537280083f, 0.01138292346149683f, 0.013712160289287567f, 0.004488997161388397f, 0.01541332807391882f, 0.00810411386191845f, 0.0035843669902533293f, 0.004737572278827429f, 0.004836401902139187f, 0.010462198406457901f, 0.016689477488398552f, 0.015631327405571938f, 0.01365656591951847f, 0.009349772706627846f, 0.005699009634554386f, 0.011179073713719845f, 0.00661528529599309f, 0.005518983118236065f, 0.00461951969191432f, 0.016210317611694336f, 0.0057927509769797325f, 0.004017784260213375f, 0.014948355033993721f, 0.007535729091614485f, 0.011401663534343243f, 0.013953662477433681f, 0.009510565549135208f, 0.004709645640105009f, 0.009058363735675812f, 0.014281030744314194f, 0.009338575415313244f, 0.008554995991289616f, 0.02324359491467476f, 0.007230388466268778f, 0.006347567308694124f, 0.010009064339101315f, 0.009108521044254303f, 0.007956603541970253f, 0.008036910556256771f, 0.009820247069001198f, 0.009419012814760208f, 0.012349415570497513f, 0.010689630173146725f, 0.007476128172129393f, 0.011484396643936634f, 0.009065031073987484f, 0.008509505540132523f, 0.007654349319636822f, 0.00608071917667985f, 0.007562538143247366f, 0.005506650544703007f, 0.0075212339870631695f, 0.011064804159104824f, 0.00873300340026617f, 0.006157928146421909f, 0.0038347917143255472f, 0.009278681129217148f, 0.010003820061683655f, 0.018040616065263748f, 0.0075356438755989075f, 0.015195676125586033f, 0.010229847393929958f, 0.0072231474332511425f);
static const ai_u16 conv2d_53_t_out_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_53_t_out_0_shape_h_const_u16 = 3;

static const ai_u16 conv2d_54_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_54_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_54_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_54_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_54_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_54_t_out_0_shape_ch_const_u16 = 56;
static const ai_i8 conv2d_54_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_54_t_out_0_fmt_zero_const_s8 = -10;
static const ai_float conv2d_54_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_54_t_out_0_fmt_scale_const_f32 = 0.1923304796218872f;
static const ai_float conv2d_54_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.005952573847025633f, 0.007328647654503584f, 0.010206098668277264f, 0.004287115763872862f, 0.005578349810093641f, 0.004503092262893915f, 0.00905909389257431f, 0.009465143084526062f, 0.004243234638124704f, 0.010894753970205784f, 0.013387537561357021f, 0.005018715746700764f, 0.00847125705331564f, 0.005343730095773935f, 0.004186044912785292f, 0.004137171432375908f, 0.01163631584495306f, 0.008479845710098743f, 0.004682554863393307f, 0.01450236514210701f, 0.005584117490798235f, 0.007741802372038364f, 0.01342736929655075f, 0.007378918584436178f, 0.0067600891925394535f, 0.005700161214917898f, 0.007184943184256554f, 0.007881206460297108f, 0.004091257695108652f, 0.0032599549740552902f, 0.013101056218147278f, 0.003626966616138816f, 0.0057241362519562244f, 0.004157618619501591f, 0.009239674545824528f, 0.00552951917052269f, 0.011658902280032635f, 0.0036093704402446747f, 0.006941957399249077f, 0.005278483033180237f, 0.005829744040966034f, 0.004713923204690218f, 0.0035179711412638426f, 0.0060344114899635315f, 0.005071031395345926f, 0.004822274204343557f, 0.014355921186506748f, 0.0050781117752194405f, 0.015687545761466026f, 0.00496154697611928f, 0.008538828231394291f, 0.010632908903062344f, 0.007512915413826704f, 0.005426902789622545f, 0.00419593695551157f, 0.006769475061446428f);
static const ai_layer_format_type conv2d_54_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_56_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_56_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_56_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_56_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_56_t_in_0_shape_ch_const_u16 = 56;
static const ai_u16 conv2d_56_t_out_0_shape_ch_const_u16 = 336;
static const ai_i8 conv2d_56_t_in_0_fmt_zero_const_s8 = -10;
static const ai_i8 conv2d_56_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_56_t_in_0_fmt_scale_const_f32 = 0.1923304796218872f;
static const ai_float conv2d_56_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_56_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.00027276703622192144f, 0.0003688308352138847f, 0.0006433676462620497f, 0.0005393258761614561f, 0.00038352166302502155f, 0.0006662853993475437f, 0.0007618375238962471f, 0.0006253454484976828f, 0.0006105253123678267f, 0.0003904429613612592f, 0.0007238870020955801f, 0.00012075134145561606f, 0.0006268221186473966f, 0.000380303303245455f, 0.0005931947962380946f, 0.0007662053685635328f, 0.000633661518804729f, 0.0005169903743080795f, 0.0007642764248885214f, 0.0003060353919863701f, 0.0012747395085170865f, 0.00038518995279446244f, 0.0008183253812603652f, 0.00021604749781545252f, 0.0004865156952291727f, 0.00048100482672452927f, 0.0003453748649917543f, 0.0005030559259466827f, 0.0006350342882797122f, 0.0006066909991204739f, 0.00044043295201845467f, 0.0005077616078779101f, 0.0007394845597445965f, 0.0006131098489277065f, 0.00032310737879015505f, 0.0003909546067006886f, 0.0006265034899115562f, 0.0008171373629011214f, 0.0002455847570672631f, 0.0006892954115755856f, 0.0005044867284595966f, 0.00042401114478707314f, 0.0006232112646102905f, 0.0005268500535748899f, 0.0006274849874898791f, 0.0005806601839140058f, 0.00042567693162709475f, 0.0007581871468573809f, 0.00022922377684153616f, 0.0008982860599644482f, 0.0006947082583792508f, 0.0003638372290879488f, 0.0002560239809099585f, 0.0012064115144312382f, 0.00036284574889577925f, 0.0005087662138976157f, 0.0006809909245930612f, 0.0003356569795869291f, 0.0003145507362205535f, 0.000849905249197036f, 0.0007649398758076131f, 0.0002864103007595986f, 0.0005322651704773307f, 0.0009355835500173271f, 0.0005156829720363021f, 0.0005410572630353272f, 0.0004627395246643573f, 0.0005067044985480607f, 0.00035883128293789923f, 0.000720108684618026f, 0.00045901405974291265f, 0.000566685339435935f, 0.0009391061612404883f, 0.0009005096508190036f, 0.00033628824166953564f, 0.0005625428748317063f, 0.000615050841588527f, 0.0008019017404876649f, 0.0007432050770148635f, 0.0004306295595597476f, 0.0004447170067578554f, 0.0003726491122506559f, 0.0003038860158994794f, 0.0004189124738331884f, 0.0006512976251542568f, 0.0006598221370950341f, 0.0007399708847515285f, 0.0006863432936370373f, 0.0007825908833183348f, 0.0006170037668198347f, 0.0003935165877919644f, 0.0004604013229254633f, 0.00042933921213261783f, 0.0007300462457351387f, 0.0002913933130912483f, 0.00022989972785580903f, 0.0004502786323428154f, 0.0004094077739864588f, 0.0007600964745506644f, 0.000543455418664962f, 0.0006707895663566887f, 0.0004604178830049932f, 0.0005635779234580696f, 0.0003649764403235167f, 0.0006126437801867723f, 0.0006692894385196269f, 0.0004829300451092422f, 0.0005573491216637194f, 0.0007248368347063661f, 0.000464817276224494f, 0.000664114486426115f, 0.00039019572432152927f, 0.0005738245090469718f, 0.0006370110204443336f, 0.00024395561194978654f, 0.0005296582821756601f, 0.00038381796912290156f, 0.00027489548665471375f, 0.000631735660135746f, 0.0003498650621622801f, 0.0008264100179076195f, 0.0008225945639424026f, 0.000848163093905896f, 0.0009829060873016715f, 0.0006550558027811348f, 0.00038846232928335667f, 0.0004418500466272235f, 0.00022941140923649073f, 0.0006198848132044077f, 0.0004594501806423068f, 0.0007079598726704717f, 0.0005792892188765109f, 0.0003827115288004279f, 0.0006852374062873423f, 0.0007124125841073692f, 0.0006029437063261867f, 0.0004984150873497128f, 0.000603922875598073f, 0.0004639997787307948f, 0.0004802861949428916f, 0.0006391846109181643f, 0.00023705091734882444f, 0.000584787514526397f, 0.00047855539014562964f, 0.0006602945504710078f, 0.0007075005560182035f, 0.00021914839453529567f, 0.00044720456935465336f, 0.0006470951484516263f, 0.000625998480245471f, 0.0003972856793552637f, 0.0007318056304939091f, 0.0005149988573975861f, 0.000790449499618262f, 0.0006710473098792136f, 0.00026828667614609003f, 0.0007700032438151538f, 0.0006557171000167727f, 0.0005341872456483543f, 0.0007229226757772267f, 0.0005119816632941365f, 0.0004347397480159998f, 0.0004852328565903008f, 0.0007311120280064642f, 0.0005543644656427205f, 0.0005945109878666699f, 0.0007928068516775966f, 0.0008805074030533433f, 4.507831545197405e-05f, 0.0008050474571064115f, 0.0006854378734715283f, 0.0008313597645610571f, 0.0003102524788118899f, 0.0005543884471990168f, 0.0005501609412021935f, 0.0006206889520399272f, 0.0005900527467019856f, 0.00038697582203894854f, 0.0003264830447733402f, 0.0003484127519186586f, 0.00036557402927428484f, 0.0007582532707601786f, 0.0006776312366127968f, 0.0006434629904106259f, 0.0009366859449073672f, 0.000680126657243818f, 0.0005317789036780596f, 0.0007814415730535984f, 0.00019666859589051455f, 0.0004880947235506028f, 0.0006148726679384708f, 0.0003810749913100153f, 0.0011785097885876894f, 0.000377733405912295f, 0.0006301504909060895f, 0.0005740762571804225f, 0.0004947512061335146f, 0.0005055173533037305f, 0.0005043791607022285f, 0.0004398586170282215f, 0.0004093289899174124f, 0.0007529249414801598f, 0.00048692309064790606f, 0.00043291656766086817f, 0.0007339526200667024f, 0.0006361239938996732f, 0.00047325080959126353f, 0.0005842277896590531f, 0.0005150829674676061f, 0.0004283810849301517f, 0.0007266787579283118f, 0.0004848184180445969f, 0.0006907372153364122f, 0.0006091936957091093f, 0.000734893255867064f, 0.00040406384505331516f, 0.0004630862094927579f, 0.0005214585107751191f, 0.000803240982349962f, 0.00041477239574305713f, 0.0006031380617059767f, 0.0006447451887652278f, 0.00027146501815877855f, 0.0005752082797698677f, 0.0005082703428342938f, 0.0005250545218586922f, 0.00034986421815119684f, 0.0007688215700909495f, 0.00044539105147123337f, 0.0009080374729819596f, 0.00045266954111866653f, 0.0005920293042436242f, 0.0006034702528268099f, 0.0006501512252725661f, 0.0005405241390690207f, 0.0007026950479485095f, 0.0005672874394804239f, 0.00033150275703519583f, 0.0007050842395983636f, 0.00038552822661586106f, 0.0010363607434555888f, 0.0009228757699020207f, 0.000696041330229491f, 0.00040340106352232397f, 0.0007959555950947106f, 0.00032490998273715377f, 0.0006403555162250996f, 0.0008491898188367486f, 0.0005888216546736658f, 0.0006002596346661448f, 0.000357157172402367f, 0.0004969706642441452f, 0.0004569677694234997f, 0.00018941992311738431f, 0.00020759148173965514f, 0.0004370573442429304f, 0.00031224358826875687f, 0.0003967402735725045f, 0.00042013981146737933f, 0.0003196725738234818f, 0.00030446486198343337f, 0.00027344567934051156f, 0.0004617644299287349f, 0.0005133073427714407f, 0.0002468485035933554f, 0.00029781326884403825f, 0.00037393896491266787f, 0.0007704776944592595f, 0.0003074906417168677f, 0.0007767155184410512f, 0.0006372663774527609f, 0.0006242677918635309f, 0.0006228492129594088f, 0.0006708409637212753f, 0.0006859778077341616f, 0.00023860539658926427f, 0.0008827506680972874f, 0.0002243531635031104f, 0.0005681355833075941f, 0.000768192985560745f, 0.0003751808253582567f, 0.000426934304414317f, 0.000249095173785463f, 0.0005721002235077322f, 0.0003385381423868239f, 0.0006195065216161311f, 0.00039936459506861866f, 0.0005184437613934278f, 0.000623681815341115f, 0.0003464131150394678f, 0.0006889270152896643f, 0.0003864178725052625f, 0.0005625976482406259f, 0.0007049617706798017f, 0.0007630061591044068f, 0.0009692384046502411f, 0.0003769837785512209f, 0.00031881482573226094f, 0.0004004225193057209f, 0.0005376936751417816f, 0.0005211406387388706f, 0.0002735862508416176f, 0.0005691443802788854f, 0.0005787975969724357f, 0.00042463798308745027f, 0.0006469271029345691f, 0.0003921539173461497f, 0.0010953487362712622f, 0.0006684184772893786f, 0.0003221018414478749f, 0.0004398351884447038f, 0.0005525384331122041f, 0.0006938357255421579f, 0.0004662766004912555f, 0.0008471840992569923f, 0.00043632552842609584f, 0.000434475572546944f, 0.0005294877337291837f, 0.0007172304904088378f, 0.0002516809618100524f, 0.00030298338970169425f, 0.0005369389546103776f, 0.00027610972756519914f, 0.0006401057471521199f, 0.0006204874371178448f, 0.0003790798655245453f, 0.00037964090006425977f, 0.000641679740510881f, 0.000641835096757859f, 0.0004404533829074353f, 0.000904868938960135f, 0.0008240019087679684f, 0.00042865093564614654f, 0.00044603284914046526f, 0.001444981200620532f, 0.0006251051090657711f);
static const ai_layer_format_type conv2d_56_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_57_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_57_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_57_pad_before_t_in_0_shape_h_const_u32 = 3;

static const ai_u16 conv2d_57_t_in_0_shape_w_const_u16 = 5;
static const ai_u16 conv2d_57_t_in_0_shape_h_const_u16 = 5;
static const ai_u16 conv2d_57_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_57_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_57_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_57_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_57_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_57_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_57_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_57_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.007439701352268457f, 0.024387290701270103f, 0.004872194956988096f, 0.006465050857514143f, 0.00411494355648756f, 0.006293666083365679f, 0.005618090741336346f, 0.005111583974212408f, 0.006495030131191015f, 0.015444322489202023f, 0.008093221113085747f, 0.04166002944111824f, 0.006187824532389641f, 0.007324793841689825f, 0.0061113531701266766f, 0.007330636959522963f, 0.008669243194162846f, 0.010581482201814651f, 0.008258960209786892f, 0.0305828507989645f, 0.0030487726908177137f, 0.009589442051947117f, 0.006963661871850491f, 0.011062892153859138f, 0.020355600863695145f, 0.00980041641741991f, 0.005984902381896973f, 0.008181559853255749f, 0.009151712991297245f, 0.014975435100495815f, 0.006513278465718031f, 0.004849469289183617f, 0.0037121193017810583f, 0.006601910572499037f, 0.009636159054934978f, 0.021461021155118942f, 0.007207354065030813f, 0.00508304126560688f, 0.010824308730661869f, 0.010645205155014992f, 0.007297700736671686f, 0.012056317180395126f, 0.006467655301094055f, 0.00554502010345459f, 0.007814791984856129f, 0.02078886143863201f, 0.0112002557143569f, 0.0037151307333260775f, 0.00954387616366148f, 0.0066797928884625435f, 0.0088051687926054f, 0.006627637427300215f, 0.024649523198604584f, 0.010598338209092617f, 0.007345590740442276f, 0.0025750300846993923f, 0.005063442513346672f, 0.0046029225923120975f, 0.010972656309604645f, 0.005152927711606026f, 0.008783187717199326f, 0.00737368268892169f, 0.006812565494328737f, 0.005848604254424572f, 0.009747126139700413f, 0.0042173778638243675f, 0.008110919035971165f, 0.008927198126912117f, 0.013510484248399734f, 0.00894732866436243f, 0.007725318428128958f, 0.01218478474766016f, 0.007022468373179436f, 0.007427691016346216f, 0.014400915242731571f, 0.0015116040594875813f, 0.003450885182246566f, 0.008603908121585846f, 0.012139721773564816f, 0.008025843650102615f, 0.012086926959455013f, 0.009447474963963032f, 0.010497652925550938f, 0.010918094776570797f, 0.002251059515401721f, 0.0036186594516038895f, 0.003383486997336149f, 0.007732811849564314f, 0.013955284841358662f, 0.0031628631986677647f, 0.013447469100356102f, 0.005490022245794535f, 0.006986008957028389f, 0.0015297237550839782f, 0.009386847727000713f, 0.011318475008010864f, 0.008589481934905052f, 0.019359635189175606f, 0.006262480281293392f, 0.008096632547676563f, 0.00756102567538619f, 0.0058011868968605995f, 0.01215384341776371f, 0.0059296004474163055f, 0.005801607854664326f, 0.006301322486251593f, 0.013326693326234818f, 0.005369587801396847f, 0.006744793616235256f, 0.0152058694511652f, 0.00787840224802494f, 0.003541494021192193f, 0.01154189184308052f, 0.0054408349096775055f, 0.027368592098355293f, 0.004231502301990986f, 0.006680748891085386f, 0.01609533093869686f, 0.03934817761182785f, 0.01589730568230152f, 0.0057666911743581295f, 0.008516035042703152f, 0.002183334669098258f, 0.006647375412285328f, 0.004331965930759907f, 0.007112136576324701f, 0.0054598585702478886f, 0.011794609017670155f, 0.005203110631555319f, 0.008980038575828075f, 0.0042324308305978775f, 0.005714224651455879f, 0.012662830762565136f, 0.008339295163750648f, 0.0069173360243439674f, 0.007024743594229221f, 0.007279833313077688f, 0.004443153738975525f, 0.007975926622748375f, 0.0047431751154363155f, 0.00390991335734725f, 0.01311661209911108f, 0.011074202135205269f, 0.007682609371840954f, 0.0032779965549707413f, 0.008787527680397034f, 0.010794778354465961f, 0.011457676999270916f, 0.009136955253779888f, 0.010118445381522179f, 0.006928186863660812f, 0.0030838276725262403f, 0.006429200526326895f, 0.004568880423903465f, 0.005185512360185385f, 0.006821794901043177f, 0.0036722251679748297f, 0.013145205564796925f, 0.007676388137042522f, 0.0036788680590689182f, 0.013862084597349167f, 0.0040365285240113735f, 0.012242191471159458f, 0.0037431998644024134f, 0.014179687947034836f, 0.004120889585465193f, 0.004418670665472746f, 0.009177561849355698f, 0.23728813230991364f, 0.007766561582684517f, 0.005787866190075874f, 0.0025044868234544992f, 0.007399165071547031f, 0.004864478018134832f, 0.00740891182795167f, 0.004979575052857399f, 0.01848778873682022f, 0.007137950975447893f, 0.016470815986394882f, 0.013092143461108208f, 0.01279241032898426f, 0.002487877383828163f, 0.0066612642258405685f, 0.005878973752260208f, 0.005204644985496998f, 0.004725708160549402f, 0.00789818074554205f, 0.006138967350125313f, 0.016740405932068825f, 0.005344520788639784f, 0.00377154559828341f, 0.007221339736133814f, 0.006175314076244831f, 0.025498365983366966f, 0.004951560869812965f, 0.008305806666612625f, 0.008893260732293129f, 0.007033572532236576f, 0.00325478776358068f, 0.009491657838225365f, 0.01683696359395981f, 0.008858823217451572f, 0.008173924870789051f, 0.0075621223077178f, 0.004152978304773569f, 0.008562134578824043f, 0.007305309642106295f, 0.0025355264078825712f, 0.013402510434389114f, 0.018938470631837845f, 0.006462277378886938f, 0.006835546810179949f, 0.0028344939928501844f, 0.010226177982985973f, 0.004470823332667351f, 0.007599686272442341f, 0.008371667936444283f, 0.01087171770632267f, 0.0020498952362686396f, 0.010477087460458279f, 0.007797235157340765f, 0.004867049865424633f, 0.014945625327527523f, 0.00864251609891653f, 0.003641013987362385f, 0.0050108530558645725f, 0.014447114430367947f, 0.006473942194133997f, 0.005080359522253275f, 0.006686914712190628f, 0.01855386421084404f, 0.008070589043200016f, 0.009354183450341225f, 0.004403379745781422f, 0.007419245317578316f, 0.008808339945971966f, 0.008647685870528221f, 0.017116744071245193f, 0.008211818523705006f, 0.006999130826443434f, 0.002678302116692066f, 0.007459511514753103f, 0.008016211912035942f, 0.011131986044347286f, 0.009117981418967247f, 0.010653063654899597f, 0.012105566449463367f, 0.0034501031041145325f, 0.024001402780413628f, 0.004192135762423277f, 0.015248885378241539f, 0.005322341341525316f, 0.006206778343766928f, 0.015112238936126232f, 0.007179887033998966f, 0.01293284073472023f, 0.022579582408070564f, 0.01734152063727379f, 0.0038089253939688206f, 0.009432502090930939f, 0.00982122402638197f, 0.0058412207290530205f, 0.0020662795286625624f, 0.005200050305575132f, 0.012539257295429707f, 0.008503318764269352f, 0.006880232132971287f, 0.0037875056732445955f, 0.010941040702164173f, 0.003929516300559044f, 0.01106507983058691f, 0.005316467024385929f, 0.010248396545648575f, 0.0028792363591492176f, 0.004592640791088343f, 0.020914776250720024f, 0.0018331985920667648f, 0.010947334580123425f, 0.004375220276415348f, 0.002175142290070653f, 0.008999031968414783f, 0.008859136141836643f, 0.010218605399131775f, 0.006884977221488953f, 0.011147440411150455f, 0.010411208495497704f, 0.00404570484533906f, 0.00769527954980731f, 0.04838215932250023f, 0.018773695454001427f, 0.00506872683763504f, 0.017926707863807678f, 0.006126811727881432f, 0.0009409935446456075f, 0.007930993102490902f, 0.008017879910767078f, 0.006901577115058899f, 0.009738496504724026f, 0.008179997093975544f, 0.003983245696872473f, 0.0029947462026029825f, 0.01081107184290886f, 0.00969215389341116f, 0.010214285925030708f, 0.009538734331727028f, 0.00567357474938035f, 0.005633873399347067f, 0.0074692764319479465f, 0.011959046125411987f, 0.01764828898012638f, 0.007392149418592453f, 0.013962024822831154f, 0.006665023509413004f, 0.007837275043129921f, 0.012541147880256176f, 0.011143987067043781f, 0.005296193063259125f, 0.008508128114044666f, 0.002390439622104168f, 0.010397168807685375f, 0.013262990862131119f, 0.006114907097071409f, 0.014178904704749584f, 0.0105180274695158f, 0.01196602825075388f, 0.00853497814387083f, 0.016290493309497833f, 0.00915925856679678f, 0.012964732013642788f, 0.014222820289433002f, 0.0072943419218063354f, 0.0035737636499106884f, 0.007450466509908438f, 0.003100726054981351f, 0.0012179762125015259f, 0.006368205416947603f);
static const ai_u16 conv2d_57_t_out_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_57_t_out_0_shape_h_const_u16 = 3;

static const ai_u16 conv2d_58_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_58_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_58_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_58_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_58_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_58_t_out_0_shape_ch_const_u16 = 56;
static const ai_i8 conv2d_58_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_58_t_out_0_fmt_zero_const_s8 = 0;
static const ai_float conv2d_58_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_58_t_out_0_fmt_scale_const_f32 = 0.22229193150997162f;
static const ai_float conv2d_58_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.010458688251674175f, 0.012289855629205704f, 0.011741310358047485f, 0.00718075642362237f, 0.00758621608838439f, 0.005704180803149939f, 0.011267698369920254f, 0.014724335633218288f, 0.006334240082651377f, 0.014498775824904442f, 0.01741517335176468f, 0.006707967724651098f, 0.008419481106102467f, 0.0077170561999082565f, 0.006548824720084667f, 0.005940933711826801f, 0.012229721061885357f, 0.009340719319880009f, 0.00635591521859169f, 0.01828855089843273f, 0.005638014990836382f, 0.0068938108161091805f, 0.011579588986933231f, 0.007886002771556377f, 0.007938827387988567f, 0.010821017436683178f, 0.008063769899308681f, 0.008490707725286484f, 0.00669627171009779f, 0.007497393526136875f, 0.03138245269656181f, 0.007455224636942148f, 0.008021933026611805f, 0.006312413606792688f, 0.005687888246029615f, 0.0062121739611029625f, 0.01385002862662077f, 0.008111770264804363f, 0.005642303265631199f, 0.010313712991774082f, 0.010149328969419003f, 0.007274078670889139f, 0.009463157504796982f, 0.009019731543958187f, 0.009426901116967201f, 0.0062392824329435825f, 0.02558158151805401f, 0.008950775489211082f, 0.01696404255926609f, 0.008816675283014774f, 0.007699268404394388f, 0.007227625697851181f, 0.01286141574382782f, 0.008631954900920391f, 0.005777081474661827f, 0.006966948974877596f);
static const ai_layer_format_type conv2d_58_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;


static const ai_u16 conv2d_60_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_60_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_60_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_60_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_60_t_in_0_shape_ch_const_u16 = 56;
static const ai_u16 conv2d_60_t_out_0_shape_ch_const_u16 = 336;
static const ai_i8 conv2d_60_t_in_0_fmt_zero_const_s8 = 0;
static const ai_i8 conv2d_60_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_60_t_in_0_fmt_scale_const_f32 = 0.22229193150997162f;
static const ai_float conv2d_60_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_60_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0013153688050806522f, 0.0005152380326762795f, 0.0004171458713244647f, 0.0006955028511583805f, 0.0007657159585505724f, 0.0004818440938834101f, 0.0007861441699787974f, 0.0007169824093580246f, 0.0005505862063728273f, 0.0006060124142095447f, 0.000510645448230207f, 0.0005637842696160078f, 0.0003998178872279823f, 0.0004862627247348428f, 0.0006785289733670652f, 0.0006661981460638344f, 0.0006035161786712706f, 0.0006967916269786656f, 0.0004197768575977534f, 0.000540509819984436f, 0.0004754814726766199f, 0.00028454637504182756f, 0.0010513078887015581f, 0.000841634813696146f, 0.0007253636140376329f, 0.0007334963302128017f, 0.000623982516117394f, 0.00048564281314611435f, 0.0005988601478748024f, 0.0007394733838737011f, 0.00044498741044662893f, 0.0006714058690704405f, 0.0010468694381415844f, 0.0006191962165758014f, 0.0008136961259879172f, 0.0011604350293055177f, 0.0006031891680322587f, 0.00067366030998528f, 0.0007429782999679446f, 0.0006618714542128146f, 0.0008898384403437376f, 0.000725291611161083f, 0.0004336602578405291f, 0.0012397965183481574f, 0.0009293949115090072f, 0.00043543518404476345f, 0.00027433628565631807f, 0.0006669932045042515f, 0.0005727839306928217f, 0.000723494915291667f, 0.0007156679639592767f, 0.0005684756324626505f, 0.0006651498842984438f, 0.0005702292546629906f, 0.0006085055647417903f, 0.001050357474014163f, 0.00033122801687568426f, 0.0005163339665159583f, 0.000646910339128226f, 0.0005668562953360379f, 0.0010359594598412514f, 0.0017290242249146104f, 0.0008945511654019356f, 0.00033336522756144404f, 0.0005648150108754635f, 0.0007498472696170211f, 0.0009923960315063596f, 0.0007212504278868437f, 0.00048355190665461123f, 0.0007175701903179288f, 0.0014713812852278352f, 0.0005741772474721074f, 0.0005574921960942447f, 0.0006182134384289384f, 0.0007545235566794872f, 0.0006219127681106329f, 0.0006397098768502474f, 0.0007398235029540956f, 0.0006756286020390689f, 0.0006019797874614596f, 0.0009708787547424436f, 0.0006620213389396667f, 0.0006683090468868613f, 0.0005415461491793394f, 0.0015772012993693352f, 0.0006217434420250356f, 0.0008400445221923292f, 0.000614625052548945f, 0.0008401438244618475f, 0.00048326593241654336f, 0.0007473065634258091f, 0.0007505792309530079f, 0.0005623803590424359f, 0.00043363001896068454f, 0.0005945041193626821f, 0.0005532083450816572f, 0.0008344558300450444f, 0.0007349987863563001f, 0.0003971183905377984f, 0.0005504749133251607f, 0.0006091042887419462f, 0.0004531337763182819f, 0.0006778060342185199f, 0.0008138223201967776f, 0.0008588657365180552f, 0.0007936075562611222f, 0.0007572754984721541f, 0.0006303759873844683f, 0.0009283472900278866f, 0.0008423864492215216f, 0.000615122087765485f, 0.0009903254685923457f, 0.0009044543839991093f, 0.0009302119142375886f, 0.0005212727119214833f, 0.00041218523983843625f, 0.0005052086198702455f, 0.0006109104142524302f, 0.0005427654832601547f, 0.0007173423655331135f, 0.0006786581361666322f, 0.0010805227793753147f, 0.0009088394581340253f, 0.0007180784014053643f, 0.0004699802375398576f, 0.0004920672508887947f, 0.0007326271152123809f, 0.0006141632911749184f, 0.0005834567127749324f, 0.0006269777077250183f, 0.000476460118079558f, 0.0005375753971748054f, 0.0009679337381385267f, 0.0006303182453848422f, 0.0005844365223310888f, 0.0005518312100321054f, 0.0004802502226084471f, 0.0008071330958046019f, 0.0005274369614198804f, 0.0005308075342327356f, 0.0008528666803613305f, 0.00107922637835145f, 0.0008298722095787525f, 0.0009293188923038542f, 0.0005075364606454968f, 0.0005929199978709221f, 0.0006149387918412685f, 0.0005026206490583718f, 0.0006189077976159751f, 0.0006031850352883339f, 0.0007544754771515727f, 0.0005425174604170024f, 0.0004838087479583919f, 0.0007550473092123866f, 0.000606643792707473f, 0.0006149776745587587f, 0.0006951821851544082f, 0.0008823752868920565f, 0.0004576682113111019f, 0.0005767893744632602f, 0.0005180433508940041f, 0.0007201640983112156f, 0.0004612335760612041f, 0.0007676875102333724f, 0.0005778362974524498f, 0.0005085196462459862f, 0.0008177761337719858f, 0.0005390403675846756f, 0.0011112638749182224f, 0.0012682321248576045f, 0.00037695016362704337f, 0.00036913881194777787f, 0.0005342478398233652f, 0.00027187971863895655f, 0.0005545993335545063f, 0.0005555007955990732f, 0.0005772921722382307f, 0.000886324152816087f, 0.0005610772641375661f, 0.00048484530998393893f, 0.00046754057984799147f, 0.0005072877393104136f, 0.00048027621232904494f, 0.0005211142706684768f, 0.0006637126207351685f, 0.0005615318659693003f, 0.0006623723893426359f, 0.0007878268952481449f, 0.0004596298676915467f, 0.0005214186967350543f, 0.0005558590055443347f, 0.0004806786309927702f, 0.00042615330312401056f, 0.0005371987354010344f, 0.0006442359299398959f, 0.0007328712963499129f, 0.0006198156042955816f, 0.0005101169808767736f, 0.0005544619052670896f, 0.0005968000623397529f, 0.0006894366233609617f, 0.000653006718493998f, 0.0005036491784267128f, 0.0006367203895933926f, 0.0008803477394394577f, 0.0008240090101025999f, 0.00034927361411973834f, 0.0004542261885944754f, 0.0007291481597349048f, 0.0007422951166518033f, 0.0006227540434338152f, 0.00039976713014766574f, 0.0005458605592139065f, 0.0006622861255891621f, 0.0005748352268710732f, 0.0005933773936703801f, 0.0008786801481619477f, 0.0006544053321704268f, 0.0005835244664922357f, 0.0006578853353857994f, 0.0005957871326245368f, 0.0007584751001559198f, 0.0005522043793462217f, 0.0005856830975972116f, 0.0007761664455756545f, 0.0002296076127095148f, 0.0007712829392403364f, 0.0005279285251162946f, 0.0005253505660220981f, 0.0009059326839633286f, 0.0006900954176671803f, 0.0007531538140028715f, 0.001014118199236691f, 0.0006777974194847047f, 0.0005319926422089338f, 0.0006924633635208011f, 0.0006366948946379125f, 0.0003862474986817688f, 0.0004413939896039665f, 0.0003419164859224111f, 0.0007490325369872153f, 0.0005921120173297822f, 0.0009664620738476515f, 0.000618993304669857f, 0.0005599930300377309f, 0.00026720567257143557f, 0.00048212180263362825f, 0.0012294859625399113f, 0.00027980803861282766f, 0.000752077903598547f, 0.0005683989147655666f, 0.00046212910092435777f, 0.0006157176103442907f, 0.0003992832498624921f, 0.0006202036747708917f, 0.0009418798144906759f, 0.000332647527102381f, 0.00034605988184921443f, 0.0006624322850257158f, 0.0006345233414322138f, 0.00064155861036852f, 0.0005647228099405766f, 0.00027085779584012926f, 0.0006146007217466831f, 0.000672996393404901f, 0.00048229581443592906f, 0.00038827676326036453f, 0.0009420731221325696f, 0.0004818060260731727f, 0.0009938734583556652f, 0.0003606607497204095f, 0.0005954002263024449f, 0.0006532213883474469f, 0.0005291351699270308f, 0.0005958824185654521f, 0.0005487788585014641f, 0.0007597513613291085f, 0.00047375145368278027f, 0.0010771637316793203f, 0.000781975279096514f, 0.0005966890603303909f, 0.000787003431469202f, 0.0005643228068947792f, 0.0007202727720141411f, 0.0004786139470525086f, 0.00031115752062760293f, 0.0010020346380770206f, 0.0005312360008247197f, 0.0006572400452569127f, 0.0003975393483415246f, 0.0005808647838421166f, 0.0005000823875889182f, 0.000329603353748098f, 0.000532314064912498f, 0.0005890918546356261f, 0.0007312212255783379f, 0.000584833906032145f, 0.001080756657756865f, 0.0007302341400645673f, 0.0005542105645872653f, 0.0004783212498296052f, 0.0005338133196346462f, 0.00030391785548999906f, 0.00045882316771894693f, 0.0004970448790118098f, 0.0006988027598708868f, 0.0005275552393868566f, 0.0007504387758672237f, 0.0004755089175887406f, 0.0004801726609002799f, 0.0003763945715036243f, 0.0004466001992113888f, 0.00040885049384087324f, 0.00036754185566678643f, 0.0009791835909709334f, 0.0009880643337965012f, 0.0007238942198455334f, 0.0004968674038536847f, 0.0007183596026152372f, 0.00081630953354761f, 0.0005735153681598604f, 0.0010428469395264983f, 0.0005785963148809969f, 0.0007524871616624296f, 0.0005061146221123636f, 0.0009162347996607423f, 0.00034597099875099957f, 0.0005730115808546543f, 0.00035145264700986445f, 0.000615360215306282f, 0.0006786503945477307f, 0.0011307457461953163f, 0.000794524559751153f, 0.000716081412974745f, 0.0005411430029198527f, 0.0006156868766993284f);
static const ai_layer_format_type conv2d_60_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_i8 conv2d_61_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_61_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_61_pad_before_t_in_0_shape_h_const_u32 = 3;

static const ai_u16 conv2d_61_t_in_0_shape_w_const_u16 = 5;
static const ai_u16 conv2d_61_t_in_0_shape_h_const_u16 = 5;
static const ai_u16 conv2d_61_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_61_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_61_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_61_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_61_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_61_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_61_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_61_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.007619256153702736f, 0.006655739154666662f, 0.007459786254912615f, 0.008488543331623077f, 0.04569759964942932f, 0.0055036949925124645f, 0.006506187375634909f, 0.007357408292591572f, 0.0074016023427248f, 0.0037897403817623854f, 0.005972469225525856f, 0.008842968381941319f, 0.007448650896549225f, 0.00627856096252799f, 0.00706373481079936f, 0.00811285525560379f, 0.005224218592047691f, 0.003743015229701996f, 0.00639032106846571f, 0.006758110132068396f, 0.008160472847521305f, 0.013833936303853989f, 0.003092653350904584f, 0.006853215862065554f, 0.006029676180332899f, 0.00588731374591589f, 0.007128378376364708f, 0.009664538316428661f, 0.005499209277331829f, 0.004151932895183563f, 0.009588036686182022f, 0.005209974478930235f, 0.004780150484293699f, 0.006228295154869556f, 0.005990947596728802f, 0.0019115251488983631f, 0.0064347879961133f, 0.008297997526824474f, 0.007732486352324486f, 0.006796959321945906f, 0.0038675356190651655f, 0.005416941363364458f, 0.008145688101649284f, 0.004148427862673998f, 0.0036870590411126614f, 0.005595280323177576f, 0.012288111262023449f, 0.003592756111174822f, 0.010397203266620636f, 0.0036172757390886545f, 0.0069103664718568325f, 0.008765947073698044f, 0.0063835131004452705f, 0.007683304138481617f, 0.0038060941733419895f, 0.0038976045325398445f, 0.007696529850363731f, 0.007664306554943323f, 0.01587190106511116f, 0.00811636634171009f, 0.006986367981880903f, 0.002482087118551135f, 0.005173265002667904f, 0.006542708724737167f, 0.003924850840121508f, 0.0040656644850969315f, 0.0046396516263484955f, 0.008683734573423862f, 0.006553631741553545f, 0.0061075082048773766f, 0.0031244668643921614f, 0.008559000678360462f, 0.006771540269255638f, 0.005575674585998058f, 0.00563934538513422f, 0.00918601080775261f, 0.010019559413194656f, 0.007180505897849798f, 0.00790055375546217f, 0.00520448200404644f, 0.005759858526289463f, 0.00631077541038394f, 0.00307052256539464f, 0.008718143217265606f, 0.002848142758011818f, 0.00782934669405222f, 0.007233249023556709f, 0.008447961881756783f, 0.0026318198069930077f, 0.004980767611414194f, 0.008759862743318081f, 0.007878321222960949f, 0.009081556461751461f, 0.0054335216991603374f, 0.00875869207084179f, 0.0066462731920182705f, 0.005940136965364218f, 0.007492744829505682f, 0.00995948351919651f, 0.005238731391727924f, 0.0067968955263495445f, 0.006273818202316761f, 0.0071917446330189705f, 0.0077214608900249004f, 0.0074958656914532185f, 0.004998865071684122f, 0.004958993289619684f, 0.005296504124999046f, 0.006414726842194796f, 0.0097722252830863f, 0.005586226936429739f, 0.004532113205641508f, 0.0076121375896036625f, 0.0055047906935215f, 0.007471906021237373f, 0.007023016922175884f, 0.004476166795939207f, 0.004713580012321472f, 0.005629664286971092f, 0.008083012886345387f, 0.006436575669795275f, 0.0035928781144320965f, 0.0030356384813785553f, 0.00534016964957118f, 0.006248061079531908f, 0.006911494769155979f, 0.007616790011525154f, 0.004859809298068285f, 0.005955734755843878f, 0.005949190352112055f, 0.005777623504400253f, 0.008363639935851097f, 0.005089336074888706f, 0.00516399135813117f, 0.006990933325141668f, 0.0064481934532523155f, 0.008504391647875309f, 0.012369291856884956f, 0.005322083830833435f, 0.007581897079944611f, 0.005660536233335733f, 0.0022219172678887844f, 0.008193212561309338f, 0.0034722124692052603f, 0.009595116600394249f, 0.01094419602304697f, 0.005046423524618149f, 0.007127897348254919f, 0.005475018639117479f, 0.009152323007583618f, 0.003932022023946047f, 0.006563262548297644f, 0.006340222433209419f, 0.00442940229550004f, 0.00916721299290657f, 0.005067278631031513f, 0.0072202677838504314f, 0.006650058552622795f, 0.0059412335976958275f, 0.006732899695634842f, 0.004985489882528782f, 0.007419622037559748f, 0.004753335379064083f, 0.003517064033076167f, 0.005889774765819311f, 0.006501435302197933f, 0.004635431803762913f, 0.008456651121377945f, 0.008384072221815586f, 0.0019304624292999506f, 0.006452642846852541f, 0.008211572654545307f, 0.0045913890935480595f, 0.023038560524582863f, 0.0063102953135967255f, 0.007080105133354664f, 0.01113780215382576f, 0.004196192137897015f, 0.004120286088436842f, 0.00535776698961854f, 0.005691792815923691f, 0.006040152162313461f, 0.007661390118300915f, 0.006392537150532007f, 0.007101962808519602f, 0.005553496070206165f, 0.0053956471383571625f, 0.004765867721289396f, 0.006452622357755899f, 0.005706477910280228f, 0.006541060283780098f, 0.005101633723825216f, 0.007754730060696602f, 0.006155673414468765f, 0.008704837411642075f, 0.0036735860630869865f, 0.005191904492676258f, 0.004593475256115198f, 0.006625899579375982f, 0.004935092758387327f, 0.006620244588702917f, 0.00605896906927228f, 0.00637221522629261f, 0.007132681552320719f, 0.005309810861945152f, 0.003633790882304311f, 0.008461267687380314f, 0.006525138393044472f, 0.005189026705920696f, 0.0028359494172036648f, 0.004183065611869097f, 0.009877346456050873f, 0.008641072548925877f, 0.003115355037152767f, 0.00366793991997838f, 0.003533654846251011f, 0.006593209225684404f, 0.009711830876767635f, 0.005887059029191732f, 0.008243799209594727f, 0.007658831775188446f, 0.009955610148608685f, 0.006535771768540144f, 0.006626129616051912f, 0.006471092812716961f, 0.013265404850244522f, 0.007795072626322508f, 0.0062370384112000465f, 0.006160184275358915f, 0.007064392324537039f, 0.004932939074933529f, 0.00457008508965373f, 0.003507680492475629f, 0.0071328068152070045f, 0.006795918568968773f, 0.004045133478939533f, 0.005832895170897245f, 0.009320056065917015f, 0.005391601473093033f, 0.008979765698313713f, 0.004577528219670057f, 0.007011057808995247f, 0.002898646052926779f, 0.007126403506845236f, 0.009853492490947247f, 0.00880301184952259f, 0.0069879041984677315f, 0.006687771063297987f, 0.011080704629421234f, 0.011714626103639603f, 0.008099171333014965f, 0.006009918637573719f, 0.004770887549966574f, 0.006131940986961126f, 0.005195070058107376f, 0.008588690310716629f, 0.011712275445461273f, 0.008873657323420048f, 0.006855016108602285f, 0.006521407514810562f, 0.008919311687350273f, 0.006656517740339041f, 0.02325522154569626f, 0.0037733865901827812f, 0.004439365118741989f, 0.005657938774675131f, 0.009834831580519676f, 0.005853766109794378f, 0.00776425376534462f, 0.0066371806897223f, 0.006754755508154631f, 0.006847481243312359f, 0.006203650031238794f, 0.005175842437893152f, 0.005263158585876226f, 0.00619171280413866f, 0.004941083490848541f, 0.00717635452747345f, 0.00875874888151884f, 0.004584056790918112f, 0.009352740831673145f, 0.005475850310176611f, 0.0066904909908771515f, 0.005648770835250616f, 0.005740335676819086f, 0.010140150785446167f, 0.004740921314805746f, 0.005622772499918938f, 0.008149025961756706f, 0.008709380403161049f, 0.005072008352726698f, 0.006365902256220579f, 0.010281802155077457f, 0.006259833462536335f, 0.016741327941417694f, 0.007031697314232588f, 0.006503470707684755f, 0.005602724850177765f, 0.005918945651501417f, 0.003644790267571807f, 0.004958926234394312f, 0.0069010816514492035f, 0.008420431055128574f, 0.007696382701396942f, 0.006981249433010817f, 0.008141669444739819f, 0.0064857881516218185f, 0.007038247771561146f, 0.0041397251188755035f, 0.008095434866845608f, 0.010486333630979061f, 0.007883668877184391f, 0.00787153746932745f, 0.008266506716609001f, 0.00763404555618763f, 0.00431403610855341f, 0.007841352373361588f, 0.007877654395997524f, 0.007862119004130363f, 0.010792415589094162f, 0.0040801106952130795f, 0.005456435494124889f, 0.0077138603664934635f, 0.004612900782376528f, 0.005882133264094591f, 0.005546326283365488f, 0.009776835329830647f, 0.008807742968201637f, 0.006641380023211241f, 0.006509414408355951f, 0.005475251004099846f, 0.007846294902265072f, 0.008109573274850845f, 0.007419679779559374f, 0.007465242873877287f, 0.003132257144898176f);
static const ai_u16 conv2d_61_t_out_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_61_t_out_0_shape_h_const_u16 = 3;

static const ai_u16 conv2d_62_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_62_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_62_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_62_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_62_t_in_0_shape_ch_const_u16 = 336;
static const ai_u16 conv2d_62_t_out_0_shape_ch_const_u16 = 112;
static const ai_i8 conv2d_62_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_62_t_out_0_fmt_zero_const_s8 = -11;
static const ai_float conv2d_62_t_in_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_62_t_out_0_fmt_scale_const_f32 = 0.13179849088191986f;
static const ai_float conv2d_62_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.006302709691226482f, 0.003739624982699752f, 0.0037579291965812445f, 0.0041806697845458984f, 0.004479706287384033f, 0.003891721135005355f, 0.007189713418483734f, 0.006474340800195932f, 0.005508754402399063f, 0.0051512569189071655f, 0.006902884226292372f, 0.005389745347201824f, 0.0042940774001181126f, 0.005659086629748344f, 0.0039005510043352842f, 0.005391819402575493f, 0.005746249575167894f, 0.005394239444285631f, 0.005081838928163052f, 0.006084902212023735f, 0.005037802271544933f, 0.005415130872279406f, 0.004960509017109871f, 0.004196283407509327f, 0.0066919163800776005f, 0.006577889434993267f, 0.005070100072771311f, 0.0060948641039431095f, 0.006733268033713102f, 0.005512469448149204f, 0.004900158382952213f, 0.0046027647331357f, 0.004339647013694048f, 0.0055458820424973965f, 0.004958458244800568f, 0.004655618220567703f, 0.005098328460007906f, 0.004460612777620554f, 0.00399435730651021f, 0.004787837155163288f, 0.0065194000490009785f, 0.004524060059338808f, 0.0046358173713088036f, 0.0049376594834029675f, 0.006272908765822649f, 0.004363621585071087f, 0.004723287653177977f, 0.004798347596079111f, 0.00551981758326292f, 0.006313016638159752f, 0.00363895739428699f, 0.0071533601731061935f, 0.005080714821815491f, 0.006173469126224518f, 0.0051705013029277325f, 0.006089138798415661f, 0.005272566806524992f, 0.005423584952950478f, 0.0035488854628056288f, 0.003981204703450203f, 0.004943509586155415f, 0.00619521364569664f, 0.006291722413152456f, 0.004749050363898277f, 0.0063252756372094154f, 0.005018716678023338f, 0.004183166194707155f, 0.0042759994976222515f, 0.005009335000067949f, 0.005190109834074974f, 0.0035181816201657057f, 0.00490373931825161f, 0.005833393428474665f, 0.004514983855187893f, 0.005799193400889635f, 0.005981360096484423f, 0.006296859122812748f, 0.004694344010204077f, 0.0038778448943048716f, 0.004694897215813398f, 0.004663908388465643f, 0.004766179248690605f, 0.004447671584784985f, 0.005480878986418247f, 0.005718514323234558f, 0.005110702943056822f, 0.0036481821443885565f, 0.004832955077290535f, 0.004005573224276304f, 0.004664720501750708f, 0.0069128102622926235f, 0.005939153488725424f, 0.003441575914621353f, 0.007382594980299473f, 0.0050767106004059315f, 0.0056338245049119f, 0.005706384312361479f, 0.0053904103115201f, 0.005504757631570101f, 0.005487189162522554f, 0.004528185818344355f, 0.004533593077212572f, 0.003901856718584895f, 0.0034035774879157543f, 0.003529009874910116f, 0.0061775390058755875f, 0.004869963973760605f, 0.004274119157344103f, 0.004757137503474951f, 0.005514363292604685f, 0.003979536704719067f, 0.0056682126596570015f);
static const ai_layer_format_type conv2d_62_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_63_t_in_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_63_t_in_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_63_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_63_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_63_t_in_0_shape_ch_const_u16 = 112;
static const ai_u16 conv2d_63_t_out_0_shape_ch_const_u16 = 1280;
static const ai_i8 conv2d_63_t_in_0_fmt_zero_const_s8 = -11;
static const ai_i8 conv2d_63_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_63_t_in_0_fmt_scale_const_f32 = 0.13179849088191986f;
static const ai_float conv2d_63_t_out_0_fmt_scale_const_f32 = 0.0235294122248888f;
static const ai_float conv2d_63_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0026665213517844677f, 0.00305090076290071f, 0.0023590687196701765f, 0.003144465386867523f, 0.003022683784365654f, 0.0029213493689894676f, 0.003353528678417206f, 0.002734253415837884f, 0.0025639180094003677f, 0.003245360916480422f, 0.0027661356143653393f, 0.003665729658678174f, 0.0027724134270101786f, 0.002345385728403926f, 0.004304681904613972f, 0.0030309867579489946f, 0.0031750802882015705f, 0.002609930234029889f, 0.0027005968149751425f, 0.002926636952906847f, 0.002744168508797884f, 0.003463509725406766f, 0.002856291364878416f, 0.002806623000651598f, 0.003179546445608139f, 0.0039015861693769693f, 0.0033581757452338934f, 0.0027180358301848173f, 0.0028919056057929993f, 0.002523923059925437f, 0.003159873653203249f, 0.0027461263816803694f, 0.0030742529779672623f, 0.0031675552017986774f, 0.0034635860938578844f, 0.0031552661675959826f, 0.0028394709806889296f, 0.002534274710342288f, 0.003032262437045574f, 0.0035673759412020445f, 0.0037365364842116833f, 0.003545174840837717f, 0.003559670876711607f, 0.002825140953063965f, 0.0029514816123992205f, 0.0028164784889668226f, 0.002935498720034957f, 0.003926242236047983f, 0.002794028026983142f, 0.0025844883639365435f, 0.004085778724402189f, 0.0033942749723792076f, 0.0028717759996652603f, 0.002589730778709054f, 0.0027547269128262997f, 0.003822357626631856f, 0.003705001436173916f, 0.0036432540509849787f, 0.0027344089467078447f, 0.0031047523953020573f, 0.0034627029672265053f, 0.0029546916484832764f, 0.002703247359022498f, 0.0034455854911357164f, 0.0028689259197562933f, 0.00282311555929482f, 0.0036483427975326777f, 0.002842770656570792f, 0.0033518520649522543f, 0.0034643595572561026f, 0.0030523748137056828f, 0.0029837621841579676f, 0.0026557818055152893f, 0.0026634205132722855f, 0.0043090758845210075f, 0.0032615887466818094f, 0.0026357905007898808f, 0.002670182613655925f, 0.004076195880770683f, 0.0032489581499248743f, 0.0036190911196172237f, 0.0030417453963309526f, 0.002881967695429921f, 0.0032063410617411137f, 0.002359507605433464f, 0.002959720091894269f, 0.002750537358224392f, 0.0035118921659886837f, 0.00288175861351192f, 0.0026733889244496822f, 0.0031564778182655573f, 0.003243133658543229f, 0.0029903568793088198f, 0.002795256907120347f, 0.004537253640592098f, 0.0034483582712709904f, 0.0031086932867765427f, 0.0024154200218617916f, 0.002675793832167983f, 0.004082057159394026f, 0.0034409640356898308f, 0.0023108425084501505f, 0.002712574787437916f, 0.002545740222558379f, 0.002502491930499673f, 0.0026507435832172632f, 0.0030969863291829824f, 0.002839617198333144f, 0.0035509876906871796f, 0.002924959408119321f, 0.0027809878811240196f, 0.002735159592702985f, 0.0027883455622941256f, 0.003090290119871497f, 0.002592220436781645f, 0.0033738899510353804f, 0.0027288312558084726f, 0.002767314435914159f, 0.002851140918210149f, 0.003283038269728422f, 0.0038224898744374514f, 0.002623481908813119f, 0.0027961742598563433f, 0.0028246839065104723f, 0.002910543465986848f, 0.0023419801145792007f, 0.002904493361711502f, 0.0026700887829065323f, 0.0037842250894755125f, 0.00307966279797256f, 0.003160745371133089f, 0.003677274566143751f, 0.002895843703299761f, 0.0033581824973225594f, 0.00298312702216208f, 0.005233325529843569f, 0.0028449688106775284f, 0.0037302840501070023f, 0.0028384265024214983f, 0.0029891873709857464f, 0.002991474699229002f, 0.003254922106862068f, 0.0026343686040490866f, 0.002888738177716732f, 0.003392537822946906f, 0.0029461511876434088f, 0.0028846696950495243f, 0.0030523871537297964f, 0.002702108584344387f, 0.003605837235227227f, 0.0035257055424153805f, 0.0032157166860997677f, 0.0032549812458455563f, 0.0031374788377434015f, 0.004103775601834059f, 0.0025701364502310753f, 0.002983544720336795f, 0.0028035598807036877f, 0.0031699209939688444f, 0.002985440893098712f, 0.002428084146231413f, 0.0028879137244075537f, 0.0034422387834638357f, 0.002653968520462513f, 0.003195663681253791f, 0.0025985618121922016f, 0.0029504606500267982f, 0.0035989275202155113f, 0.002902248641476035f, 0.002739204792305827f, 0.0032901859376579523f, 0.0027083351742476225f, 0.0025551882572472095f, 0.00382122746668756f, 0.0033949518110603094f, 0.0024951614905148745f, 0.002843286842107773f, 0.0031357284169644117f, 0.0036351343151181936f, 0.0030086878687143326f, 0.0026243804022669792f, 0.003338526003062725f, 0.003835359588265419f, 0.003943957854062319f, 0.003932939376682043f, 0.003577287308871746f, 0.0035940867383033037f, 0.003106112824752927f, 0.0026405733078718185f, 0.0026662356685847044f, 0.0036114670801907778f, 0.0030677879694849253f, 0.002782612107694149f, 0.002825914416462183f, 0.003077044617384672f, 0.0032220534048974514f, 0.002867224859073758f, 0.00305352290160954f, 0.004076297860592604f, 0.0026302868500351906f, 0.0027199997566640377f, 0.002707416657358408f, 0.0026687481440603733f, 0.003571398090571165f, 0.002875294303521514f, 0.0029694316908717155f, 0.002974678063765168f, 0.0031763885635882616f, 0.0031776968389749527f, 0.0028883328195661306f, 0.003737828927114606f, 0.003060785122215748f, 0.0025201262906193733f, 0.003152403049170971f, 0.004276264924556017f, 0.0028673051856458187f, 0.0034343062434345484f, 0.004321605432778597f, 0.0035151795018464327f, 0.0028840936720371246f, 0.003278639866039157f, 0.0029106552246958017f, 0.0032452379819005728f, 0.0030795265920460224f, 0.0027584070339798927f, 0.002825662959367037f, 0.003714099060744047f, 0.0029310162644833326f, 0.002783837728202343f, 0.003054905217140913f, 0.0029909557197242975f, 0.0032158216927200556f, 0.002916244324296713f, 0.0035191369242966175f, 0.002623495180159807f, 0.0029353699646890163f, 0.002642960986122489f, 0.002342491876333952f, 0.0026086284779012203f, 0.002961874706670642f, 0.002912841271609068f, 0.004095111973583698f, 0.0024527839850634336f, 0.002508469158783555f, 0.0022836001589894295f, 0.003133988007903099f, 0.003490394214168191f, 0.0044083017855882645f, 0.002869528019800782f, 0.0029278506990522146f, 0.002765787299722433f, 0.003661414375528693f, 0.002654646523296833f, 0.00343636074103415f, 0.0030334636103361845f, 0.0026188238989561796f, 0.00330697582103312f, 0.0026889981236308813f, 0.002858604770153761f, 0.00256918603554368f, 0.003126139286905527f, 0.002445724792778492f, 0.0027457261458039284f, 0.0026888169813901186f, 0.003266950836405158f, 0.002759878057986498f, 0.0027266410179436207f, 0.0030851413030177355f, 0.003204485634341836f, 0.0030074259266257286f, 0.002738870680332184f, 0.0033550416119396687f, 0.0028187655843794346f, 0.0027478341944515705f, 0.0032200319692492485f, 0.00373449781909585f, 0.003489297814667225f, 0.0035596308298408985f, 0.0030308000277727842f, 0.0038258135318756104f, 0.0028020283207297325f, 0.003674349281936884f, 0.002782796509563923f, 0.0036498832050710917f, 0.0036097185220569372f, 0.003220875281840563f, 0.0033772995229810476f, 0.0032733939588069916f, 0.003603307530283928f, 0.0028840783052146435f, 0.002721792319789529f, 0.0026417425833642483f, 0.002866338239982724f, 0.0029237675480544567f, 0.0028316068928688765f, 0.003105208743363619f, 0.0036268127150833607f, 0.003110164077952504f, 0.0027411505579948425f, 0.003016375470906496f, 0.00285321194678545f, 0.0025551312137395144f, 0.0030960259027779102f, 0.0033699769992381334f, 0.0033025697339326143f, 0.0030107395723462105f, 0.003194954711943865f, 0.0028494244907051325f, 0.0026203852612525225f, 0.002416004426777363f, 0.002569339005276561f, 0.0025957191828638315f, 0.0022665138822048903f, 0.0038319258019328117f, 0.0033963043242692947f, 0.002992157358676195f, 0.00284568895585835f, 0.0033785831183195114f, 0.0028565083630383015f, 0.002402246231213212f, 0.002366249682381749f, 0.003644520416855812f, 0.0033968256320804358f, 0.002314204117283225f, 0.002951642731204629f, 0.0025678714737296104f, 0.004053935874253511f, 0.0036265419330447912f, 0.0029187200125306845f, 0.0031008650548756123f, 0.00442698085680604f, 0.0033490287605673075f, 0.0024056476540863514f, 0.003215838922187686f, 0.00303596630692482f, 0.0037559508346021175f, 0.0029925790149718523f, 0.002786241238936782f, 0.003523408668115735f, 0.003560261335223913f, 0.003165776375681162f, 0.0033140257000923157f, 0.002668768400326371f, 0.0032804044894874096f, 0.0032225348986685276f, 0.002623578766360879f, 0.0023560686968266964f, 0.002874491736292839f, 0.004515024833381176f, 0.004033924080431461f, 0.002355060074478388f, 0.0029231118969619274f, 0.002940337872132659f, 0.0027142686303704977f, 0.0028746987227350473f, 0.00365249696187675f, 0.0034851899836212397f, 0.0036245633382350206f, 0.0028880846220999956f, 0.004429257940500975f, 0.0033445090521126986f, 0.0033219007309526205f, 0.0030215089209377766f, 0.0030399507377296686f, 0.004049642942845821f, 0.002425203565508127f, 0.0036732810549438f, 0.003397304331883788f, 0.0030972780659794807f, 0.002887795213609934f, 0.004132094793021679f, 0.0034753240179270506f, 0.003324266290292144f, 0.0030286076944321394f, 0.0037432298995554447f, 0.00302953296341002f, 0.0030572328250855207f, 0.003256649011746049f, 0.0029951606411486864f, 0.0032896967604756355f, 0.002953083487227559f, 0.003524694824591279f, 0.0030138357542455196f, 0.002972951391711831f, 0.0030188129749149084f, 0.0037484164349734783f, 0.0031095948070287704f, 0.0029185705352574587f, 0.0032595142256468534f, 0.0034920317120850086f, 0.0036928129848092794f, 0.0033952579833567142f, 0.002869518008083105f, 0.0036866802256554365f, 0.0025859440211206675f, 0.002716194372624159f, 0.002715782029554248f, 0.003015749854966998f, 0.003597546834498644f, 0.003264885162934661f, 0.003078038804233074f, 0.0031570149585604668f, 0.004495554603636265f, 0.0026163035072386265f, 0.002655328018590808f, 0.0024604122154414654f, 0.0026199975982308388f, 0.0027849688194692135f, 0.0027906796894967556f, 0.00253686192445457f, 0.0025244641583412886f, 0.004044877365231514f, 0.0028017412405461073f, 0.0023329865653067827f, 0.003948959056288004f, 0.003745228750631213f, 0.0030736010521650314f, 0.003069814993068576f, 0.0038855625316500664f, 0.0034097766038030386f, 0.002375255338847637f, 0.0031189993023872375f, 0.0037007490172982216f, 0.002589999232441187f, 0.002669022884219885f, 0.002769388724118471f, 0.0034832144156098366f, 0.0036342490930110216f, 0.0029572276398539543f, 0.002945389598608017f, 0.0026382338255643845f, 0.0030600661411881447f, 0.0028031030669808388f, 0.0031923141796141863f, 0.002591110300272703f, 0.0033440180122852325f, 0.0030168292578309774f, 0.0035477220080792904f, 0.0034983723890036345f, 0.002921503270044923f, 0.0035865786485373974f, 0.0033044067677110434f, 0.0026777193415910006f, 0.002628560643643141f, 0.003935500513762236f, 0.002904308494180441f, 0.003836945164948702f, 0.003283015452325344f, 0.0033544544130563736f, 0.002601105021312833f, 0.003277631476521492f, 0.0028034881688654423f, 0.002517872489988804f, 0.002535872859880328f, 0.003339151619002223f, 0.0027900501154363155f, 0.0036978076677769423f, 0.0032609603367745876f, 0.0035415329039096832f, 0.0032151355408132076f, 0.0029885934200137854f, 0.0034605960827320814f, 0.002364245941862464f, 0.0026004151441156864f, 0.002465457422658801f, 0.0027243872173130512f, 0.003923460375517607f, 0.0031318236142396927f, 0.0028741362038999796f, 0.0033386077266186476f, 0.003238877048715949f, 0.002957777353003621f, 0.0030021690763533115f, 0.00350857712328434f, 0.002869821386411786f, 0.003489190014079213f, 0.0035565150901675224f, 0.0038534917403012514f, 0.0033309385180473328f, 0.0030928943306207657f, 0.0034835170954465866f, 0.003168773604556918f, 0.0028563733212649822f, 0.0031086013186722994f, 0.004168507643043995f, 0.0024983971379697323f, 0.0033650235272943974f, 0.0033493090886622667f, 0.0031315560918301344f, 0.002606046386063099f, 0.0040871682576835155f, 0.0036346898414194584f, 0.00282872817479074f, 0.0024634648580104113f, 0.002729980042204261f, 0.003125603776425123f, 0.0029972768388688564f, 0.002656854223459959f, 0.003664969466626644f, 0.00327184796333313f, 0.002587463241070509f, 0.003502710023894906f, 0.003160790540277958f, 0.002986629493534565f, 0.003004740457981825f, 0.0030907299369573593f, 0.002770527731627226f, 0.0027406467124819756f, 0.002675107214599848f, 0.002793095773085952f, 0.0032880613580346107f, 0.002721811644732952f, 0.0023515259381383657f, 0.0035036669578403234f, 0.002788231475278735f, 0.002706131199374795f, 0.002757203532382846f, 0.002913643606007099f, 0.0032019757200032473f, 0.0037262451369315386f, 0.0030386624857783318f, 0.0032601607963442802f, 0.003172683296725154f, 0.003340773284435272f, 0.0026396685279905796f, 0.0026659821160137653f, 0.0026060808449983597f, 0.003321737051010132f, 0.004159899428486824f, 0.002946132095530629f, 0.003306998871266842f, 0.0032915116753429174f, 0.0033097185660153627f, 0.0032091846223920584f, 0.00303570250980556f, 0.0026115914806723595f, 0.0035433301236480474f, 0.0026623117737472057f, 0.002766846911981702f, 0.0033157786820083857f, 0.0030244833324104548f, 0.0032217269763350487f, 0.0026282977778464556f, 0.0038932773750275373f, 0.002766329562291503f, 0.0023814979940652847f, 0.003819848643615842f, 0.0027482486329972744f, 0.003151213750243187f, 0.0029720866587013006f, 0.0030656238086521626f, 0.002696750685572624f, 0.0026640959549695253f, 0.0027825033757835627f, 0.002905029570683837f, 0.0032890753354877234f, 0.002910959767177701f, 0.00383456121198833f, 0.0031705715227872133f, 0.002756343223154545f, 0.003254898125305772f, 0.002616917947307229f, 0.002547389594838023f, 0.0028544291853904724f, 0.0032672355882823467f, 0.002852862933650613f, 0.004038043785840273f, 0.0026696862187236547f, 0.0027026983443647623f, 0.0035421496722847223f, 0.003735712030902505f, 0.0026060568634420633f, 0.002789765130728483f, 0.0025855302810668945f, 0.0024024625308811665f, 0.0038780653849244118f, 0.0028718546964228153f, 0.0024691380094736814f, 0.0026575555093586445f, 0.0031230850145220757f, 0.003333969274535775f, 0.004644400905817747f, 0.0027966659981757402f, 0.0025899061001837254f, 0.003088494297116995f, 0.003380980808287859f, 0.003214012598618865f, 0.0031558044720441103f, 0.0029439448844641447f, 0.002699194010347128f, 0.0036659887991845608f, 0.0039792489260435104f, 0.004056091420352459f, 0.0030434865038841963f, 0.0034253206104040146f, 0.004195705987513065f, 0.002680299337953329f, 0.0032253682147711515f, 0.0035417070612311363f, 0.003810040419921279f, 0.0035175438970327377f, 0.0025112521834671497f, 0.002927844412624836f, 0.0028313444927334785f, 0.003704293631017208f, 0.0030755142215639353f, 0.0024237153120338917f, 0.0031651929020881653f, 0.0031206493731588125f, 0.002673197304829955f, 0.002477492904290557f, 0.003570632543414831f, 0.00265321577899158f, 0.0036729814019054174f, 0.003112141275778413f, 0.002924845088273287f, 0.003017846727743745f, 0.003475348697975278f, 0.0028690400067716837f, 0.0031608142890036106f, 0.0030200828332453966f, 0.0027586573269218206f, 0.0030374934431165457f, 0.0032643487211316824f, 0.0028064115904271603f, 0.0032139343675225973f, 0.0038054301403462887f, 0.003488983726128936f, 0.0032393895089626312f, 0.0027440874837338924f, 0.0033823030535131693f, 0.0027998974546790123f, 0.003103294176980853f, 0.0026672494132071733f, 0.002977294148877263f, 0.0036150773521512747f, 0.0029789735563099384f, 0.0032919151708483696f, 0.0032401166390627623f, 0.002743879798799753f, 0.0036153756082057953f, 0.002620347775518894f, 0.0035864037927240133f, 0.0029884842224419117f, 0.003313913242891431f, 0.0026011301670223475f, 0.00284733809530735f, 0.0030740988440811634f, 0.003088094061240554f, 0.0035281821619719267f, 0.003474404336884618f, 0.003449833020567894f, 0.003315818263217807f, 0.0032185756135731936f, 0.003166090464219451f, 0.0028063186910003424f, 0.003008155385032296f, 0.0027803026605397463f, 0.0033186848741024733f, 0.003027802100405097f, 0.0029267522040754557f, 0.0031497615855187178f, 0.0035055852495133877f, 0.003700608154758811f, 0.003298824653029442f, 0.003070054342970252f, 0.00257701869122684f, 0.0036106484476476908f, 0.003181695006787777f, 0.0034896498546004295f, 0.0033104594331234694f, 0.0029417986515909433f, 0.002864533569663763f, 0.003787602763622999f, 0.0031862142495810986f, 0.003231900976970792f, 0.0028061189223080873f, 0.0032721597235649824f, 0.004091167356818914f, 0.0033182636834681034f, 0.003093416104093194f, 0.0038785217329859734f, 0.0039038360118865967f, 0.0036706828977912664f, 0.003650427795946598f, 0.0026936957146972418f, 0.0033702810760587454f, 0.003013218054547906f, 0.0031674131751060486f, 0.0030513049568980932f, 0.0029261051677167416f, 0.002735626185312867f, 0.0030635485891252756f, 0.0026235245168209076f, 0.00393170490860939f, 0.0032930197194218636f, 0.003409422468394041f, 0.002788454294204712f, 0.00277024507522583f, 0.003937656059861183f, 0.0028478580061346292f, 0.002646378707140684f, 0.003548374632373452f, 0.002342197112739086f, 0.002848218195140362f, 0.0033348803408443928f, 0.002552884863689542f, 0.002450388390570879f, 0.002617999678477645f, 0.0030133863911032677f, 0.00369455898180604f, 0.002581564709544182f, 0.002806631848216057f, 0.0029936814680695534f, 0.0029904155526310205f, 0.0028291817288845778f, 0.0031293281354010105f, 0.002863864414393902f, 0.0027503138408064842f, 0.0028474207501858473f, 0.003303836565464735f, 0.0027555259875953197f, 0.002923661842942238f, 0.0026926689315587282f, 0.003368510166183114f, 0.0030176457948982716f, 0.0029982959385961294f, 0.0028231018222868443f, 0.0038284296169877052f, 0.0027361467946320772f, 0.0035518500953912735f, 0.0028495686128735542f, 0.0029084612615406513f, 0.003095009131357074f, 0.002780007664114237f, 0.0029778559692204f, 0.002877594204619527f, 0.002431420609354973f, 0.0037938642781227827f, 0.0025571011938154697f, 0.0029731602407991886f, 0.0035751154646277428f, 0.003086285199970007f, 0.0033150885719805956f, 0.003964263945817947f, 0.003594087902456522f, 0.002651962684467435f, 0.002470880514010787f, 0.003072186606004834f, 0.003001363482326269f, 0.0027034892700612545f, 0.002904667519032955f, 0.0032979659736156464f, 0.0036286127287894487f, 0.0035777899902313948f, 0.0032071038149297237f, 0.0028420602902770042f, 0.002949421526864171f, 0.0034650855232030153f, 0.0032579186372458935f, 0.0031036881264299154f, 0.0025649084709584713f, 0.0030190825928002596f, 0.0026180522982031107f, 0.0025325044989585876f, 0.0026865105610340834f, 0.0030276367906481028f, 0.003943994641304016f, 0.0027063062880188227f, 0.003515659598633647f, 0.0029147055465728045f, 0.0033159286249428988f, 0.0028757050167769194f, 0.0045633092522621155f, 0.003008487168699503f, 0.0030208479147404432f, 0.0027736376505345106f, 0.002746576676145196f, 0.0028518536128103733f, 0.003487683366984129f, 0.0029655946418642998f, 0.003898868802934885f, 0.003721769666299224f, 0.003120468696579337f, 0.004289690405130386f, 0.0024936741683632135f, 0.002673966810107231f, 0.002750837942585349f, 0.003704194910824299f, 0.0023974233772605658f, 0.003604827681556344f, 0.0034526193048805f, 0.0030559408478438854f, 0.003182404674589634f, 0.0032513560727238655f, 0.002981789642944932f, 0.002914966084063053f, 0.003281892044469714f, 0.002796189161017537f, 0.002524853218346834f, 0.0029994817450642586f, 0.0033523961901664734f, 0.0028603961691260338f, 0.0026675327681005f, 0.0033022798597812653f, 0.0030756336636841297f, 0.003862706245854497f, 0.0026120964903384447f, 0.0037725467700511217f, 0.0025952383875846863f, 0.004328682087361813f, 0.0031758761033415794f, 0.0028222932014614344f, 0.0034966368693858385f, 0.0028575025498867035f, 0.0046045128256082535f, 0.0023581713903695345f, 0.0033599615562707186f, 0.0028671284671872854f, 0.003648661309853196f, 0.002579077845439315f, 0.004124016501009464f, 0.003072679741308093f, 0.0034861424937844276f, 0.00308597250841558f, 0.0027613446582108736f, 0.0033135979901999235f, 0.002963597420603037f, 0.003307398408651352f, 0.002954063005745411f, 0.0024739038199186325f, 0.0035548240412026644f, 0.003067275043576956f, 0.002180614974349737f, 0.0028522880747914314f, 0.004126076120883226f, 0.003685509320348501f, 0.0037877329159528017f, 0.0029667625203728676f, 0.0027375095523893833f, 0.002644686959683895f, 0.0028718234971165657f, 0.002760348841547966f, 0.003624062752351165f, 0.002766137011349201f, 0.0033270972780883312f, 0.004038685001432896f, 0.0037662647664546967f, 0.0028813183307647705f, 0.0033178820740431547f, 0.0029068710282444954f, 0.0029125597793608904f, 0.0028051265981048346f, 0.0024238054174929857f, 0.002405349863693118f, 0.0026963602285832167f, 0.0036370214074850082f, 0.004269592929631472f, 0.0031010068487375975f, 0.0027673630975186825f, 0.0031487734522670507f, 0.002365975407883525f, 0.002794167958199978f, 0.0028559789061546326f, 0.00277653057128191f, 0.002470519859343767f, 0.0031005805358290672f, 0.003571762703359127f, 0.0030236721504479647f, 0.0037723593413829803f, 0.0029829044360667467f, 0.0027423519641160965f, 0.002960808342322707f, 0.0029963040724396706f, 0.002789659658446908f, 0.0029859368223696947f, 0.0023981190752238035f, 0.002578841522336006f, 0.0036540201399475336f, 0.0031774055678397417f, 0.0029249093495309353f, 0.0037178071215748787f, 0.0026858013588935137f, 0.003138476749882102f, 0.0028893148992210627f, 0.0038458688650280237f, 0.002597732236608863f, 0.002354678697884083f, 0.0026904090773314238f, 0.0031034599523991346f, 0.0027337565552443266f, 0.0034270216710865498f, 0.0025929140392690897f, 0.003298981813713908f, 0.004252564627677202f, 0.0032003256492316723f, 0.003392762504518032f, 0.0027097768615931273f, 0.003565539140254259f, 0.003951516468077898f, 0.002706281142309308f, 0.0027326582930982113f, 0.0024638306349515915f, 0.0026076682843267918f, 0.0029340554028749466f, 0.0028486615046858788f, 0.0026407684199512005f, 0.0032607114408165216f, 0.0025039082393050194f, 0.0029214085079729557f, 0.0030301695223897696f, 0.003505315398797393f, 0.0027403449639678f, 0.0028176503255963326f, 0.0025964861270040274f, 0.002980667632073164f, 0.003029752057045698f, 0.002985777100548148f, 0.0028027312364429235f, 0.003187828930094838f, 0.004335734527558088f, 0.0028586634434759617f, 0.002998344600200653f, 0.003314907429739833f, 0.0027348706498742104f, 0.0029351364355534315f, 0.00305271428078413f, 0.0026925294660031796f, 0.003108253004029393f, 0.0029731704853475094f, 0.0026345355436205864f, 0.0028387149795889854f, 0.0028164812829345465f, 0.003707190277054906f, 0.0029031753074377775f, 0.0029419208876788616f, 0.0030945215839892626f, 0.0032656253315508366f, 0.0030369290616363287f, 0.003447262803092599f, 0.004109601024538279f, 0.002676695818081498f, 0.002598799066618085f, 0.002962230471894145f, 0.0033560749143362045f, 0.002799802226945758f, 0.0027511443477123976f, 0.0030591303948312998f, 0.0031169529538601637f, 0.0028342304285615683f, 0.0024675102904438972f, 0.002972011687234044f, 0.002985710045322776f, 0.003193268785253167f, 0.0030711644794791937f, 0.003630732884630561f, 0.0029076714999973774f, 0.0030927928164601326f, 0.003205380402505398f, 0.0035526130814105272f, 0.003278016112744808f, 0.003566358471289277f, 0.0029543491546064615f, 0.0029504941776394844f, 0.0028548745904117823f, 0.003815949195995927f, 0.0027362932451069355f, 0.002744300290942192f, 0.0027469308115541935f, 0.00343610648997128f, 0.002537462394684553f, 0.0032543824054300785f, 0.003290105378255248f, 0.003077472560107708f, 0.0028418705333024263f, 0.0027903346344828606f, 0.002898332430049777f, 0.002461089985445142f, 0.002559487707912922f, 0.0026442750822752714f, 0.0026012645103037357f, 0.0027231399435549974f, 0.0025428468361496925f, 0.0028695629443973303f, 0.0034116976894438267f, 0.003409458789974451f, 0.0026730478275567293f, 0.0030368755105882883f, 0.004069325048476458f, 0.0033065536990761757f, 0.0033318682108074427f, 0.002791464328765869f, 0.0027517620474100113f, 0.002765027107670903f, 0.0032077799551188946f, 0.0029254588298499584f, 0.003001418663188815f, 0.0027497666887938976f, 0.002718852600082755f, 0.002372915390878916f, 0.00263472693040967f, 0.003357475623488426f, 0.002672990784049034f, 0.004381713457405567f, 0.0029359289910644293f, 0.003342673648148775f, 0.0029948705341666937f, 0.00361692369915545f, 0.003373936051502824f, 0.0033004949800670147f, 0.002848203992471099f, 0.0030610307585448027f, 0.002759278519079089f, 0.00275087496265769f, 0.0036805071868002415f, 0.0030083812307566404f, 0.0031893716659396887f, 0.002928792731836438f, 0.0027928880881518126f, 0.0033154943957924843f, 0.002944879699498415f, 0.0032147567253559828f, 0.0030163920018821955f, 0.002901847707107663f, 0.003022419987246394f, 0.0030279671773314476f, 0.0029243486933410168f, 0.004211920779198408f, 0.0031921030022203922f, 0.0029245216865092516f, 0.002577423118054867f, 0.0033564972691237926f, 0.0042786444537341595f, 0.002549937926232815f, 0.004318918567150831f, 0.003315660171210766f, 0.00345644005574286f, 0.004474686458706856f, 0.0030701279174536467f, 0.0034223138354718685f, 0.0033432331401854753f, 0.003085568081587553f, 0.0025172284804284573f, 0.0034598750062286854f, 0.002937262412160635f, 0.0027156355790793896f, 0.002943780506029725f, 0.004489130340516567f, 0.003673285013064742f, 0.003970538266003132f, 0.002887727925553918f, 0.0024937523994594812f, 0.0029390340205281973f, 0.004009694792330265f, 0.003223386825993657f, 0.002927060704678297f, 0.003088826546445489f, 0.0034735542722046375f, 0.002655780641362071f, 0.002984052523970604f, 0.002570148091763258f, 0.0032128309831023216f, 0.0036583493929356337f, 0.0025138009805232286f, 0.0036391462199389935f, 0.0025343208108097315f, 0.002960737096145749f, 0.002716419054195285f, 0.004052596632391214f, 0.0026046321727335453f, 0.0026282286271452904f, 0.0037545368541032076f, 0.0027676853351294994f, 0.00288137118332088f, 0.002459370531141758f, 0.0028064819052815437f, 0.004661879502236843f, 0.002770327962934971f, 0.0029094198253005743f, 0.002979002892971039f, 0.0027190065011382103f, 0.002859627129510045f, 0.0032877791672945023f, 0.0032372567802667618f, 0.003399986308068037f, 0.0036808447912335396f, 0.0029034200124442577f, 0.0034110289998352528f, 0.0028400092851370573f, 0.0025848515797406435f, 0.002541372086852789f, 0.0028701615519821644f, 0.002983709564432502f, 0.0031723626889288425f, 0.0031203434336930513f, 0.002591145923361182f, 0.0040785339660942554f, 0.0028970027342438698f, 0.003246261505410075f, 0.0025051841512322426f, 0.00255112093873322f, 0.002952519804239273f, 0.003273668698966503f, 0.0034637432545423508f, 0.0032225060276687145f, 0.002678142860531807f, 0.0029914844781160355f, 0.0030043066944926977f, 0.003083131043240428f, 0.0032589687034487724f, 0.002587235765531659f, 0.0029388561379164457f, 0.0028913284186273813f, 0.002834528684616089f, 0.0031104194931685925f, 0.002880248473957181f, 0.003201357088983059f, 0.0037924896460026503f, 0.0022885682992637157f, 0.003009853884577751f, 0.0025916805025190115f, 0.0025833584368228912f, 0.002611172618344426f, 0.003194545628502965f, 0.0027955928817391396f, 0.003806025255471468f, 0.002724233316257596f, 0.003745627123862505f, 0.0028109648264944553f, 0.0025234438944607973f, 0.003209788352251053f, 0.003073434578254819f, 0.0029973071068525314f, 0.0029151199851185083f, 0.0038300827145576477f, 0.003105481853708625f, 0.003432521829381585f, 0.0023330552503466606f, 0.004067760892212391f, 0.004166914150118828f, 0.0029048577416688204f, 0.003091901307925582f, 0.002861334942281246f, 0.0035056278575211763f, 0.003766796085983515f, 0.0033282700460404158f, 0.0029244881588965654f, 0.00307256611995399f, 0.004512890242040157f, 0.002767179859802127f, 0.003123892704024911f, 0.0036857088562101126f, 0.0033726480323821306f, 0.003410115372389555f, 0.003390106139704585f, 0.002657427918165922f, 0.003629988059401512f, 0.0031252168118953705f, 0.0032529220916330814f, 0.003653043881058693f, 0.002977970754727721f, 0.0026511745527386665f, 0.0029963201377540827f, 0.00263024284504354f, 0.003341515315696597f, 0.003250748384743929f, 0.0031087538227438927f, 0.002868032781407237f, 0.004195076413452625f, 0.003578773932531476f, 0.0029912618920207024f, 0.0024615724105387926f, 0.002653139177709818f, 0.003294203197583556f, 0.0025434924755245447f, 0.0033516651019454002f, 0.002760449657216668f, 0.0033438242971897125f, 0.003436305560171604f, 0.002891985699534416f, 0.0027244577649980783f, 0.002885274589061737f, 0.00314242928288877f, 0.004031490068882704f, 0.0032957785297185183f, 0.0030766986310482025f, 0.003884899662807584f, 0.0038790085818618536f, 0.0033378470689058304f, 0.003105910262092948f, 0.0034156441688537598f, 0.0026929504238069057f, 0.0022305194288492203f, 0.002707878826186061f, 0.003112539416179061f, 0.0024945437908172607f, 0.0031765352468937635f, 0.0035155501682311296f, 0.003721788991242647f, 0.004227075260132551f, 0.0024126279167830944f, 0.002979802433401346f, 0.0031104525551199913f, 0.0034306624438613653f, 0.0031628233846277f, 0.0027167510706931353f, 0.0028809073846787214f, 0.002833549166098237f, 0.003413568949326873f, 0.0034910477697849274f, 0.003843865590170026f, 0.0033547403290867805f, 0.0028032183181494474f, 0.002649529604241252f, 0.003098161891102791f, 0.0027140311431139708f, 0.0037063059862703085f, 0.0024107799399644136f, 0.0026425833348184824f, 0.003520945319905877f, 0.0032468202989548445f, 0.003370865248143673f, 0.0028604494873434305f, 0.0024674427695572376f, 0.0026260358281433582f, 0.0037090075202286243f, 0.0024570913519710302f, 0.002826176816597581f, 0.003131377976387739f, 0.0027653726283460855f, 0.0028558119665831327f, 0.0026382969226688147f, 0.0027837869711220264f, 0.002537728985771537f, 0.0027780230157077312f, 0.0026736415456980467f, 0.004247669130563736f, 0.003076769644394517f, 0.003029370680451393f, 0.0034638436045497656f, 0.0033608132507652044f, 0.002525021554902196f, 0.0031612245365977287f, 0.004140874836593866f, 0.002971486421301961f, 0.0028990302234888077f, 0.0025959592312574387f, 0.0023247082717716694f, 0.0030284905806183815f, 0.0033997974824160337f, 0.0029043774120509624f, 0.003463468747213483f, 0.004386506509035826f, 0.0029879349749535322f, 0.0028664020355790854f, 0.003344304161146283f, 0.0030961991287767887f, 0.0030402550473809242f, 0.003926805220544338f, 0.004425558727234602f, 0.0037656098138540983f, 0.002320928033441305f, 0.0030117579735815525f, 0.0037990135606378317f, 0.003503488376736641f, 0.0031284294091165066f, 0.0029736082069575787f, 0.0028907142113894224f, 0.002924134023487568f, 0.003934371750801802f, 0.003536011790856719f, 0.0027672015130519867f, 0.0038156567607074976f, 0.0027296689804643393f, 0.003093583043664694f, 0.003100356552749872f, 0.0032478859648108482f, 0.0032498331274837255f, 0.002477009315043688f, 0.003228934248909354f);
static const ai_layer_format_type conv2d_63_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;



static const ai_u32 nl_66_t_in_0_shape_ch_prod_const_u32 = 3;
STAI_API_ENTRY
stai_return_code stai_network_run(
  stai_network* network,
  const stai_run_mode mode)
{
   STAI_UNUSED(mode)
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_ACTIVATIONS) != STAI_FLAG_ACTIVATIONS,
        STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_INPUTS) != STAI_FLAG_INPUTS,
                  STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_OUTPUTS) != STAI_FLAG_OUTPUTS,
                  STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_WEIGHTS) != STAI_FLAG_WEIGHTS,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)


  /* LITE_KERNEL_SECTION BEGIN eltwise_0 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_0(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_0 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_1 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_1(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_1 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_2 */
  {
      const ai_i8* conv2d_2_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 14288);
    const ai_i8* conv2d_2_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 8);
    const ai_i32* conv2d_2_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 440);
    ai_i8* conv2d_2_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4288);
    ai_i16* conv2d_2_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 41940);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(2, 1, {(stai_ptr) conv2d_2_t_in_0_ptr_const_s8});
    
  forward_lite_conv2d_rgb_sssa8_ch(conv2d_2_t_in_0_ptr_const_s8, conv2d_2_t_in_0_shape_w_const_u16, conv2d_2_t_weight_0_ptr_const_s8, conv2d_2_t_out_0_shape_ch_const_u16, conv2d_2_t_weight_0_shape_w_const_u16, conv2d_2_l_pad_W_0_const_s32, conv2d_2_l_stride_0_const_u16, conv2d_2_t_weight_1_ptr_const_s32, conv2d_2_t_in_0_fmt_zero_const_s8, conv2d_2_t_out_0_fmt_zero_const_s8, conv2d_2_t_in_0_fmt_scale_const_f32, conv2d_2_t_out_0_fmt_scale_const_f32, conv2d_2_t_weight_0_fmt_scale_const_f32, conv2d_2_l_out_ch_format_const_layer_format_type, conv2d_2_t_out_0_ptr_s8, conv2d_2_t_out_0_shape_w_const_u16, 1196, conv2d_2_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(2, 1, {(stai_ptr) conv2d_2_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_2 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_3_pad_before */
  {
      const ai_ptr conv2d_3_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 4288);
    ai_ptr conv2d_3_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 1152);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(3, 1, {(stai_ptr) conv2d_3_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_3_pad_before_t_in_0_ptr_const_ptr, conv2d_3_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_3_pad_before_v_pad_constant_value_const_s8), conv2d_3_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_3_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(768), (ai_i32)(800), (ai_i32)(800), (ai_i32)(16), (ai_i32)(16));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(3, 1, {(stai_ptr) conv2d_3_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_3_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_3 */
  {
      const ai_i8* conv2d_3_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1152);
    const ai_i8* conv2d_3_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 504);
    const ai_i32* conv2d_3_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 648);
    ai_i8* conv2d_3_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 384);
    ai_i16* conv2d_3_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 41152);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(3, 1, {(stai_ptr) conv2d_3_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_3_t_in_0_ptr_const_s8, conv2d_3_t_in_0_shape_w_const_u16, conv2d_3_t_in_0_shape_h_const_u16, conv2d_3_t_in_0_shape_ch_const_u16, conv2d_3_t_weight_0_ptr_const_s8, conv2d_3_l_stride_1_const_u16, conv2d_3_l_stride_0_const_u16, conv2d_3_t_weight_1_ptr_const_s32, conv2d_3_t_in_0_fmt_zero_const_s8, conv2d_3_t_out_0_fmt_zero_const_s8, conv2d_3_t_in_0_fmt_scale_const_f32, conv2d_3_t_out_0_fmt_scale_const_f32, conv2d_3_t_weight_0_fmt_scale_const_f32, conv2d_3_t_out_0_ptr_s8, conv2d_3_t_out_0_shape_w_const_u16, conv2d_3_t_out_0_shape_h_const_u16, 0, 593, conv2d_3_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(3, 1, {(stai_ptr) conv2d_3_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_3 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_4 */
  {
      const ai_i8* conv2d_4_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 384);
    const ai_i8* conv2d_4_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 712);
    const ai_i32* conv2d_4_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 840);
    ai_i8* conv2d_4_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    ai_i16* conv2d_4_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 37248);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(4, 1, {(stai_ptr) conv2d_4_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_4_t_in_0_ptr_const_s8, conv2d_4_t_in_0_shape_w_const_u16, conv2d_4_t_in_0_shape_h_const_u16, conv2d_4_l_stride_1_const_u16, conv2d_4_l_stride_0_const_u16, conv2d_4_t_in_0_shape_ch_const_u16, conv2d_4_t_weight_0_ptr_const_s8, conv2d_4_t_out_0_shape_ch_const_u16, conv2d_4_t_weight_1_ptr_const_s32, conv2d_4_t_in_0_fmt_zero_const_s8, conv2d_4_t_out_0_fmt_zero_const_s8, conv2d_4_t_in_0_fmt_scale_const_f32, conv2d_4_t_out_0_fmt_scale_const_f32, conv2d_4_t_weight_0_fmt_scale_const_f32, conv2d_4_l_out_ch_format_const_layer_format_type, conv2d_4_t_out_0_ptr_s8, 1, 144, conv2d_4_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(4, 1, {(stai_ptr) conv2d_4_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_4 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_5 */
  {
      const ai_i8* conv2d_5_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* conv2d_5_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 872);
    const ai_i32* conv2d_5_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 1256);
    ai_i8* conv2d_5_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 18944);
    ai_i16* conv2d_5_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 18432);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(5, 1, {(stai_ptr) conv2d_5_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_5_t_in_0_ptr_const_s8, conv2d_5_t_in_0_shape_w_const_u16, conv2d_5_t_in_0_shape_h_const_u16, conv2d_5_l_stride_1_const_u16, conv2d_5_l_stride_0_const_u16, conv2d_5_t_in_0_shape_ch_const_u16, conv2d_5_t_weight_0_ptr_const_s8, conv2d_5_t_out_0_shape_ch_const_u16, conv2d_5_t_weight_1_ptr_const_s32, conv2d_5_t_in_0_fmt_zero_const_s8, conv2d_5_t_out_0_fmt_zero_const_s8, conv2d_5_t_in_0_fmt_scale_const_f32, conv2d_5_t_out_0_fmt_scale_const_f32, conv2d_5_t_weight_0_fmt_scale_const_f32, conv2d_5_l_out_ch_format_const_layer_format_type, conv2d_5_t_out_0_ptr_s8, 1, 512, conv2d_5_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(5, 1, {(stai_ptr) conv2d_5_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_5 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_6_pad_before */
  {
      const ai_ptr conv2d_6_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 18944);
    ai_ptr conv2d_6_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 9536);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(6, 1, {(stai_ptr) conv2d_6_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_6_pad_before_t_in_0_ptr_const_ptr, conv2d_6_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_6_pad_before_v_pad_constant_value_const_s8), conv2d_6_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_6_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(2304), (ai_i32)(0), (ai_i32)(4800), (ai_i32)(0), (ai_i32)(96));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(6, 1, {(stai_ptr) conv2d_6_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_6_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_6 */
  {
      const ai_i8* conv2d_6_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 9536);
    const ai_i8* conv2d_6_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 1448);
    const ai_i32* conv2d_6_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 1880);
    ai_i8* conv2d_6_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 8384);
    ai_i16* conv2d_6_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(6, 1, {(stai_ptr) conv2d_6_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_6_t_in_0_ptr_const_s8, conv2d_6_t_in_0_shape_w_const_u16, conv2d_6_t_in_0_shape_h_const_u16, conv2d_6_t_in_0_shape_ch_const_u16, conv2d_6_t_weight_0_ptr_const_s8, conv2d_6_l_stride_1_const_u16, conv2d_6_l_stride_0_const_u16, conv2d_6_t_weight_1_ptr_const_s32, conv2d_6_t_in_0_fmt_zero_const_s8, conv2d_6_t_out_0_fmt_zero_const_s8, conv2d_6_t_in_0_fmt_scale_const_f32, conv2d_6_t_out_0_fmt_scale_const_f32, conv2d_6_t_weight_0_fmt_scale_const_f32, conv2d_6_t_out_0_ptr_s8, conv2d_6_t_out_0_shape_w_const_u16, conv2d_6_t_out_0_shape_h_const_u16, 0, 1777, conv2d_6_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(6, 1, {(stai_ptr) conv2d_6_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_6 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_7 */
  {
      const ai_i8* conv2d_7_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8384);
    const ai_i8* conv2d_7_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 2072);
    const ai_i32* conv2d_7_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 2456);
    ai_i8* conv2d_7_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 272);
    ai_i16* conv2d_7_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(7, 1, {(stai_ptr) conv2d_7_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_7_t_in_0_ptr_const_s8, conv2d_7_t_in_0_shape_w_const_u16, conv2d_7_t_in_0_shape_h_const_u16, conv2d_7_l_stride_1_const_u16, conv2d_7_l_stride_0_const_u16, conv2d_7_t_in_0_shape_ch_const_u16, conv2d_7_t_weight_0_ptr_const_s8, conv2d_7_t_out_0_shape_ch_const_u16, conv2d_7_t_weight_1_ptr_const_s32, conv2d_7_t_in_0_fmt_zero_const_s8, conv2d_7_t_out_0_fmt_zero_const_s8, conv2d_7_t_in_0_fmt_scale_const_f32, conv2d_7_t_out_0_fmt_scale_const_f32, conv2d_7_t_weight_0_fmt_scale_const_f32, conv2d_7_l_out_ch_format_const_layer_format_type, conv2d_7_t_out_0_ptr_s8, 1, 272, conv2d_7_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(7, 1, {(stai_ptr) conv2d_7_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_7 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_8 */
  {
      const ai_i8* conv2d_8_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 272);
    const ai_i8* conv2d_8_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 2488);
    const ai_i32* conv2d_8_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 2872);
    ai_i8* conv2d_8_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5392);
    ai_i16* conv2d_8_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 4880);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(8, 1, {(stai_ptr) conv2d_8_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_8_t_in_0_ptr_const_s8, conv2d_8_t_in_0_shape_w_const_u16, conv2d_8_t_in_0_shape_h_const_u16, conv2d_8_l_stride_1_const_u16, conv2d_8_l_stride_0_const_u16, conv2d_8_t_in_0_shape_ch_const_u16, conv2d_8_t_weight_0_ptr_const_s8, conv2d_8_t_out_0_shape_ch_const_u16, conv2d_8_t_weight_1_ptr_const_s32, conv2d_8_t_in_0_fmt_zero_const_s8, conv2d_8_t_out_0_fmt_zero_const_s8, conv2d_8_t_in_0_fmt_scale_const_f32, conv2d_8_t_out_0_fmt_scale_const_f32, conv2d_8_t_weight_0_fmt_scale_const_f32, conv2d_8_l_out_ch_format_const_layer_format_type, conv2d_8_t_out_0_ptr_s8, 1, 512, conv2d_8_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(8, 1, {(stai_ptr) conv2d_8_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_8 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_9_pad_before */
  {
      const ai_ptr conv2d_9_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 5392);
    ai_ptr conv2d_9_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 33040);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(9, 1, {(stai_ptr) conv2d_9_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_9_pad_before_t_in_0_ptr_const_ptr, conv2d_9_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_9_pad_before_v_pad_constant_value_const_s8), conv2d_9_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_9_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(1248), (ai_i32)(1248), (ai_i32)(48), (ai_i32)(48));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(9, 1, {(stai_ptr) conv2d_9_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_9_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_9 */
  {
      const ai_i8* conv2d_9_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 33040);
    const ai_i8* conv2d_9_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 3064);
    const ai_i32* conv2d_9_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 3496);
    ai_i8* conv2d_9_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 65488);
    ai_i16* conv2d_9_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 4880);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(9, 1, {(stai_ptr) conv2d_9_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_9_t_in_0_ptr_const_s8, conv2d_9_t_in_0_shape_w_const_u16, conv2d_9_t_in_0_shape_h_const_u16, conv2d_9_t_in_0_shape_ch_const_u16, conv2d_9_t_weight_0_ptr_const_s8, conv2d_9_l_stride_1_const_u16, conv2d_9_l_stride_0_const_u16, conv2d_9_t_weight_1_ptr_const_s32, conv2d_9_t_in_0_fmt_zero_const_s8, conv2d_9_t_out_0_fmt_zero_const_s8, conv2d_9_t_in_0_fmt_scale_const_f32, conv2d_9_t_out_0_fmt_scale_const_f32, conv2d_9_t_weight_0_fmt_scale_const_f32, conv2d_9_t_out_0_ptr_s8, conv2d_9_t_out_0_shape_w_const_u16, conv2d_9_t_out_0_shape_h_const_u16, 0, 1777, conv2d_9_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(9, 1, {(stai_ptr) conv2d_9_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_9 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_10 */
  {
      const ai_i8* conv2d_10_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 65488);
    const ai_i8* conv2d_10_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 3688);
    const ai_i32* conv2d_10_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 4072);
    ai_i8* conv2d_10_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4880);
    ai_i16* conv2d_10_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(10, 1, {(stai_ptr) conv2d_10_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_10_t_in_0_ptr_const_s8, conv2d_10_t_in_0_shape_w_const_u16, conv2d_10_t_in_0_shape_h_const_u16, conv2d_10_l_stride_1_const_u16, conv2d_10_l_stride_0_const_u16, conv2d_10_t_in_0_shape_ch_const_u16, conv2d_10_t_weight_0_ptr_const_s8, conv2d_10_t_out_0_shape_ch_const_u16, conv2d_10_t_weight_1_ptr_const_s32, conv2d_10_t_in_0_fmt_zero_const_s8, conv2d_10_t_out_0_fmt_zero_const_s8, conv2d_10_t_in_0_fmt_scale_const_f32, conv2d_10_t_out_0_fmt_scale_const_f32, conv2d_10_t_weight_0_fmt_scale_const_f32, conv2d_10_l_out_ch_format_const_layer_format_type, conv2d_10_t_out_0_ptr_s8, 1, 272, conv2d_10_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(10, 1, {(stai_ptr) conv2d_10_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_10 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_11 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_11(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_11 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_12 */
  {
      const ai_i8* conv2d_12_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 9488);
    const ai_i8* conv2d_12_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 4104);
    const ai_i32* conv2d_12_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 4488);
    ai_i8* conv2d_12_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 14096);
    ai_i16* conv2d_12_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(12, 1, {(stai_ptr) conv2d_12_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_12_t_in_0_ptr_const_s8, conv2d_12_t_in_0_shape_w_const_u16, conv2d_12_t_in_0_shape_h_const_u16, conv2d_12_l_stride_1_const_u16, conv2d_12_l_stride_0_const_u16, conv2d_12_t_in_0_shape_ch_const_u16, conv2d_12_t_weight_0_ptr_const_s8, conv2d_12_t_out_0_shape_ch_const_u16, conv2d_12_t_weight_1_ptr_const_s32, conv2d_12_t_in_0_fmt_zero_const_s8, conv2d_12_t_out_0_fmt_zero_const_s8, conv2d_12_t_in_0_fmt_scale_const_f32, conv2d_12_t_out_0_fmt_scale_const_f32, conv2d_12_t_weight_0_fmt_scale_const_f32, conv2d_12_l_out_ch_format_const_layer_format_type, conv2d_12_t_out_0_ptr_s8, 1, 512, conv2d_12_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(12, 1, {(stai_ptr) conv2d_12_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_12 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_13_pad_before */
  {
      const ai_ptr conv2d_13_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 14096);
    ai_ptr conv2d_13_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 41744);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(13, 1, {(stai_ptr) conv2d_13_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_13_pad_before_t_in_0_ptr_const_ptr, conv2d_13_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_13_pad_before_v_pad_constant_value_const_s8), conv2d_13_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_13_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(0), (ai_i32)(2496), (ai_i32)(0), (ai_i32)(96));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(13, 1, {(stai_ptr) conv2d_13_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_13_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_13 */
  {
      const ai_i8* conv2d_13_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 41744);
    const ai_i8* conv2d_13_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 4680);
    const ai_i32* conv2d_13_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 5112);
    ai_i8* conv2d_13_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1780);
    ai_i16* conv2d_13_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(13, 1, {(stai_ptr) conv2d_13_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_13_t_in_0_ptr_const_s8, conv2d_13_t_in_0_shape_w_const_u16, conv2d_13_t_in_0_shape_h_const_u16, conv2d_13_t_in_0_shape_ch_const_u16, conv2d_13_t_weight_0_ptr_const_s8, conv2d_13_l_stride_1_const_u16, conv2d_13_l_stride_0_const_u16, conv2d_13_t_weight_1_ptr_const_s32, conv2d_13_t_in_0_fmt_zero_const_s8, conv2d_13_t_out_0_fmt_zero_const_s8, conv2d_13_t_in_0_fmt_scale_const_f32, conv2d_13_t_out_0_fmt_scale_const_f32, conv2d_13_t_weight_0_fmt_scale_const_f32, conv2d_13_t_out_0_ptr_s8, conv2d_13_t_out_0_shape_w_const_u16, conv2d_13_t_out_0_shape_h_const_u16, 0, 1777, conv2d_13_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(13, 1, {(stai_ptr) conv2d_13_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_13 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_14 */
  {
      const ai_i8* conv2d_14_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1780);
    const ai_i8* conv2d_14_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 5304);
    const ai_i32* conv2d_14_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 6072);
    ai_i8* conv2d_14_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 8692);
    ai_i16* conv2d_14_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(14, 1, {(stai_ptr) conv2d_14_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_14_t_in_0_ptr_const_s8, conv2d_14_t_in_0_shape_w_const_u16, conv2d_14_t_in_0_shape_h_const_u16, conv2d_14_l_stride_1_const_u16, conv2d_14_l_stride_0_const_u16, conv2d_14_t_in_0_shape_ch_const_u16, conv2d_14_t_weight_0_ptr_const_s8, conv2d_14_t_out_0_shape_ch_const_u16, conv2d_14_t_weight_1_ptr_const_s32, conv2d_14_t_in_0_fmt_zero_const_s8, conv2d_14_t_out_0_fmt_zero_const_s8, conv2d_14_t_in_0_fmt_scale_const_f32, conv2d_14_t_out_0_fmt_scale_const_f32, conv2d_14_t_weight_0_fmt_scale_const_f32, conv2d_14_l_out_ch_format_const_layer_format_type, conv2d_14_t_out_0_ptr_s8, 1, 352, conv2d_14_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(14, 1, {(stai_ptr) conv2d_14_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_14 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_15 */
  {
      const ai_i8* conv2d_15_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8692);
    const ai_i8* conv2d_15_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 6136);
    const ai_i32* conv2d_15_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 7672);
    ai_i8* conv2d_15_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 10996);
    ai_i16* conv2d_15_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(15, 1, {(stai_ptr) conv2d_15_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_15_t_in_0_ptr_const_s8, conv2d_15_t_in_0_shape_w_const_u16, conv2d_15_t_in_0_shape_h_const_u16, conv2d_15_l_stride_1_const_u16, conv2d_15_l_stride_0_const_u16, conv2d_15_t_in_0_shape_ch_const_u16, conv2d_15_t_weight_0_ptr_const_s8, conv2d_15_t_out_0_shape_ch_const_u16, conv2d_15_t_weight_1_ptr_const_s32, conv2d_15_t_in_0_fmt_zero_const_s8, conv2d_15_t_out_0_fmt_zero_const_s8, conv2d_15_t_in_0_fmt_scale_const_f32, conv2d_15_t_out_0_fmt_scale_const_f32, conv2d_15_t_weight_0_fmt_scale_const_f32, conv2d_15_l_out_ch_format_const_layer_format_type, conv2d_15_t_out_0_ptr_s8, 1, 1024, conv2d_15_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(15, 1, {(stai_ptr) conv2d_15_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_15 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_16_pad_before */
  {
      const ai_ptr conv2d_16_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 10996);
    ai_ptr conv2d_16_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 24820);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(16, 1, {(stai_ptr) conv2d_16_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_16_pad_before_t_in_0_ptr_const_ptr, conv2d_16_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_16_pad_before_v_pad_constant_value_const_s8), conv2d_16_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_16_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(1344), (ai_i32)(1344), (ai_i32)(96), (ai_i32)(96));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(16, 1, {(stai_ptr) conv2d_16_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_16_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_16 */
  {
      const ai_i8* conv2d_16_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 24820);
    const ai_i8* conv2d_16_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 8056);
    const ai_i32* conv2d_16_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 8920);
    ai_i8* conv2d_16_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 10996);
    ai_i16* conv2d_16_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(16, 1, {(stai_ptr) conv2d_16_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_16_t_in_0_ptr_const_s8, conv2d_16_t_in_0_shape_w_const_u16, conv2d_16_t_in_0_shape_h_const_u16, conv2d_16_t_in_0_shape_ch_const_u16, conv2d_16_t_weight_0_ptr_const_s8, conv2d_16_l_stride_1_const_u16, conv2d_16_l_stride_0_const_u16, conv2d_16_t_weight_1_ptr_const_s32, conv2d_16_t_in_0_fmt_zero_const_s8, conv2d_16_t_out_0_fmt_zero_const_s8, conv2d_16_t_in_0_fmt_scale_const_f32, conv2d_16_t_out_0_fmt_scale_const_f32, conv2d_16_t_weight_0_fmt_scale_const_f32, conv2d_16_t_out_0_ptr_s8, conv2d_16_t_out_0_shape_w_const_u16, conv2d_16_t_out_0_shape_h_const_u16, 0, 3553, conv2d_16_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(16, 1, {(stai_ptr) conv2d_16_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_16 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_17 */
  {
      const ai_i8* conv2d_17_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 10996);
    const ai_i8* conv2d_17_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 9304);
    const ai_i32* conv2d_17_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 10840);
    ai_i8* conv2d_17_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 544);
    ai_i16* conv2d_17_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(17, 1, {(stai_ptr) conv2d_17_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_17_t_in_0_ptr_const_s8, conv2d_17_t_in_0_shape_w_const_u16, conv2d_17_t_in_0_shape_h_const_u16, conv2d_17_l_stride_1_const_u16, conv2d_17_l_stride_0_const_u16, conv2d_17_t_in_0_shape_ch_const_u16, conv2d_17_t_weight_0_ptr_const_s8, conv2d_17_t_out_0_shape_ch_const_u16, conv2d_17_t_weight_1_ptr_const_s32, conv2d_17_t_in_0_fmt_zero_const_s8, conv2d_17_t_out_0_fmt_zero_const_s8, conv2d_17_t_in_0_fmt_scale_const_f32, conv2d_17_t_out_0_fmt_scale_const_f32, conv2d_17_t_weight_0_fmt_scale_const_f32, conv2d_17_l_out_ch_format_const_layer_format_type, conv2d_17_t_out_0_ptr_s8, 1, 544, conv2d_17_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(17, 1, {(stai_ptr) conv2d_17_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_17 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_18 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_18(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_18 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_19 */
  {
      const ai_i8* conv2d_19_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 2848);
    const ai_i8* conv2d_19_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 10904);
    const ai_i32* conv2d_19_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 12440);
    ai_i8* conv2d_19_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5152);
    ai_i16* conv2d_19_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(19, 1, {(stai_ptr) conv2d_19_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_19_t_in_0_ptr_const_s8, conv2d_19_t_in_0_shape_w_const_u16, conv2d_19_t_in_0_shape_h_const_u16, conv2d_19_l_stride_1_const_u16, conv2d_19_l_stride_0_const_u16, conv2d_19_t_in_0_shape_ch_const_u16, conv2d_19_t_weight_0_ptr_const_s8, conv2d_19_t_out_0_shape_ch_const_u16, conv2d_19_t_weight_1_ptr_const_s32, conv2d_19_t_in_0_fmt_zero_const_s8, conv2d_19_t_out_0_fmt_zero_const_s8, conv2d_19_t_in_0_fmt_scale_const_f32, conv2d_19_t_out_0_fmt_scale_const_f32, conv2d_19_t_weight_0_fmt_scale_const_f32, conv2d_19_l_out_ch_format_const_layer_format_type, conv2d_19_t_out_0_ptr_s8, 1, 1024, conv2d_19_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(19, 1, {(stai_ptr) conv2d_19_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_19 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_20_pad_before */
  {
      const ai_ptr conv2d_20_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 5152);
    ai_ptr conv2d_20_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 18976);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(20, 1, {(stai_ptr) conv2d_20_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_20_pad_before_t_in_0_ptr_const_ptr, conv2d_20_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_20_pad_before_v_pad_constant_value_const_s8), conv2d_20_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_20_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(1344), (ai_i32)(1344), (ai_i32)(96), (ai_i32)(96));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(20, 1, {(stai_ptr) conv2d_20_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_20_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_20 */
  {
      const ai_i8* conv2d_20_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 18976);
    const ai_i8* conv2d_20_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 12824);
    const ai_i32* conv2d_20_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 13688);
    ai_i8* conv2d_20_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 37792);
    ai_i16* conv2d_20_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 5152);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(20, 1, {(stai_ptr) conv2d_20_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_20_t_in_0_ptr_const_s8, conv2d_20_t_in_0_shape_w_const_u16, conv2d_20_t_in_0_shape_h_const_u16, conv2d_20_t_in_0_shape_ch_const_u16, conv2d_20_t_weight_0_ptr_const_s8, conv2d_20_l_stride_1_const_u16, conv2d_20_l_stride_0_const_u16, conv2d_20_t_weight_1_ptr_const_s32, conv2d_20_t_in_0_fmt_zero_const_s8, conv2d_20_t_out_0_fmt_zero_const_s8, conv2d_20_t_in_0_fmt_scale_const_f32, conv2d_20_t_out_0_fmt_scale_const_f32, conv2d_20_t_weight_0_fmt_scale_const_f32, conv2d_20_t_out_0_ptr_s8, conv2d_20_t_out_0_shape_w_const_u16, conv2d_20_t_out_0_shape_h_const_u16, 0, 3553, conv2d_20_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(20, 1, {(stai_ptr) conv2d_20_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_20 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_21 */
  {
      const ai_i8* conv2d_21_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 37792);
    const ai_i8* conv2d_21_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 14072);
    const ai_i32* conv2d_21_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 15608);
    ai_i8* conv2d_21_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 544);
    ai_i16* conv2d_21_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(21, 1, {(stai_ptr) conv2d_21_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_21_t_in_0_ptr_const_s8, conv2d_21_t_in_0_shape_w_const_u16, conv2d_21_t_in_0_shape_h_const_u16, conv2d_21_l_stride_1_const_u16, conv2d_21_l_stride_0_const_u16, conv2d_21_t_in_0_shape_ch_const_u16, conv2d_21_t_weight_0_ptr_const_s8, conv2d_21_t_out_0_shape_ch_const_u16, conv2d_21_t_weight_1_ptr_const_s32, conv2d_21_t_in_0_fmt_zero_const_s8, conv2d_21_t_out_0_fmt_zero_const_s8, conv2d_21_t_in_0_fmt_scale_const_f32, conv2d_21_t_out_0_fmt_scale_const_f32, conv2d_21_t_weight_0_fmt_scale_const_f32, conv2d_21_l_out_ch_format_const_layer_format_type, conv2d_21_t_out_0_ptr_s8, 1, 544, conv2d_21_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(21, 1, {(stai_ptr) conv2d_21_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_21 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_22 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_22(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_22 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_23 */
  {
      const ai_i8* conv2d_23_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 5152);
    const ai_i8* conv2d_23_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 15672);
    const ai_i32* conv2d_23_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 17208);
    ai_i8* conv2d_23_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 7456);
    ai_i16* conv2d_23_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(23, 1, {(stai_ptr) conv2d_23_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_23_t_in_0_ptr_const_s8, conv2d_23_t_in_0_shape_w_const_u16, conv2d_23_t_in_0_shape_h_const_u16, conv2d_23_l_stride_1_const_u16, conv2d_23_l_stride_0_const_u16, conv2d_23_t_in_0_shape_ch_const_u16, conv2d_23_t_weight_0_ptr_const_s8, conv2d_23_t_out_0_shape_ch_const_u16, conv2d_23_t_weight_1_ptr_const_s32, conv2d_23_t_in_0_fmt_zero_const_s8, conv2d_23_t_out_0_fmt_zero_const_s8, conv2d_23_t_in_0_fmt_scale_const_f32, conv2d_23_t_out_0_fmt_scale_const_f32, conv2d_23_t_weight_0_fmt_scale_const_f32, conv2d_23_l_out_ch_format_const_layer_format_type, conv2d_23_t_out_0_ptr_s8, 1, 1024, conv2d_23_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(23, 1, {(stai_ptr) conv2d_23_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_23 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_24_pad_before */
  {
      const ai_ptr conv2d_24_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 7456);
    ai_ptr conv2d_24_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 21280);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(24, 1, {(stai_ptr) conv2d_24_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_24_pad_before_t_in_0_ptr_const_ptr, conv2d_24_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_24_pad_before_v_pad_constant_value_const_s8), conv2d_24_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_24_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(0), (ai_i32)(2688), (ai_i32)(0), (ai_i32)(192));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(24, 1, {(stai_ptr) conv2d_24_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_24_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_24 */
  {
      const ai_i8* conv2d_24_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 21280);
    const ai_i8* conv2d_24_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 17592);
    const ai_i32* conv2d_24_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 18456);
    ai_i8* conv2d_24_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 3556);
    ai_i16* conv2d_24_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(24, 1, {(stai_ptr) conv2d_24_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_24_t_in_0_ptr_const_s8, conv2d_24_t_in_0_shape_w_const_u16, conv2d_24_t_in_0_shape_h_const_u16, conv2d_24_t_in_0_shape_ch_const_u16, conv2d_24_t_weight_0_ptr_const_s8, conv2d_24_l_stride_1_const_u16, conv2d_24_l_stride_0_const_u16, conv2d_24_t_weight_1_ptr_const_s32, conv2d_24_t_in_0_fmt_zero_const_s8, conv2d_24_t_out_0_fmt_zero_const_s8, conv2d_24_t_in_0_fmt_scale_const_f32, conv2d_24_t_out_0_fmt_scale_const_f32, conv2d_24_t_weight_0_fmt_scale_const_f32, conv2d_24_t_out_0_ptr_s8, conv2d_24_t_out_0_shape_w_const_u16, conv2d_24_t_out_0_shape_h_const_u16, 0, 3553, conv2d_24_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(24, 1, {(stai_ptr) conv2d_24_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_24 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_25 */
  {
      const ai_i8* conv2d_25_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 3556);
    const ai_i8* conv2d_25_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 18840);
    const ai_i32* conv2d_25_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 21144);
    ai_i8* conv2d_25_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 624);
    ai_i16* conv2d_25_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(25, 1, {(stai_ptr) conv2d_25_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_25_t_in_0_ptr_const_s8, conv2d_25_t_in_0_shape_w_const_u16, conv2d_25_t_in_0_shape_h_const_u16, conv2d_25_l_stride_1_const_u16, conv2d_25_l_stride_0_const_u16, conv2d_25_t_in_0_shape_ch_const_u16, conv2d_25_t_weight_0_ptr_const_s8, conv2d_25_t_out_0_shape_ch_const_u16, conv2d_25_t_weight_1_ptr_const_s32, conv2d_25_t_in_0_fmt_zero_const_s8, conv2d_25_t_out_0_fmt_zero_const_s8, conv2d_25_t_in_0_fmt_scale_const_f32, conv2d_25_t_out_0_fmt_scale_const_f32, conv2d_25_t_weight_0_fmt_scale_const_f32, conv2d_25_l_out_ch_format_const_layer_format_type, conv2d_25_t_out_0_ptr_s8, 1, 624, conv2d_25_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(25, 1, {(stai_ptr) conv2d_25_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_25 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_26 */
  {
      const ai_i8* conv2d_26_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 624);
    const ai_i8* conv2d_26_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 21240);
    const ai_i32* conv2d_26_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 24696);
    ai_i8* conv2d_26_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 3024);
    ai_i16* conv2d_26_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1488);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(26, 1, {(stai_ptr) conv2d_26_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_26_t_in_0_ptr_const_s8, conv2d_26_t_in_0_shape_w_const_u16, conv2d_26_t_in_0_shape_h_const_u16, conv2d_26_l_stride_1_const_u16, conv2d_26_l_stride_0_const_u16, conv2d_26_t_in_0_shape_ch_const_u16, conv2d_26_t_weight_0_ptr_const_s8, conv2d_26_t_out_0_shape_ch_const_u16, conv2d_26_t_weight_1_ptr_const_s32, conv2d_26_t_in_0_fmt_zero_const_s8, conv2d_26_t_out_0_fmt_zero_const_s8, conv2d_26_t_in_0_fmt_scale_const_f32, conv2d_26_t_out_0_fmt_scale_const_f32, conv2d_26_t_weight_0_fmt_scale_const_f32, conv2d_26_l_out_ch_format_const_layer_format_type, conv2d_26_t_out_0_ptr_s8, 1, 1536, conv2d_26_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(26, 1, {(stai_ptr) conv2d_26_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_26 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_27_pad_before */
  {
      const ai_ptr conv2d_27_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 3024);
    ai_ptr conv2d_27_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 8208);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(27, 1, {(stai_ptr) conv2d_27_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_27_pad_before_t_in_0_ptr_const_ptr, conv2d_27_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_27_pad_before_v_pad_constant_value_const_s8), conv2d_27_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_27_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(864), (ai_i32)(1152), (ai_i32)(1152), (ai_i32)(144), (ai_i32)(144));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(27, 1, {(stai_ptr) conv2d_27_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_27_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_27 */
  {
      const ai_i8* conv2d_27_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8208);
    const ai_i8* conv2d_27_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 25272);
    const ai_i32* conv2d_27_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 26568);
    ai_i8* conv2d_27_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 17424);
    ai_i16* conv2d_27_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1488);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(27, 1, {(stai_ptr) conv2d_27_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_27_t_in_0_ptr_const_s8, conv2d_27_t_in_0_shape_w_const_u16, conv2d_27_t_in_0_shape_h_const_u16, conv2d_27_t_in_0_shape_ch_const_u16, conv2d_27_t_weight_0_ptr_const_s8, conv2d_27_l_stride_1_const_u16, conv2d_27_l_stride_0_const_u16, conv2d_27_t_weight_1_ptr_const_s32, conv2d_27_t_in_0_fmt_zero_const_s8, conv2d_27_t_out_0_fmt_zero_const_s8, conv2d_27_t_in_0_fmt_scale_const_f32, conv2d_27_t_out_0_fmt_scale_const_f32, conv2d_27_t_weight_0_fmt_scale_const_f32, conv2d_27_t_out_0_ptr_s8, conv2d_27_t_out_0_shape_w_const_u16, conv2d_27_t_out_0_shape_h_const_u16, 0, 5329, conv2d_27_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(27, 1, {(stai_ptr) conv2d_27_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_27 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_28 */
  {
      const ai_i8* conv2d_28_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 17424);
    const ai_i8* conv2d_28_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 27144);
    const ai_i32* conv2d_28_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 30600);
    ai_i8* conv2d_28_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 2304);
    ai_i16* conv2d_28_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1488);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(28, 1, {(stai_ptr) conv2d_28_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_28_t_in_0_ptr_const_s8, conv2d_28_t_in_0_shape_w_const_u16, conv2d_28_t_in_0_shape_h_const_u16, conv2d_28_l_stride_1_const_u16, conv2d_28_l_stride_0_const_u16, conv2d_28_t_in_0_shape_ch_const_u16, conv2d_28_t_weight_0_ptr_const_s8, conv2d_28_t_out_0_shape_ch_const_u16, conv2d_28_t_weight_1_ptr_const_s32, conv2d_28_t_in_0_fmt_zero_const_s8, conv2d_28_t_out_0_fmt_zero_const_s8, conv2d_28_t_in_0_fmt_scale_const_f32, conv2d_28_t_out_0_fmt_scale_const_f32, conv2d_28_t_weight_0_fmt_scale_const_f32, conv2d_28_l_out_ch_format_const_layer_format_type, conv2d_28_t_out_0_ptr_s8, 1, 816, conv2d_28_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(28, 1, {(stai_ptr) conv2d_28_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_28 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_29 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_29(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_29 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_30 */
  {
      const ai_i8* conv2d_30_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 3168);
    const ai_i8* conv2d_30_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 30696);
    const ai_i32* conv2d_30_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 34152);
    ai_i8* conv2d_30_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4032);
    ai_i16* conv2d_30_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(30, 1, {(stai_ptr) conv2d_30_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_30_t_in_0_ptr_const_s8, conv2d_30_t_in_0_shape_w_const_u16, conv2d_30_t_in_0_shape_h_const_u16, conv2d_30_l_stride_1_const_u16, conv2d_30_l_stride_0_const_u16, conv2d_30_t_in_0_shape_ch_const_u16, conv2d_30_t_weight_0_ptr_const_s8, conv2d_30_t_out_0_shape_ch_const_u16, conv2d_30_t_weight_1_ptr_const_s32, conv2d_30_t_in_0_fmt_zero_const_s8, conv2d_30_t_out_0_fmt_zero_const_s8, conv2d_30_t_in_0_fmt_scale_const_f32, conv2d_30_t_out_0_fmt_scale_const_f32, conv2d_30_t_weight_0_fmt_scale_const_f32, conv2d_30_l_out_ch_format_const_layer_format_type, conv2d_30_t_out_0_ptr_s8, 1, 1536, conv2d_30_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(30, 1, {(stai_ptr) conv2d_30_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_30 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_31_pad_before */
  {
      const ai_ptr conv2d_31_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 4032);
    ai_ptr conv2d_31_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 9216);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(31, 1, {(stai_ptr) conv2d_31_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_31_pad_before_t_in_0_ptr_const_ptr, conv2d_31_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_31_pad_before_v_pad_constant_value_const_s8), conv2d_31_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_31_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(864), (ai_i32)(1152), (ai_i32)(1152), (ai_i32)(144), (ai_i32)(144));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(31, 1, {(stai_ptr) conv2d_31_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_31_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_31 */
  {
      const ai_i8* conv2d_31_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 9216);
    const ai_i8* conv2d_31_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 34728);
    const ai_i32* conv2d_31_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 36024);
    ai_i8* conv2d_31_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4032);
    ai_i16* conv2d_31_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 18432);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(31, 1, {(stai_ptr) conv2d_31_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_31_t_in_0_ptr_const_s8, conv2d_31_t_in_0_shape_w_const_u16, conv2d_31_t_in_0_shape_h_const_u16, conv2d_31_t_in_0_shape_ch_const_u16, conv2d_31_t_weight_0_ptr_const_s8, conv2d_31_l_stride_1_const_u16, conv2d_31_l_stride_0_const_u16, conv2d_31_t_weight_1_ptr_const_s32, conv2d_31_t_in_0_fmt_zero_const_s8, conv2d_31_t_out_0_fmt_zero_const_s8, conv2d_31_t_in_0_fmt_scale_const_f32, conv2d_31_t_out_0_fmt_scale_const_f32, conv2d_31_t_weight_0_fmt_scale_const_f32, conv2d_31_t_out_0_ptr_s8, conv2d_31_t_out_0_shape_w_const_u16, conv2d_31_t_out_0_shape_h_const_u16, 0, 5329, conv2d_31_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(31, 1, {(stai_ptr) conv2d_31_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_31 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_32 */
  {
      const ai_i8* conv2d_32_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 4032);
    const ai_i8* conv2d_32_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 36600);
    const ai_i32* conv2d_32_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 40056);
    ai_i8* conv2d_32_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 816);
    ai_i16* conv2d_32_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(32, 1, {(stai_ptr) conv2d_32_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_32_t_in_0_ptr_const_s8, conv2d_32_t_in_0_shape_w_const_u16, conv2d_32_t_in_0_shape_h_const_u16, conv2d_32_l_stride_1_const_u16, conv2d_32_l_stride_0_const_u16, conv2d_32_t_in_0_shape_ch_const_u16, conv2d_32_t_weight_0_ptr_const_s8, conv2d_32_t_out_0_shape_ch_const_u16, conv2d_32_t_weight_1_ptr_const_s32, conv2d_32_t_in_0_fmt_zero_const_s8, conv2d_32_t_out_0_fmt_zero_const_s8, conv2d_32_t_in_0_fmt_scale_const_f32, conv2d_32_t_out_0_fmt_scale_const_f32, conv2d_32_t_weight_0_fmt_scale_const_f32, conv2d_32_l_out_ch_format_const_layer_format_type, conv2d_32_t_out_0_ptr_s8, 1, 816, conv2d_32_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(32, 1, {(stai_ptr) conv2d_32_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_32 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_33 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_33(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_33 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_34 */
  {
      const ai_i8* conv2d_34_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1680);
    const ai_i8* conv2d_34_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 40152);
    const ai_i32* conv2d_34_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 43608);
    ai_i8* conv2d_34_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 2544);
    ai_i16* conv2d_34_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(34, 1, {(stai_ptr) conv2d_34_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_34_t_in_0_ptr_const_s8, conv2d_34_t_in_0_shape_w_const_u16, conv2d_34_t_in_0_shape_h_const_u16, conv2d_34_l_stride_1_const_u16, conv2d_34_l_stride_0_const_u16, conv2d_34_t_in_0_shape_ch_const_u16, conv2d_34_t_weight_0_ptr_const_s8, conv2d_34_t_out_0_shape_ch_const_u16, conv2d_34_t_weight_1_ptr_const_s32, conv2d_34_t_in_0_fmt_zero_const_s8, conv2d_34_t_out_0_fmt_zero_const_s8, conv2d_34_t_in_0_fmt_scale_const_f32, conv2d_34_t_out_0_fmt_scale_const_f32, conv2d_34_t_weight_0_fmt_scale_const_f32, conv2d_34_l_out_ch_format_const_layer_format_type, conv2d_34_t_out_0_ptr_s8, 1, 1536, conv2d_34_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(34, 1, {(stai_ptr) conv2d_34_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_34 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_35_pad_before */
  {
      const ai_ptr conv2d_35_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 2544);
    ai_ptr conv2d_35_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 7728);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(35, 1, {(stai_ptr) conv2d_35_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_35_pad_before_t_in_0_ptr_const_ptr, conv2d_35_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_35_pad_before_v_pad_constant_value_const_s8), conv2d_35_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_35_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(864), (ai_i32)(1152), (ai_i32)(1152), (ai_i32)(144), (ai_i32)(144));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(35, 1, {(stai_ptr) conv2d_35_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_35_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_35 */
  {
      const ai_i8* conv2d_35_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 7728);
    const ai_i8* conv2d_35_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 44184);
    const ai_i32* conv2d_35_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 45480);
    ai_i8* conv2d_35_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 2544);
    ai_i16* conv2d_35_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 16944);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(35, 1, {(stai_ptr) conv2d_35_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_35_t_in_0_ptr_const_s8, conv2d_35_t_in_0_shape_w_const_u16, conv2d_35_t_in_0_shape_h_const_u16, conv2d_35_t_in_0_shape_ch_const_u16, conv2d_35_t_weight_0_ptr_const_s8, conv2d_35_l_stride_1_const_u16, conv2d_35_l_stride_0_const_u16, conv2d_35_t_weight_1_ptr_const_s32, conv2d_35_t_in_0_fmt_zero_const_s8, conv2d_35_t_out_0_fmt_zero_const_s8, conv2d_35_t_in_0_fmt_scale_const_f32, conv2d_35_t_out_0_fmt_scale_const_f32, conv2d_35_t_weight_0_fmt_scale_const_f32, conv2d_35_t_out_0_ptr_s8, conv2d_35_t_out_0_shape_w_const_u16, conv2d_35_t_out_0_shape_h_const_u16, 0, 5329, conv2d_35_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(35, 1, {(stai_ptr) conv2d_35_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_35 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_36 */
  {
      const ai_i8* conv2d_36_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 2544);
    const ai_i8* conv2d_36_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 46056);
    const ai_i32* conv2d_36_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 49512);
    ai_i8* conv2d_36_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 816);
    ai_i16* conv2d_36_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(36, 1, {(stai_ptr) conv2d_36_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_36_t_in_0_ptr_const_s8, conv2d_36_t_in_0_shape_w_const_u16, conv2d_36_t_in_0_shape_h_const_u16, conv2d_36_l_stride_1_const_u16, conv2d_36_l_stride_0_const_u16, conv2d_36_t_in_0_shape_ch_const_u16, conv2d_36_t_weight_0_ptr_const_s8, conv2d_36_t_out_0_shape_ch_const_u16, conv2d_36_t_weight_1_ptr_const_s32, conv2d_36_t_in_0_fmt_zero_const_s8, conv2d_36_t_out_0_fmt_zero_const_s8, conv2d_36_t_in_0_fmt_scale_const_f32, conv2d_36_t_out_0_fmt_scale_const_f32, conv2d_36_t_weight_0_fmt_scale_const_f32, conv2d_36_l_out_ch_format_const_layer_format_type, conv2d_36_t_out_0_ptr_s8, 1, 816, conv2d_36_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(36, 1, {(stai_ptr) conv2d_36_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_36 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_37 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_37(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_37 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_38 */
  {
      const ai_i8* conv2d_38_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 2544);
    const ai_i8* conv2d_38_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 49608);
    const ai_i32* conv2d_38_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 53064);
    ai_i8* conv2d_38_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 3408);
    ai_i16* conv2d_38_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(38, 1, {(stai_ptr) conv2d_38_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_38_t_in_0_ptr_const_s8, conv2d_38_t_in_0_shape_w_const_u16, conv2d_38_t_in_0_shape_h_const_u16, conv2d_38_l_stride_1_const_u16, conv2d_38_l_stride_0_const_u16, conv2d_38_t_in_0_shape_ch_const_u16, conv2d_38_t_weight_0_ptr_const_s8, conv2d_38_t_out_0_shape_ch_const_u16, conv2d_38_t_weight_1_ptr_const_s32, conv2d_38_t_in_0_fmt_zero_const_s8, conv2d_38_t_out_0_fmt_zero_const_s8, conv2d_38_t_in_0_fmt_scale_const_f32, conv2d_38_t_out_0_fmt_scale_const_f32, conv2d_38_t_weight_0_fmt_scale_const_f32, conv2d_38_l_out_ch_format_const_layer_format_type, conv2d_38_t_out_0_ptr_s8, 1, 1536, conv2d_38_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(38, 1, {(stai_ptr) conv2d_38_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_38 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_39_pad_before */
  {
      const ai_ptr conv2d_39_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 3408);
    ai_ptr conv2d_39_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 8592);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(39, 1, {(stai_ptr) conv2d_39_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_39_pad_before_t_in_0_ptr_const_ptr, conv2d_39_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_39_pad_before_v_pad_constant_value_const_s8), conv2d_39_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_39_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(864), (ai_i32)(1152), (ai_i32)(1152), (ai_i32)(144), (ai_i32)(144));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(39, 1, {(stai_ptr) conv2d_39_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_39_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_39 */
  {
      const ai_i8* conv2d_39_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8592);
    const ai_i8* conv2d_39_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 53640);
    const ai_i32* conv2d_39_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 54936);
    ai_i8* conv2d_39_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 17808);
    ai_i16* conv2d_39_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(39, 1, {(stai_ptr) conv2d_39_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_39_t_in_0_ptr_const_s8, conv2d_39_t_in_0_shape_w_const_u16, conv2d_39_t_in_0_shape_h_const_u16, conv2d_39_t_in_0_shape_ch_const_u16, conv2d_39_t_weight_0_ptr_const_s8, conv2d_39_l_stride_1_const_u16, conv2d_39_l_stride_0_const_u16, conv2d_39_t_weight_1_ptr_const_s32, conv2d_39_t_in_0_fmt_zero_const_s8, conv2d_39_t_out_0_fmt_zero_const_s8, conv2d_39_t_in_0_fmt_scale_const_f32, conv2d_39_t_out_0_fmt_scale_const_f32, conv2d_39_t_weight_0_fmt_scale_const_f32, conv2d_39_t_out_0_ptr_s8, conv2d_39_t_out_0_shape_w_const_u16, conv2d_39_t_out_0_shape_h_const_u16, 0, 5329, conv2d_39_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(39, 1, {(stai_ptr) conv2d_39_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_39 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_40 */
  {
      const ai_i8* conv2d_40_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 17808);
    const ai_i8* conv2d_40_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 55512);
    const ai_i32* conv2d_40_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 60120);
    ai_i8* conv2d_40_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 896);
    ai_i16* conv2d_40_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(40, 1, {(stai_ptr) conv2d_40_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_40_t_in_0_ptr_const_s8, conv2d_40_t_in_0_shape_w_const_u16, conv2d_40_t_in_0_shape_h_const_u16, conv2d_40_l_stride_1_const_u16, conv2d_40_l_stride_0_const_u16, conv2d_40_t_in_0_shape_ch_const_u16, conv2d_40_t_weight_0_ptr_const_s8, conv2d_40_t_out_0_shape_ch_const_u16, conv2d_40_t_weight_1_ptr_const_s32, conv2d_40_t_in_0_fmt_zero_const_s8, conv2d_40_t_out_0_fmt_zero_const_s8, conv2d_40_t_in_0_fmt_scale_const_f32, conv2d_40_t_out_0_fmt_scale_const_f32, conv2d_40_t_weight_0_fmt_scale_const_f32, conv2d_40_l_out_ch_format_const_layer_format_type, conv2d_40_t_out_0_ptr_s8, 1, 896, conv2d_40_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(40, 1, {(stai_ptr) conv2d_40_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_40 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_41 */
  {
      const ai_i8* conv2d_41_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 896);
    const ai_i8* conv2d_41_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 60248);
    const ai_i32* conv2d_41_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 66392);
    ai_i8* conv2d_41_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4096);
    ai_i16* conv2d_41_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 2048);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(41, 1, {(stai_ptr) conv2d_41_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_41_t_in_0_ptr_const_s8, conv2d_41_t_in_0_shape_w_const_u16, conv2d_41_t_in_0_shape_h_const_u16, conv2d_41_l_stride_1_const_u16, conv2d_41_l_stride_0_const_u16, conv2d_41_t_in_0_shape_ch_const_u16, conv2d_41_t_weight_0_ptr_const_s8, conv2d_41_t_out_0_shape_ch_const_u16, conv2d_41_t_weight_1_ptr_const_s32, conv2d_41_t_in_0_fmt_zero_const_s8, conv2d_41_t_out_0_fmt_zero_const_s8, conv2d_41_t_in_0_fmt_scale_const_f32, conv2d_41_t_out_0_fmt_scale_const_f32, conv2d_41_t_weight_0_fmt_scale_const_f32, conv2d_41_l_out_ch_format_const_layer_format_type, conv2d_41_t_out_0_ptr_s8, 1, 2048, conv2d_41_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(41, 1, {(stai_ptr) conv2d_41_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_41 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_42_pad_before */
  {
      const ai_ptr conv2d_42_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 4096);
    ai_ptr conv2d_42_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 11008);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(42, 1, {(stai_ptr) conv2d_42_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_42_pad_before_t_in_0_ptr_const_ptr, conv2d_42_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_42_pad_before_v_pad_constant_value_const_s8), conv2d_42_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_42_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(1536), (ai_i32)(1536), (ai_i32)(192), (ai_i32)(192));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(42, 1, {(stai_ptr) conv2d_42_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_42_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_42 */
  {
      const ai_i8* conv2d_42_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 11008);
    const ai_i8* conv2d_42_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 67160);
    const ai_i32* conv2d_42_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 68888);
    ai_i8* conv2d_42_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 23296);
    ai_i16* conv2d_42_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 2048);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(42, 1, {(stai_ptr) conv2d_42_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_42_t_in_0_ptr_const_s8, conv2d_42_t_in_0_shape_w_const_u16, conv2d_42_t_in_0_shape_h_const_u16, conv2d_42_t_in_0_shape_ch_const_u16, conv2d_42_t_weight_0_ptr_const_s8, conv2d_42_l_stride_1_const_u16, conv2d_42_l_stride_0_const_u16, conv2d_42_t_weight_1_ptr_const_s32, conv2d_42_t_in_0_fmt_zero_const_s8, conv2d_42_t_out_0_fmt_zero_const_s8, conv2d_42_t_in_0_fmt_scale_const_f32, conv2d_42_t_out_0_fmt_scale_const_f32, conv2d_42_t_weight_0_fmt_scale_const_f32, conv2d_42_t_out_0_ptr_s8, conv2d_42_t_out_0_shape_w_const_u16, conv2d_42_t_out_0_shape_h_const_u16, 0, 7105, conv2d_42_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(42, 1, {(stai_ptr) conv2d_42_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_42 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_43 */
  {
      const ai_i8* conv2d_43_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 23296);
    const ai_i8* conv2d_43_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 69656);
    const ai_i32* conv2d_43_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 75800);
    ai_i8* conv2d_43_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 3136);
    ai_i16* conv2d_43_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 2048);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(43, 1, {(stai_ptr) conv2d_43_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_43_t_in_0_ptr_const_s8, conv2d_43_t_in_0_shape_w_const_u16, conv2d_43_t_in_0_shape_h_const_u16, conv2d_43_l_stride_1_const_u16, conv2d_43_l_stride_0_const_u16, conv2d_43_t_in_0_shape_ch_const_u16, conv2d_43_t_weight_0_ptr_const_s8, conv2d_43_t_out_0_shape_ch_const_u16, conv2d_43_t_weight_1_ptr_const_s32, conv2d_43_t_in_0_fmt_zero_const_s8, conv2d_43_t_out_0_fmt_zero_const_s8, conv2d_43_t_in_0_fmt_scale_const_f32, conv2d_43_t_out_0_fmt_scale_const_f32, conv2d_43_t_weight_0_fmt_scale_const_f32, conv2d_43_l_out_ch_format_const_layer_format_type, conv2d_43_t_out_0_ptr_s8, 1, 1088, conv2d_43_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(43, 1, {(stai_ptr) conv2d_43_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_43 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_44 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_44(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_44 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_45 */
  {
      const ai_i8* conv2d_45_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 4288);
    const ai_i8* conv2d_45_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 75928);
    const ai_i32* conv2d_45_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 82072);
    ai_i8* conv2d_45_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5440);
    ai_i16* conv2d_45_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(45, 1, {(stai_ptr) conv2d_45_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_45_t_in_0_ptr_const_s8, conv2d_45_t_in_0_shape_w_const_u16, conv2d_45_t_in_0_shape_h_const_u16, conv2d_45_l_stride_1_const_u16, conv2d_45_l_stride_0_const_u16, conv2d_45_t_in_0_shape_ch_const_u16, conv2d_45_t_weight_0_ptr_const_s8, conv2d_45_t_out_0_shape_ch_const_u16, conv2d_45_t_weight_1_ptr_const_s32, conv2d_45_t_in_0_fmt_zero_const_s8, conv2d_45_t_out_0_fmt_zero_const_s8, conv2d_45_t_in_0_fmt_scale_const_f32, conv2d_45_t_out_0_fmt_scale_const_f32, conv2d_45_t_weight_0_fmt_scale_const_f32, conv2d_45_l_out_ch_format_const_layer_format_type, conv2d_45_t_out_0_ptr_s8, 1, 2048, conv2d_45_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(45, 1, {(stai_ptr) conv2d_45_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_45 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_46_pad_before */
  {
      const ai_ptr conv2d_46_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 5440);
    ai_ptr conv2d_46_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 12352);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(46, 1, {(stai_ptr) conv2d_46_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_46_pad_before_t_in_0_ptr_const_ptr, conv2d_46_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_46_pad_before_v_pad_constant_value_const_s8), conv2d_46_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_46_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(1536), (ai_i32)(1536), (ai_i32)(192), (ai_i32)(192));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(46, 1, {(stai_ptr) conv2d_46_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_46_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_46 */
  {
      const ai_i8* conv2d_46_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 12352);
    const ai_i8* conv2d_46_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 82840);
    const ai_i32* conv2d_46_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 84568);
    ai_i8* conv2d_46_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5440);
    ai_i16* conv2d_46_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 24640);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(46, 1, {(stai_ptr) conv2d_46_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_46_t_in_0_ptr_const_s8, conv2d_46_t_in_0_shape_w_const_u16, conv2d_46_t_in_0_shape_h_const_u16, conv2d_46_t_in_0_shape_ch_const_u16, conv2d_46_t_weight_0_ptr_const_s8, conv2d_46_l_stride_1_const_u16, conv2d_46_l_stride_0_const_u16, conv2d_46_t_weight_1_ptr_const_s32, conv2d_46_t_in_0_fmt_zero_const_s8, conv2d_46_t_out_0_fmt_zero_const_s8, conv2d_46_t_in_0_fmt_scale_const_f32, conv2d_46_t_out_0_fmt_scale_const_f32, conv2d_46_t_weight_0_fmt_scale_const_f32, conv2d_46_t_out_0_ptr_s8, conv2d_46_t_out_0_shape_w_const_u16, conv2d_46_t_out_0_shape_h_const_u16, 0, 7105, conv2d_46_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(46, 1, {(stai_ptr) conv2d_46_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_46 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_47 */
  {
      const ai_i8* conv2d_47_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 5440);
    const ai_i8* conv2d_47_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 85336);
    const ai_i32* conv2d_47_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 91480);
    ai_i8* conv2d_47_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1088);
    ai_i16* conv2d_47_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(47, 1, {(stai_ptr) conv2d_47_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_47_t_in_0_ptr_const_s8, conv2d_47_t_in_0_shape_w_const_u16, conv2d_47_t_in_0_shape_h_const_u16, conv2d_47_l_stride_1_const_u16, conv2d_47_l_stride_0_const_u16, conv2d_47_t_in_0_shape_ch_const_u16, conv2d_47_t_weight_0_ptr_const_s8, conv2d_47_t_out_0_shape_ch_const_u16, conv2d_47_t_weight_1_ptr_const_s32, conv2d_47_t_in_0_fmt_zero_const_s8, conv2d_47_t_out_0_fmt_zero_const_s8, conv2d_47_t_in_0_fmt_scale_const_f32, conv2d_47_t_out_0_fmt_scale_const_f32, conv2d_47_t_weight_0_fmt_scale_const_f32, conv2d_47_l_out_ch_format_const_layer_format_type, conv2d_47_t_out_0_ptr_s8, 1, 1088, conv2d_47_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(47, 1, {(stai_ptr) conv2d_47_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_47 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_48 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_48(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_48 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_49 */
  {
      const ai_i8* conv2d_49_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 2240);
    const ai_i8* conv2d_49_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 91608);
    const ai_i32* conv2d_49_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 97752);
    ai_i8* conv2d_49_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 3392);
    ai_i16* conv2d_49_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(49, 1, {(stai_ptr) conv2d_49_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_49_t_in_0_ptr_const_s8, conv2d_49_t_in_0_shape_w_const_u16, conv2d_49_t_in_0_shape_h_const_u16, conv2d_49_l_stride_1_const_u16, conv2d_49_l_stride_0_const_u16, conv2d_49_t_in_0_shape_ch_const_u16, conv2d_49_t_weight_0_ptr_const_s8, conv2d_49_t_out_0_shape_ch_const_u16, conv2d_49_t_weight_1_ptr_const_s32, conv2d_49_t_in_0_fmt_zero_const_s8, conv2d_49_t_out_0_fmt_zero_const_s8, conv2d_49_t_in_0_fmt_scale_const_f32, conv2d_49_t_out_0_fmt_scale_const_f32, conv2d_49_t_weight_0_fmt_scale_const_f32, conv2d_49_l_out_ch_format_const_layer_format_type, conv2d_49_t_out_0_ptr_s8, 1, 2048, conv2d_49_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(49, 1, {(stai_ptr) conv2d_49_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_49 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_50_pad_before */
  {
      const ai_ptr conv2d_50_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 3392);
    ai_ptr conv2d_50_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 10304);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(50, 1, {(stai_ptr) conv2d_50_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_50_pad_before_t_in_0_ptr_const_ptr, conv2d_50_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_50_pad_before_v_pad_constant_value_const_s8), conv2d_50_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_50_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1152), (ai_i32)(0), (ai_i32)(3072), (ai_i32)(0), (ai_i32)(384));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(50, 1, {(stai_ptr) conv2d_50_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_50_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_50 */
  {
      const ai_i8* conv2d_50_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 10304);
    const ai_i8* conv2d_50_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 98520);
    const ai_i32* conv2d_50_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 100248);
    ai_i8* conv2d_50_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 7108);
    ai_i16* conv2d_50_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(50, 1, {(stai_ptr) conv2d_50_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_50_t_in_0_ptr_const_s8, conv2d_50_t_in_0_shape_w_const_u16, conv2d_50_t_in_0_shape_h_const_u16, conv2d_50_t_in_0_shape_ch_const_u16, conv2d_50_t_weight_0_ptr_const_s8, conv2d_50_l_stride_1_const_u16, conv2d_50_l_stride_0_const_u16, conv2d_50_t_weight_1_ptr_const_s32, conv2d_50_t_in_0_fmt_zero_const_s8, conv2d_50_t_out_0_fmt_zero_const_s8, conv2d_50_t_in_0_fmt_scale_const_f32, conv2d_50_t_out_0_fmt_scale_const_f32, conv2d_50_t_weight_0_fmt_scale_const_f32, conv2d_50_t_out_0_ptr_s8, conv2d_50_t_out_0_shape_w_const_u16, conv2d_50_t_out_0_shape_h_const_u16, 0, 7105, conv2d_50_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(50, 1, {(stai_ptr) conv2d_50_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_50 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_51 */
  {
      const ai_i8* conv2d_51_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 7108);
    const ai_i8* conv2d_51_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 101016);
    const ai_i32* conv2d_51_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 111768);
    ai_i8* conv2d_51_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1328);
    ai_i16* conv2d_51_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(51, 1, {(stai_ptr) conv2d_51_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_51_t_in_0_ptr_const_s8, conv2d_51_t_in_0_shape_w_const_u16, conv2d_51_t_in_0_shape_h_const_u16, conv2d_51_l_stride_1_const_u16, conv2d_51_l_stride_0_const_u16, conv2d_51_t_in_0_shape_ch_const_u16, conv2d_51_t_weight_0_ptr_const_s8, conv2d_51_t_out_0_shape_ch_const_u16, conv2d_51_t_weight_1_ptr_const_s32, conv2d_51_t_in_0_fmt_zero_const_s8, conv2d_51_t_out_0_fmt_zero_const_s8, conv2d_51_t_in_0_fmt_scale_const_f32, conv2d_51_t_out_0_fmt_scale_const_f32, conv2d_51_t_weight_0_fmt_scale_const_f32, conv2d_51_l_out_ch_format_const_layer_format_type, conv2d_51_t_out_0_ptr_s8, 1, 1328, conv2d_51_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(51, 1, {(stai_ptr) conv2d_51_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_51 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_52 */
  {
      const ai_i8* conv2d_52_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1328);
    const ai_i8* conv2d_52_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 111992);
    const ai_i32* conv2d_52_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 130808);
    ai_i8* conv2d_52_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5416);
    ai_i16* conv2d_52_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1832);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(52, 1, {(stai_ptr) conv2d_52_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_52_t_in_0_ptr_const_s8, conv2d_52_t_in_0_shape_w_const_u16, conv2d_52_t_in_0_shape_h_const_u16, conv2d_52_l_stride_1_const_u16, conv2d_52_l_stride_0_const_u16, conv2d_52_t_in_0_shape_ch_const_u16, conv2d_52_t_weight_0_ptr_const_s8, conv2d_52_t_out_0_shape_ch_const_u16, conv2d_52_t_weight_1_ptr_const_s32, conv2d_52_t_in_0_fmt_zero_const_s8, conv2d_52_t_out_0_fmt_zero_const_s8, conv2d_52_t_in_0_fmt_scale_const_f32, conv2d_52_t_out_0_fmt_scale_const_f32, conv2d_52_t_weight_0_fmt_scale_const_f32, conv2d_52_l_out_ch_format_const_layer_format_type, conv2d_52_t_out_0_ptr_s8, 1, 3584, conv2d_52_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(52, 1, {(stai_ptr) conv2d_52_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_52 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_53_pad_before */
  {
      const ai_ptr conv2d_53_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 5416);
    ai_ptr conv2d_53_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 8440);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(53, 1, {(stai_ptr) conv2d_53_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_53_pad_before_t_in_0_ptr_const_ptr, conv2d_53_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_53_pad_before_v_pad_constant_value_const_s8), conv2d_53_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_53_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1008), (ai_i32)(1680), (ai_i32)(1680), (ai_i32)(336), (ai_i32)(336));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(53, 1, {(stai_ptr) conv2d_53_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_53_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_53 */
  {
      const ai_i8* conv2d_53_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8440);
    const ai_i8* conv2d_53_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 132152);
    const ai_i32* conv2d_53_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 135176);
    ai_i8* conv2d_53_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1832);
    ai_i16* conv2d_53_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 16840);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(53, 1, {(stai_ptr) conv2d_53_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_53_t_in_0_ptr_const_s8, conv2d_53_t_in_0_shape_w_const_u16, conv2d_53_t_in_0_shape_h_const_u16, conv2d_53_t_in_0_shape_ch_const_u16, conv2d_53_t_weight_0_ptr_const_s8, conv2d_53_l_stride_1_const_u16, conv2d_53_l_stride_0_const_u16, conv2d_53_t_weight_1_ptr_const_s32, conv2d_53_t_in_0_fmt_zero_const_s8, conv2d_53_t_out_0_fmt_zero_const_s8, conv2d_53_t_in_0_fmt_scale_const_f32, conv2d_53_t_out_0_fmt_scale_const_f32, conv2d_53_t_weight_0_fmt_scale_const_f32, conv2d_53_t_out_0_ptr_s8, conv2d_53_t_out_0_shape_w_const_u16, conv2d_53_t_out_0_shape_h_const_u16, 0, 12433, conv2d_53_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(53, 1, {(stai_ptr) conv2d_53_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_53 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_54 */
  {
      const ai_i8* conv2d_54_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1832);
    const ai_i8* conv2d_54_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 136520);
    const ai_i32* conv2d_54_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 155336);
    ai_i8* conv2d_54_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    ai_i16* conv2d_54_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 4856);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(54, 1, {(stai_ptr) conv2d_54_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_54_t_in_0_ptr_const_s8, conv2d_54_t_in_0_shape_w_const_u16, conv2d_54_t_in_0_shape_h_const_u16, conv2d_54_l_stride_1_const_u16, conv2d_54_l_stride_0_const_u16, conv2d_54_t_in_0_shape_ch_const_u16, conv2d_54_t_weight_0_ptr_const_s8, conv2d_54_t_out_0_shape_ch_const_u16, conv2d_54_t_weight_1_ptr_const_s32, conv2d_54_t_in_0_fmt_zero_const_s8, conv2d_54_t_out_0_fmt_zero_const_s8, conv2d_54_t_in_0_fmt_scale_const_f32, conv2d_54_t_out_0_fmt_scale_const_f32, conv2d_54_t_weight_0_fmt_scale_const_f32, conv2d_54_l_out_ch_format_const_layer_format_type, conv2d_54_t_out_0_ptr_s8, 1, 1904, conv2d_54_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(54, 1, {(stai_ptr) conv2d_54_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_54 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_55 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_55(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_55 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_56 */
  {
      const ai_i8* conv2d_56_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 504);
    const ai_i8* conv2d_56_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 155560);
    const ai_i32* conv2d_56_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 174376);
    ai_i8* conv2d_56_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 4592);
    ai_i16* conv2d_56_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1008);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(56, 1, {(stai_ptr) conv2d_56_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_56_t_in_0_ptr_const_s8, conv2d_56_t_in_0_shape_w_const_u16, conv2d_56_t_in_0_shape_h_const_u16, conv2d_56_l_stride_1_const_u16, conv2d_56_l_stride_0_const_u16, conv2d_56_t_in_0_shape_ch_const_u16, conv2d_56_t_weight_0_ptr_const_s8, conv2d_56_t_out_0_shape_ch_const_u16, conv2d_56_t_weight_1_ptr_const_s32, conv2d_56_t_in_0_fmt_zero_const_s8, conv2d_56_t_out_0_fmt_zero_const_s8, conv2d_56_t_in_0_fmt_scale_const_f32, conv2d_56_t_out_0_fmt_scale_const_f32, conv2d_56_t_weight_0_fmt_scale_const_f32, conv2d_56_l_out_ch_format_const_layer_format_type, conv2d_56_t_out_0_ptr_s8, 1, 3584, conv2d_56_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(56, 1, {(stai_ptr) conv2d_56_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_56 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_57_pad_before */
  {
      const ai_ptr conv2d_57_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 4592);
    ai_ptr conv2d_57_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 7616);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(57, 1, {(stai_ptr) conv2d_57_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_57_pad_before_t_in_0_ptr_const_ptr, conv2d_57_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_57_pad_before_v_pad_constant_value_const_s8), conv2d_57_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_57_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1008), (ai_i32)(1680), (ai_i32)(1680), (ai_i32)(336), (ai_i32)(336));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(57, 1, {(stai_ptr) conv2d_57_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_57_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_57 */
  {
      const ai_i8* conv2d_57_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 7616);
    const ai_i8* conv2d_57_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 175720);
    const ai_i32* conv2d_57_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 178744);
    ai_i8* conv2d_57_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1008);
    ai_i16* conv2d_57_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 16016);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(57, 1, {(stai_ptr) conv2d_57_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_57_t_in_0_ptr_const_s8, conv2d_57_t_in_0_shape_w_const_u16, conv2d_57_t_in_0_shape_h_const_u16, conv2d_57_t_in_0_shape_ch_const_u16, conv2d_57_t_weight_0_ptr_const_s8, conv2d_57_l_stride_1_const_u16, conv2d_57_l_stride_0_const_u16, conv2d_57_t_weight_1_ptr_const_s32, conv2d_57_t_in_0_fmt_zero_const_s8, conv2d_57_t_out_0_fmt_zero_const_s8, conv2d_57_t_in_0_fmt_scale_const_f32, conv2d_57_t_out_0_fmt_scale_const_f32, conv2d_57_t_weight_0_fmt_scale_const_f32, conv2d_57_t_out_0_ptr_s8, conv2d_57_t_out_0_shape_w_const_u16, conv2d_57_t_out_0_shape_h_const_u16, 0, 12433, conv2d_57_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(57, 1, {(stai_ptr) conv2d_57_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_57 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_58 */
  {
      const ai_i8* conv2d_58_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1008);
    const ai_i8* conv2d_58_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 180088);
    const ai_i32* conv2d_58_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 198904);
    ai_i8* conv2d_58_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    ai_i16* conv2d_58_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 4032);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(58, 1, {(stai_ptr) conv2d_58_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_58_t_in_0_ptr_const_s8, conv2d_58_t_in_0_shape_w_const_u16, conv2d_58_t_in_0_shape_h_const_u16, conv2d_58_l_stride_1_const_u16, conv2d_58_l_stride_0_const_u16, conv2d_58_t_in_0_shape_ch_const_u16, conv2d_58_t_weight_0_ptr_const_s8, conv2d_58_t_out_0_shape_ch_const_u16, conv2d_58_t_weight_1_ptr_const_s32, conv2d_58_t_in_0_fmt_zero_const_s8, conv2d_58_t_out_0_fmt_zero_const_s8, conv2d_58_t_in_0_fmt_scale_const_f32, conv2d_58_t_out_0_fmt_scale_const_f32, conv2d_58_t_weight_0_fmt_scale_const_f32, conv2d_58_l_out_ch_format_const_layer_format_type, conv2d_58_t_out_0_ptr_s8, 1, 1904, conv2d_58_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(58, 1, {(stai_ptr) conv2d_58_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_58 */
  /* LITE_KERNEL_SECTION BEGIN eltwise_59 */
  {
    
  forward_lite_eltwise_integer_INT8_eltwise_59(net_ctx);
  }
  /* LITE_KERNEL_SECTION END eltwise_59 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_60 */
  {
      const ai_i8* conv2d_60_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1008);
    const ai_i8* conv2d_60_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 199128);
    const ai_i32* conv2d_60_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 217944);
    ai_i8* conv2d_60_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5096);
    ai_i16* conv2d_60_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1512);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(60, 1, {(stai_ptr) conv2d_60_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_60_t_in_0_ptr_const_s8, conv2d_60_t_in_0_shape_w_const_u16, conv2d_60_t_in_0_shape_h_const_u16, conv2d_60_l_stride_1_const_u16, conv2d_60_l_stride_0_const_u16, conv2d_60_t_in_0_shape_ch_const_u16, conv2d_60_t_weight_0_ptr_const_s8, conv2d_60_t_out_0_shape_ch_const_u16, conv2d_60_t_weight_1_ptr_const_s32, conv2d_60_t_in_0_fmt_zero_const_s8, conv2d_60_t_out_0_fmt_zero_const_s8, conv2d_60_t_in_0_fmt_scale_const_f32, conv2d_60_t_out_0_fmt_scale_const_f32, conv2d_60_t_weight_0_fmt_scale_const_f32, conv2d_60_l_out_ch_format_const_layer_format_type, conv2d_60_t_out_0_ptr_s8, 1, 3584, conv2d_60_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(60, 1, {(stai_ptr) conv2d_60_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_60 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_61_pad_before */
  {
      const ai_ptr conv2d_61_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 5096);
    ai_ptr conv2d_61_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 8120);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(61, 1, {(stai_ptr) conv2d_61_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_61_pad_before_t_in_0_ptr_const_ptr, conv2d_61_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_61_pad_before_v_pad_constant_value_const_s8), conv2d_61_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_61_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(1008), (ai_i32)(1680), (ai_i32)(1680), (ai_i32)(336), (ai_i32)(336));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(61, 1, {(stai_ptr) conv2d_61_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_61_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_61 */
  {
      const ai_i8* conv2d_61_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8120);
    const ai_i8* conv2d_61_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 219288);
    const ai_i32* conv2d_61_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 222312);
    ai_i8* conv2d_61_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    ai_i16* conv2d_61_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 16520);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(61, 1, {(stai_ptr) conv2d_61_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_61_t_in_0_ptr_const_s8, conv2d_61_t_in_0_shape_w_const_u16, conv2d_61_t_in_0_shape_h_const_u16, conv2d_61_t_in_0_shape_ch_const_u16, conv2d_61_t_weight_0_ptr_const_s8, conv2d_61_l_stride_1_const_u16, conv2d_61_l_stride_0_const_u16, conv2d_61_t_weight_1_ptr_const_s32, conv2d_61_t_in_0_fmt_zero_const_s8, conv2d_61_t_out_0_fmt_zero_const_s8, conv2d_61_t_in_0_fmt_scale_const_f32, conv2d_61_t_out_0_fmt_scale_const_f32, conv2d_61_t_weight_0_fmt_scale_const_f32, conv2d_61_t_out_0_ptr_s8, conv2d_61_t_out_0_shape_w_const_u16, conv2d_61_t_out_0_shape_h_const_u16, 0, 12433, conv2d_61_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(61, 1, {(stai_ptr) conv2d_61_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_61 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_62 */
  {
      const ai_i8* conv2d_62_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* conv2d_62_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 223656);
    const ai_i32* conv2d_62_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 261288);
    ai_i8* conv2d_62_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 5488);
    ai_i16* conv2d_62_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 3024);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(62, 1, {(stai_ptr) conv2d_62_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_62_t_in_0_ptr_const_s8, conv2d_62_t_in_0_shape_w_const_u16, conv2d_62_t_in_0_shape_h_const_u16, conv2d_62_l_stride_1_const_u16, conv2d_62_l_stride_0_const_u16, conv2d_62_t_in_0_shape_ch_const_u16, conv2d_62_t_weight_0_ptr_const_s8, conv2d_62_t_out_0_shape_ch_const_u16, conv2d_62_t_weight_1_ptr_const_s32, conv2d_62_t_in_0_fmt_zero_const_s8, conv2d_62_t_out_0_fmt_zero_const_s8, conv2d_62_t_in_0_fmt_scale_const_f32, conv2d_62_t_out_0_fmt_scale_const_f32, conv2d_62_t_weight_0_fmt_scale_const_f32, conv2d_62_l_out_ch_format_const_layer_format_type, conv2d_62_t_out_0_ptr_s8, 1, 2464, conv2d_62_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(62, 1, {(stai_ptr) conv2d_62_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_62 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_63 */
  {
      const ai_i8* conv2d_63_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 5488);
    const ai_i8* conv2d_63_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 261736);
    const ai_i32* conv2d_63_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 405096);
    ai_i8* conv2d_63_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 19744);
    ai_i16* conv2d_63_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 6496);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(63, 1, {(stai_ptr) conv2d_63_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_63_t_in_0_ptr_const_s8, conv2d_63_t_in_0_shape_w_const_u16, conv2d_63_t_in_0_shape_h_const_u16, conv2d_63_l_stride_1_const_u16, conv2d_63_l_stride_0_const_u16, conv2d_63_t_in_0_shape_ch_const_u16, conv2d_63_t_weight_0_ptr_const_s8, conv2d_63_t_out_0_shape_ch_const_u16, conv2d_63_t_weight_1_ptr_const_s32, conv2d_63_t_in_0_fmt_zero_const_s8, conv2d_63_t_out_0_fmt_zero_const_s8, conv2d_63_t_in_0_fmt_scale_const_f32, conv2d_63_t_out_0_fmt_scale_const_f32, conv2d_63_t_weight_0_fmt_scale_const_f32, conv2d_63_l_out_ch_format_const_layer_format_type, conv2d_63_t_out_0_ptr_s8, 1, 13248, conv2d_63_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(63, 1, {(stai_ptr) conv2d_63_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_63 */
  /* LITE_KERNEL_SECTION BEGIN pool_64 */
  {
    
  forward_lite_ap_integer_INT8_pool_64(net_ctx);
  }
  /* LITE_KERNEL_SECTION END pool_64 */
  /* LITE_KERNEL_SECTION BEGIN gemm_65 */
  {
    
  forward_lite_dense_integer_SSSA_ch_gemm_65(net_ctx);
  }
  /* LITE_KERNEL_SECTION END gemm_65 */
  /* LITE_KERNEL_SECTION BEGIN nl_66 */
  {
      ai_i8* nl_66_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_outputs[0] + 0);
    const ai_i8* nl_66_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 3872);
    ai_i32* nl_66_t_scratch_0_ptr_s32 = (ai_i32*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(66, 1, {(stai_ptr) nl_66_t_in_0_ptr_const_s8});
    
  forward_lite_nl_softmax_is8os8(nl_66_t_out_0_ptr_s8, nl_66_t_in_0_ptr_const_s8, nl_66_t_in_0_shape_ch_prod_const_u32, 1, 3, 1328911232, 23, -248, nl_66_t_scratch_0_ptr_s32);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(66, 1, {(stai_ptr) nl_66_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END nl_66 */
  return net_ctx->_return_code;
}

/*****************************************************************************/
/*  Getters APIs Section  */
STAI_API_ENTRY
stai_size stai_network_get_context_size()
{
  return (stai_size)STAI_NETWORK_CONTEXT_SIZE;
}

#if defined(HAVE_NETWORK_INFO)
STAI_API_ENTRY
stai_return_code stai_network_get_info(
  stai_network* network,
  stai_network_info* info)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, info==NULL, STAI_ERROR_NETWORK_INVALID_INFO, net_ctx->_return_code)

  // Copy of network info struct
  *info = g_network_info;

  return STAI_SUCCESS;
}
#endif


STAI_API_ENTRY
stai_return_code stai_network_get_activations(
  stai_network* network, stai_ptr* activations, stai_size* n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, !n_activations, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_activations = STAI_NETWORK_ACTIVATIONS_NUM;
for (stai_size idx=0; activations && (idx<STAI_NETWORK_ACTIVATIONS_NUM); idx++) {
    // get address of the activations buffers
    activations[idx] = net_ctx->_activations[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_weights(
  stai_network* network, stai_ptr* weights, stai_size* n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_weights, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_weights = STAI_NETWORK_WEIGHTS_NUM;
for (stai_size idx=0; weights && (idx<STAI_NETWORK_WEIGHTS_NUM); idx++) {
    // get address of the weights buffers
    weights[idx] = net_ctx->_weights[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_inputs(
  stai_network* network, stai_ptr* inputs, stai_size* n_inputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_inputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_inputs = STAI_NETWORK_IN_NUM;
  for (stai_size idx=0; inputs && (idx<STAI_NETWORK_IN_NUM); idx++) {
    inputs[idx] = net_ctx->_inputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_outputs(
  stai_network* network, stai_ptr* outputs, stai_size* n_outputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_outputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_outputs = STAI_NETWORK_OUT_NUM;
  for (stai_size idx=0; outputs && (idx<STAI_NETWORK_OUT_NUM); idx++) {
    outputs[idx] = net_ctx->_outputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_error(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /* return 1st generated error or STAI_SUCCESS if no errors so far */
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_states(
  stai_network* network, stai_ptr* states, stai_size* n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_states, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  /* get the number of internals states (supporting multi-heap also for internal states) */
  *n_states = STAI_NETWORK_STATES_NUM;

  STAI_UNUSED(states)
return net_ctx->_return_code;
}


/*****************************************************************************/
/*  Setters APIs Section  */

STAI_API_ENTRY
stai_return_code stai_network_set_activations(
  stai_network* network,
  const stai_ptr* activations,
  const stai_size n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _activations_alignment[] = STAI_NETWORK_ACTIVATIONS_ALIGNMENTS;
  STAI_PRINT("  [stai_network_set_activations] network(%p) activations[%d]: %p\n\n", net_ctx, n_activations, activations)
  _STAI_SET_ERROR(net_ctx, !activations,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_activations!=STAI_NETWORK_ACTIVATIONS_NUM,
                  STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_NUM, net_ctx->_return_code)

  for (stai_size idx=0; activations && idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    STAI_PRINT("  activation[%d]: %p\n", idx, activations[idx])
    _STAI_SET_ERROR(net_ctx, activations[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)activations[idx]) & (_activations_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_activations[idx] = activations[idx];
  }
  net_ctx->_inputs[0] = activations[0] + 14292;

  net_ctx->_outputs[0] = activations[0] + 992;
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_weights(
  stai_network* network,
  const stai_ptr* weights,
  const stai_size n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _weights_alignment[] = STAI_NETWORK_WEIGHTS_ALIGNMENTS;
  _STAI_SET_ERROR(net_ctx, !weights,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_weights!=STAI_NETWORK_WEIGHTS_NUM,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_NUM, net_ctx->_return_code)
  for (stai_size idx=0; weights && idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    STAI_PRINT("  weight[%d]: %p\n", idx, weights[idx])
    _STAI_SET_ERROR(net_ctx, weights[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)weights[idx]) & (_weights_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_weights[idx] = weights[idx];
  }_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_inputs(
  stai_network* network,
  const stai_ptr* inputs,
  const stai_size n_inputs)
{
  const uintptr_t _inputs_alignment[] = STAI_NETWORK_IN_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !inputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_inputs!=STAI_NETWORK_IN_NUM,
                  STAI_ERROR_NETWORK_INVALID_IN_NUM, net_ctx->_return_code)

  for (stai_size idx=0; inputs && idx<STAI_NETWORK_IN_NUM; idx++) {
    STAI_PRINT("  input[%d]: %p\n", idx, inputs[idx])
    _STAI_SET_ERROR(net_ctx, inputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)inputs[idx]) & (_inputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_inputs[idx] = inputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_outputs(
  stai_network* network,
  const stai_ptr* outputs,
  const stai_size n_outputs)
{
  const uintptr_t _outputs_alignment[] = STAI_NETWORK_OUT_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !outputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_outputs!=STAI_NETWORK_OUT_NUM,
                  STAI_ERROR_NETWORK_INVALID_OUT_NUM, net_ctx->_return_code)

  for (stai_size idx=0; outputs && idx<n_outputs; idx++) {
    STAI_PRINT("  output[%d]: %p\n", idx, outputs[idx])
    _STAI_SET_ERROR(net_ctx, outputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)outputs[idx]) & (_outputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_outputs[idx] = outputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_states(
  stai_network* network,
  const stai_ptr* states,
  const stai_size n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  STAI_UNUSED(states)
  STAI_UNUSED(n_states)
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}

STAI_API_ENTRY
stai_return_code stai_network_set_callback(
  stai_network* network, const stai_event_cb cb, void* cb_cookie)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  STAI_PRINT("  set_callback %p cb %p cookie %p\n", net_ctx, cb, cb_cookie)
  // _STAI_SET_ERROR(net_ctx, cb==NULL, STAI_ERROR_NETWORK_INVALID_CALLBACK, net_ctx->_return_code)
  net_ctx->_callback = cb;
  net_ctx->_callback_cookie = cb_cookie;
  return net_ctx->_return_code;
}

#undef _STAI_SET_ERROR
#undef _STAI_CONTEXT_ALIGNMENT
#undef _STAI_CONTEXT_ACQUIRE
#undef _STAI_NETWORK_EVENT_NODE_START_CB
#undef _STAI_NETWORK_EVENT_NODE_STOP_CB
#undef _STAI_NETWORK_MODEL_SIGNATURE
#undef _STAI_NETWORK_DATETIME
#undef _STAI_NETWORK_COMPILE_DATETIME

