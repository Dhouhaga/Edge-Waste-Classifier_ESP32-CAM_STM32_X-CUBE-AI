/**
  ******************************************************************************
  * @file    network.h
  * @date    2026-10-08T08:42:40+0000
  * @brief   ST.AI Tool Automatic Code Generator for Embedded NN computing
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
#ifndef STAI_NETWORK_DETAILS_H
#define STAI_NETWORK_DETAILS_H

#include "stai.h"
#include "layers.h"

const stai_network_details g_network_details = {
  .tensors = (const stai_tensor[85]) {
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 96, 96, 3}}, .scale = {1, (const float[1]){1.0}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "serving_default_keras_tensor_16180_output" },
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 96, 96, 3}}, .scale = {1, (const float[1]){0.0117647061124444}}, .zeropoint = {1, (const int16_t[1]){-43}}, .name = "eltwise_0_output" },
   { .size_bytes = 27649, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 96, 96, 3}}, .scale = {1, (const float[1]){0.007843137718737125}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "eltwise_1_output" },
   { .size_bytes = 36864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 48, 48, 16}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_2_output" },
   { .size_bytes = 40000, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 50, 50, 16}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_3_pad_before_output" },
   { .size_bytes = 36864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 48, 48, 16}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_3_output" },
   { .size_bytes = 18432, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 48, 48, 8}}, .scale = {1, (const float[1]){0.3810703456401825}}, .zeropoint = {1, (const int16_t[1]){3}}, .name = "conv2d_4_output" },
   { .size_bytes = 110592, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 48, 48, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_5_output" },
   { .size_bytes = 120000, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 50, 50, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_6_pad_before_output" },
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_6_output" },
   { .size_bytes = 4608, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 8}}, .scale = {1, (const float[1]){0.3196049630641937}}, .zeropoint = {1, (const int16_t[1]){5}}, .name = "conv2d_7_output" },
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_8_output" },
   { .size_bytes = 32448, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 26, 26, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_9_pad_before_output" },
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_9_output" },
   { .size_bytes = 4608, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 8}}, .scale = {1, (const float[1]){0.5001610517501831}}, .zeropoint = {1, (const int16_t[1]){-4}}, .name = "conv2d_10_output" },
   { .size_bytes = 4608, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 8}}, .scale = {1, (const float[1]){0.48252204060554504}}, .zeropoint = {1, (const int16_t[1]){1}}, .name = "eltwise_11_output" },
   { .size_bytes = 27648, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 24, 24, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_12_output" },
   { .size_bytes = 32448, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 26, 26, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_13_pad_before_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 48}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_13_output" },
   { .size_bytes = 2304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 16}}, .scale = {1, (const float[1]){0.2943727970123291}}, .zeropoint = {1, (const int16_t[1]){20}}, .name = "conv2d_14_output" },
   { .size_bytes = 13824, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_15_output" },
   { .size_bytes = 18816, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 14, 14, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_16_pad_before_output" },
   { .size_bytes = 13824, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_16_output" },
   { .size_bytes = 2304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 16}}, .scale = {1, (const float[1]){0.27556443214416504}}, .zeropoint = {1, (const int16_t[1]){10}}, .name = "conv2d_17_output" },
   { .size_bytes = 2304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 16}}, .scale = {1, (const float[1]){0.2676216959953308}}, .zeropoint = {1, (const int16_t[1]){14}}, .name = "eltwise_18_output" },
   { .size_bytes = 13824, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_19_output" },
   { .size_bytes = 18816, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 14, 14, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_20_pad_before_output" },
   { .size_bytes = 13824, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_20_output" },
   { .size_bytes = 2304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 16}}, .scale = {1, (const float[1]){0.3419592082500458}}, .zeropoint = {1, (const int16_t[1]){8}}, .name = "conv2d_21_output" },
   { .size_bytes = 2304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 16}}, .scale = {1, (const float[1]){0.3419592082500458}}, .zeropoint = {1, (const int16_t[1]){8}}, .name = "eltwise_22_output" },
   { .size_bytes = 13824, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 12, 12, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_23_output" },
   { .size_bytes = 18816, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 14, 14, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_24_pad_before_output" },
   { .size_bytes = 3456, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 96}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_24_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.25301802158355713}}, .zeropoint = {1, (const int16_t[1]){-12}}, .name = "conv2d_25_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_26_output" },
   { .size_bytes = 9216, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_27_pad_before_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_27_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.26992267370224}}, .zeropoint = {1, (const int16_t[1]){-7}}, .name = "conv2d_28_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.26992267370224}}, .zeropoint = {1, (const int16_t[1]){-7}}, .name = "eltwise_29_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_30_output" },
   { .size_bytes = 9216, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_31_pad_before_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_31_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.27030789852142334}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "conv2d_32_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.27030789852142334}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "eltwise_33_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_34_output" },
   { .size_bytes = 9216, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_35_pad_before_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_35_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.3205251395702362}}, .zeropoint = {1, (const int16_t[1]){19}}, .name = "conv2d_36_output" },
   { .size_bytes = 864, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 24}}, .scale = {1, (const float[1]){0.3205251395702362}}, .zeropoint = {1, (const int16_t[1]){19}}, .name = "eltwise_37_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_38_output" },
   { .size_bytes = 9216, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_39_pad_before_output" },
   { .size_bytes = 5184, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 144}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_39_output" },
   { .size_bytes = 1152, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 32}}, .scale = {1, (const float[1]){0.19619816541671753}}, .zeropoint = {1, (const int16_t[1]){7}}, .name = "conv2d_40_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_41_output" },
   { .size_bytes = 12288, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_42_pad_before_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_42_output" },
   { .size_bytes = 1152, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 32}}, .scale = {1, (const float[1]){0.24855132400989532}}, .zeropoint = {1, (const int16_t[1]){-13}}, .name = "conv2d_43_output" },
   { .size_bytes = 1152, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 32}}, .scale = {1, (const float[1]){0.24855132400989532}}, .zeropoint = {1, (const int16_t[1]){-13}}, .name = "eltwise_44_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_45_output" },
   { .size_bytes = 12288, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_46_pad_before_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_46_output" },
   { .size_bytes = 1152, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 32}}, .scale = {1, (const float[1]){0.2924808859825134}}, .zeropoint = {1, (const int16_t[1]){3}}, .name = "conv2d_47_output" },
   { .size_bytes = 1152, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 32}}, .scale = {1, (const float[1]){0.2924808859825134}}, .zeropoint = {1, (const int16_t[1]){3}}, .name = "eltwise_48_output" },
   { .size_bytes = 6912, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 6, 6, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_49_output" },
   { .size_bytes = 12288, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 8, 8, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_50_pad_before_output" },
   { .size_bytes = 1728, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 192}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_50_output" },
   { .size_bytes = 504, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 56}}, .scale = {1, (const float[1]){0.16510659456253052}}, .zeropoint = {1, (const int16_t[1]){16}}, .name = "conv2d_51_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_52_output" },
   { .size_bytes = 8400, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 5, 5, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_53_pad_before_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_53_output" },
   { .size_bytes = 504, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 56}}, .scale = {1, (const float[1]){0.1923304796218872}}, .zeropoint = {1, (const int16_t[1]){-10}}, .name = "conv2d_54_output" },
   { .size_bytes = 504, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 56}}, .scale = {1, (const float[1]){0.1923304796218872}}, .zeropoint = {1, (const int16_t[1]){-10}}, .name = "eltwise_55_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_56_output" },
   { .size_bytes = 8400, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 5, 5, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_57_pad_before_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_57_output" },
   { .size_bytes = 504, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 56}}, .scale = {1, (const float[1]){0.22229193150997162}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "conv2d_58_output" },
   { .size_bytes = 504, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 56}}, .scale = {1, (const float[1]){0.22229193150997162}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "eltwise_59_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_60_output" },
   { .size_bytes = 8400, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 5, 5, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_61_pad_before_output" },
   { .size_bytes = 3024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 336}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_61_output" },
   { .size_bytes = 1008, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 112}}, .scale = {1, (const float[1]){0.13179849088191986}}, .zeropoint = {1, (const int16_t[1]){-11}}, .name = "conv2d_62_output" },
   { .size_bytes = 11520, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 3, 1280}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_63_output" },
   { .size_bytes = 1280, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 1, 1280}}, .scale = {1, (const float[1]){0.0235294122248888}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "pool_64_output" },
   { .size_bytes = 3, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 3}}, .scale = {1, (const float[1]){0.07735281437635422}}, .zeropoint = {1, (const int16_t[1]){-19}}, .name = "gemm_65_output" },
   { .size_bytes = 3, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 3}}, .scale = {1, (const float[1]){0.00390625}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "nl_66_output" }
  },
  .nodes = (const stai_node_details[84]){
    {.id = 0, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){0}}, .output_tensors = {1, (const int32_t[1]){1}} }, /* eltwise_0 */
    {.id = 1, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){1}}, .output_tensors = {1, (const int32_t[1]){2}} }, /* eltwise_1 */
    {.id = 2, .type = AI_LAYER_OPTIMIZED_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){2}}, .output_tensors = {1, (const int32_t[1]){3}} }, /* conv2d_2 */
    {.id = 3, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){3}}, .output_tensors = {1, (const int32_t[1]){4}} }, /* conv2d_3_pad_before */
    {.id = 3, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){4}}, .output_tensors = {1, (const int32_t[1]){5}} }, /* conv2d_3 */
    {.id = 4, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){5}}, .output_tensors = {1, (const int32_t[1]){6}} }, /* conv2d_4 */
    {.id = 5, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){6}}, .output_tensors = {1, (const int32_t[1]){7}} }, /* conv2d_5 */
    {.id = 6, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){7}}, .output_tensors = {1, (const int32_t[1]){8}} }, /* conv2d_6_pad_before */
    {.id = 6, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){8}}, .output_tensors = {1, (const int32_t[1]){9}} }, /* conv2d_6 */
    {.id = 7, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){9}}, .output_tensors = {1, (const int32_t[1]){10}} }, /* conv2d_7 */
    {.id = 8, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){10}}, .output_tensors = {1, (const int32_t[1]){11}} }, /* conv2d_8 */
    {.id = 9, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){11}}, .output_tensors = {1, (const int32_t[1]){12}} }, /* conv2d_9_pad_before */
    {.id = 9, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){12}}, .output_tensors = {1, (const int32_t[1]){13}} }, /* conv2d_9 */
    {.id = 10, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){13}}, .output_tensors = {1, (const int32_t[1]){14}} }, /* conv2d_10 */
    {.id = 11, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){10, 14}}, .output_tensors = {1, (const int32_t[1]){15}} }, /* eltwise_11 */
    {.id = 12, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){15}}, .output_tensors = {1, (const int32_t[1]){16}} }, /* conv2d_12 */
    {.id = 13, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){16}}, .output_tensors = {1, (const int32_t[1]){17}} }, /* conv2d_13_pad_before */
    {.id = 13, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){17}}, .output_tensors = {1, (const int32_t[1]){18}} }, /* conv2d_13 */
    {.id = 14, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){18}}, .output_tensors = {1, (const int32_t[1]){19}} }, /* conv2d_14 */
    {.id = 15, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){19}}, .output_tensors = {1, (const int32_t[1]){20}} }, /* conv2d_15 */
    {.id = 16, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){20}}, .output_tensors = {1, (const int32_t[1]){21}} }, /* conv2d_16_pad_before */
    {.id = 16, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){21}}, .output_tensors = {1, (const int32_t[1]){22}} }, /* conv2d_16 */
    {.id = 17, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){22}}, .output_tensors = {1, (const int32_t[1]){23}} }, /* conv2d_17 */
    {.id = 18, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){19, 23}}, .output_tensors = {1, (const int32_t[1]){24}} }, /* eltwise_18 */
    {.id = 19, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){24}}, .output_tensors = {1, (const int32_t[1]){25}} }, /* conv2d_19 */
    {.id = 20, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){25}}, .output_tensors = {1, (const int32_t[1]){26}} }, /* conv2d_20_pad_before */
    {.id = 20, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){26}}, .output_tensors = {1, (const int32_t[1]){27}} }, /* conv2d_20 */
    {.id = 21, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){27}}, .output_tensors = {1, (const int32_t[1]){28}} }, /* conv2d_21 */
    {.id = 22, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){24, 28}}, .output_tensors = {1, (const int32_t[1]){29}} }, /* eltwise_22 */
    {.id = 23, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){29}}, .output_tensors = {1, (const int32_t[1]){30}} }, /* conv2d_23 */
    {.id = 24, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){30}}, .output_tensors = {1, (const int32_t[1]){31}} }, /* conv2d_24_pad_before */
    {.id = 24, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){31}}, .output_tensors = {1, (const int32_t[1]){32}} }, /* conv2d_24 */
    {.id = 25, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){32}}, .output_tensors = {1, (const int32_t[1]){33}} }, /* conv2d_25 */
    {.id = 26, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){33}}, .output_tensors = {1, (const int32_t[1]){34}} }, /* conv2d_26 */
    {.id = 27, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){34}}, .output_tensors = {1, (const int32_t[1]){35}} }, /* conv2d_27_pad_before */
    {.id = 27, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){35}}, .output_tensors = {1, (const int32_t[1]){36}} }, /* conv2d_27 */
    {.id = 28, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){36}}, .output_tensors = {1, (const int32_t[1]){37}} }, /* conv2d_28 */
    {.id = 29, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){33, 37}}, .output_tensors = {1, (const int32_t[1]){38}} }, /* eltwise_29 */
    {.id = 30, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){38}}, .output_tensors = {1, (const int32_t[1]){39}} }, /* conv2d_30 */
    {.id = 31, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){39}}, .output_tensors = {1, (const int32_t[1]){40}} }, /* conv2d_31_pad_before */
    {.id = 31, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){40}}, .output_tensors = {1, (const int32_t[1]){41}} }, /* conv2d_31 */
    {.id = 32, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){41}}, .output_tensors = {1, (const int32_t[1]){42}} }, /* conv2d_32 */
    {.id = 33, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){38, 42}}, .output_tensors = {1, (const int32_t[1]){43}} }, /* eltwise_33 */
    {.id = 34, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){43}}, .output_tensors = {1, (const int32_t[1]){44}} }, /* conv2d_34 */
    {.id = 35, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){44}}, .output_tensors = {1, (const int32_t[1]){45}} }, /* conv2d_35_pad_before */
    {.id = 35, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){45}}, .output_tensors = {1, (const int32_t[1]){46}} }, /* conv2d_35 */
    {.id = 36, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){46}}, .output_tensors = {1, (const int32_t[1]){47}} }, /* conv2d_36 */
    {.id = 37, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){43, 47}}, .output_tensors = {1, (const int32_t[1]){48}} }, /* eltwise_37 */
    {.id = 38, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){48}}, .output_tensors = {1, (const int32_t[1]){49}} }, /* conv2d_38 */
    {.id = 39, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){49}}, .output_tensors = {1, (const int32_t[1]){50}} }, /* conv2d_39_pad_before */
    {.id = 39, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){50}}, .output_tensors = {1, (const int32_t[1]){51}} }, /* conv2d_39 */
    {.id = 40, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){51}}, .output_tensors = {1, (const int32_t[1]){52}} }, /* conv2d_40 */
    {.id = 41, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){52}}, .output_tensors = {1, (const int32_t[1]){53}} }, /* conv2d_41 */
    {.id = 42, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){53}}, .output_tensors = {1, (const int32_t[1]){54}} }, /* conv2d_42_pad_before */
    {.id = 42, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){54}}, .output_tensors = {1, (const int32_t[1]){55}} }, /* conv2d_42 */
    {.id = 43, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){55}}, .output_tensors = {1, (const int32_t[1]){56}} }, /* conv2d_43 */
    {.id = 44, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){52, 56}}, .output_tensors = {1, (const int32_t[1]){57}} }, /* eltwise_44 */
    {.id = 45, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){57}}, .output_tensors = {1, (const int32_t[1]){58}} }, /* conv2d_45 */
    {.id = 46, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){58}}, .output_tensors = {1, (const int32_t[1]){59}} }, /* conv2d_46_pad_before */
    {.id = 46, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){59}}, .output_tensors = {1, (const int32_t[1]){60}} }, /* conv2d_46 */
    {.id = 47, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){60}}, .output_tensors = {1, (const int32_t[1]){61}} }, /* conv2d_47 */
    {.id = 48, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){57, 61}}, .output_tensors = {1, (const int32_t[1]){62}} }, /* eltwise_48 */
    {.id = 49, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){62}}, .output_tensors = {1, (const int32_t[1]){63}} }, /* conv2d_49 */
    {.id = 50, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){63}}, .output_tensors = {1, (const int32_t[1]){64}} }, /* conv2d_50_pad_before */
    {.id = 50, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){64}}, .output_tensors = {1, (const int32_t[1]){65}} }, /* conv2d_50 */
    {.id = 51, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){65}}, .output_tensors = {1, (const int32_t[1]){66}} }, /* conv2d_51 */
    {.id = 52, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){66}}, .output_tensors = {1, (const int32_t[1]){67}} }, /* conv2d_52 */
    {.id = 53, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){67}}, .output_tensors = {1, (const int32_t[1]){68}} }, /* conv2d_53_pad_before */
    {.id = 53, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){68}}, .output_tensors = {1, (const int32_t[1]){69}} }, /* conv2d_53 */
    {.id = 54, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){69}}, .output_tensors = {1, (const int32_t[1]){70}} }, /* conv2d_54 */
    {.id = 55, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){66, 70}}, .output_tensors = {1, (const int32_t[1]){71}} }, /* eltwise_55 */
    {.id = 56, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){71}}, .output_tensors = {1, (const int32_t[1]){72}} }, /* conv2d_56 */
    {.id = 57, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){72}}, .output_tensors = {1, (const int32_t[1]){73}} }, /* conv2d_57_pad_before */
    {.id = 57, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){73}}, .output_tensors = {1, (const int32_t[1]){74}} }, /* conv2d_57 */
    {.id = 58, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){74}}, .output_tensors = {1, (const int32_t[1]){75}} }, /* conv2d_58 */
    {.id = 59, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){71, 75}}, .output_tensors = {1, (const int32_t[1]){76}} }, /* eltwise_59 */
    {.id = 60, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){76}}, .output_tensors = {1, (const int32_t[1]){77}} }, /* conv2d_60 */
    {.id = 61, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){77}}, .output_tensors = {1, (const int32_t[1]){78}} }, /* conv2d_61_pad_before */
    {.id = 61, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){78}}, .output_tensors = {1, (const int32_t[1]){79}} }, /* conv2d_61 */
    {.id = 62, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){79}}, .output_tensors = {1, (const int32_t[1]){80}} }, /* conv2d_62 */
    {.id = 63, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){80}}, .output_tensors = {1, (const int32_t[1]){81}} }, /* conv2d_63 */
    {.id = 64, .type = AI_LAYER_POOL_TYPE, .input_tensors = {1, (const int32_t[1]){81}}, .output_tensors = {1, (const int32_t[1]){82}} }, /* pool_64 */
    {.id = 65, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){82}}, .output_tensors = {1, (const int32_t[1]){83}} }, /* gemm_65 */
    {.id = 66, .type = AI_LAYER_SM_TYPE, .input_tensors = {1, (const int32_t[1]){83}}, .output_tensors = {1, (const int32_t[1]){84}} } /* nl_66 */
  },
  .n_nodes = 84
};
#endif

