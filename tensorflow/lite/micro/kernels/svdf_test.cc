/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <initializer_list>

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/testing/micro_test.h"
#include "tensorflow/lite/micro/testing/test_utils.h"

namespace tflite {
namespace testing {
namespace {

static float svdf_input[] = {
    0.12609188,  -0.46347019, -0.89598465,
    0.35867718,  0.36897406,  0.73463392,

    0.14278367,  -1.64410412, -0.75222826,
    -0.57290924, 0.12729003,  0.7567004,

    0.49837467,  0.19278903,  0.26584083,
    0.17660543,  0.52949083,  -0.77931279,

    -0.11186574, 0.13164264,  -0.05349274,
    -0.72674477, -0.5683046,  0.55900657,

    -0.68892461, 0.37783599,  0.18263303,
    -0.63690937, 0.44483393,  -0.71817774,

    -0.81299269, -0.86831826, 1.43940818,
    -0.95760226, 1.82078898,  0.71135032,

    -1.45006323, -0.82251364, -1.69082689,
    -1.65087092, -1.89238167, 1.54172635,

    0.03966608,  -0.24936394, -0.77526885,
    2.06740379,  -1.51439476, 1.43768692,

    0.11771342,  -0.23761693, -0.65898693,
    0.31088525,  -1.55601168, -0.87661445,

    -0.89477462, 1.67204106,  -0.53235275,
    -0.6230064,  0.29819036,  1.06939757,
};

static float svdf_golden_output_rank_1[] = {
    0.014899,    -0.0517661,  -0.143725,   -0.00271883,
    -0.03004015, 0.09565311,  0.1587342,   0.00784263,

    0.068281,    -0.162217,   -0.152268,   0.00323521,
    0.01582633,  0.03858774,  -0.03001583, -0.02671271,

    -0.0317821,  -0.0333089,  0.0609602,   0.0333759,
    -0.01432795, 0.05524484,  0.1101355,   -0.02382665,

    -0.00623099, -0.077701,   -0.391193,   -0.0136691,
    -0.02333033, 0.02293761,  0.12338032,  0.04326871,

    0.201551,    -0.164607,   -0.179462,   -0.0592739,
    0.01064911,  -0.17503069, 0.07821996,  -0.00224009,

    0.0886511,   -0.0875401,  -0.269283,   0.0281379,
    -0.02282338, 0.09741908,  0.32973239,  0.12281385,

    -0.201174,   -0.586145,   -0.628624,   -0.0330412,
    0.24780814,  -0.39304617, -0.22473189, 0.02589256,

    -0.0839096,  -0.299329,   0.108746,    0.109808,
    0.10084175,  -0.06416984, 0.28936723,  0.0026358,

    0.419114,    -0.237824,   -0.422627,   0.175115,
    -0.2314795,  -0.18584411, -0.4228974,  -0.12928449,

    0.36726,     -0.522303,   -0.456502,   -0.175475,
    0.17012937,  -0.34447709, 0.38505614,  -0.28158101,
};

static float svdf_golden_output_rank_2[] = {
    -0.09623547, -0.10193135, 0.11083051,  -0.0347917,
    0.1141196,   0.12965347,  -0.12652366, 0.01007236,

    -0.16396809, -0.21247184, 0.11259045,  -0.04156673,
    0.10132131,  -0.06143532, -0.00924693, 0.10084561,

    0.01257364,  0.0506071,   -0.19287863, -0.07162561,
    -0.02033747, 0.22673416,  0.15487903,  0.02525555,

    -0.1411963,  -0.37054959, 0.01774767,  0.05867489,
    0.09607603,  -0.0141301,  -0.08995658, 0.12867066,

    -0.27142537, -0.16955489, 0.18521598,  -0.12528358,
    0.00331409,  0.11167502,  0.02218599,  -0.07309391,

    0.09593632,  -0.28361851, -0.0773851,  0.17199151,
    -0.00075242, 0.33691186,  -0.1536046,  0.16572715,

    -0.27916506, -0.27626723, 0.42615682,  0.3225764,
    -0.37472126, -0.55655634, -0.05013514, 0.289112,

    -0.24418658, 0.07540751,  -0.1940318,  -0.08911639,
    0.00732617,  0.46737891,  0.26449674,  0.24888524,

    -0.17225097, -0.54660404, -0.38795233, 0.08389944,
    0.07736043,  -0.28260678, 0.15666828,  1.14949894,

    -0.57454878, -0.64704704, 0.73235172,  -0.34616736,
    0.21120001,  -0.22927976, 0.02455296,  -0.35906726,
};

void ValidateSVDFGoldens(const int batch_size, const int num_units,
                         const int input_size, const int rank,
                         TfLiteTensor* tensors, const int tensor_count,
                         bool is_hybrid_op, float* golden_input_data,
                         const int golden_input_data_size, float* output_data,
                         float* expected_output, float tolerance = 1e-5f) {
  TfLiteContext context;
  PopulateContext(tensors, tensor_count, &context);

  ::tflite::ops::micro::AllOpsResolver resolver;
  const TfLiteRegistration* registration =
      resolver.FindOp(tflite::BuiltinOperator_SVDF, 1);
  TF_LITE_MICRO_EXPECT_NE(nullptr, registration);

  TfLiteSVDFParams params;
  params.rank = rank;
  params.activation = kTfLiteActNone;

  void* user_data = nullptr;
  if (registration->init) {
    user_data = registration->init(&context, nullptr, 0);
  }

  // Bias is an optional tensor:
  // TODO(kreeger): Use input tensor as variable until scratch tensor allocation
  // has been implemented (b/132070898)
  // int inputs_array_data[] = {5, 0, 1, 2, kTfLiteOptionalTensor, 3};
  int inputs_array_data[] = {6, 0, 1, 2, kTfLiteOptionalTensor, 3, 5};
  TfLiteIntArray* inputs_array = IntArrayFromInts(inputs_array_data);

  int outputs_array_data[] = {1, 4};
  TfLiteIntArray* outputs_array = IntArrayFromInts(outputs_array_data);

  int temporaries_array_data[] = {1, 5};
  TfLiteIntArray* temporaries_array = IntArrayFromInts(temporaries_array_data);

  int hybrid_temporaries_array_data[] = {4, 5, 6, 7, 8};
  TfLiteIntArray* hybrid_temporaries_array =
      IntArrayFromInts(hybrid_temporaries_array_data);

  TfLiteNode node;
  node.inputs = inputs_array;
  node.outputs = outputs_array;
  if (is_hybrid_op) {
    node.temporaries = hybrid_temporaries_array;
  } else {
    node.temporaries = temporaries_array;
  }
  node.user_data = user_data;
  node.builtin_data = reinterpret_cast<void*>(&params);
  node.custom_initial_data = nullptr;
  node.custom_initial_data_size = 0;
  node.delegate = nullptr;
  if (registration->prepare) {
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, registration->prepare(&context, &node));
  }
  TF_LITE_MICRO_EXPECT_NE(nullptr, registration->invoke);

  int input_sequence_size =
      golden_input_data_size / sizeof(float) / (input_size * batch_size);
  for (int i = 0; i < input_sequence_size; ++i) {
    float* input_batch_start = golden_input_data + i * input_size * batch_size;
    float* input_batch_end = input_batch_start + input_size * batch_size;

    PopulateFloatTensor(&tensors[0], input_batch_start, input_batch_end);
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, registration->invoke(&context, &node));

    int output_idx = 0;
    int golden_idx = i * batch_size * num_units;
    for (int j = golden_idx; j < golden_idx + batch_size * num_units; ++j) {
      TF_LITE_MICRO_EXPECT_NEAR(expected_output[j], output_data[output_idx],
                                tolerance);
      output_idx++;
    }
  }

  if (registration->free) {
    registration->free(&context, user_data);
  }
}

void ValidateIntegerSVDFGoldens(const int batch_size, const int num_units,
                                const int input_size, const int rank,
                                TfLiteTensor* tensors, const int tensor_count,
                                int8_t* golden_input_data,
                                const int golden_input_data_size,
                                int8_t* output_data, int8_t* expected_output) {
  TfLiteContext context;
  PopulateContext(tensors, tensor_count, &context);

  ::tflite::ops::micro::AllOpsResolver resolver;
  const TfLiteRegistration* registration =
      resolver.FindOp(tflite::BuiltinOperator_SVDF, 1);
  TF_LITE_MICRO_EXPECT_NE(nullptr, registration);

  TfLiteSVDFParams params;
  params.rank = rank;
  params.activation = kTfLiteActRelu;

  void* user_data = nullptr;
  if (registration->init) {
    user_data = registration->init(&context, nullptr, 0);
  }

  // TODO(b/132070898): Use input tensor as variable until scratch tensor
  // allocation has been implemented. int inputs_array_data[] = {5, 0, 1, 2, 3,
  // 4};
  int inputs_array_data[] = {8, 0, 1, 2, 3, 4, 6, 7, 8};
  TfLiteIntArray* inputs_array = IntArrayFromInts(inputs_array_data);

  int outputs_array_data[] = {1, 5};
  TfLiteIntArray* outputs_array = IntArrayFromInts(outputs_array_data);

  int temporaries_array_data[] = {2, 7, 8};
  TfLiteIntArray* temporaries_array = IntArrayFromInts(temporaries_array_data);

  TfLiteNode node;
  node.inputs = inputs_array;
  node.outputs = outputs_array;
  node.temporaries = temporaries_array;
  node.user_data = user_data;
  node.builtin_data = reinterpret_cast<void*>(&params);
  node.custom_initial_data = nullptr;
  node.custom_initial_data_size = 0;
  node.delegate = nullptr;

  if (registration->prepare) {
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, registration->prepare(&context, &node));
  }
  TF_LITE_MICRO_EXPECT_NE(nullptr, registration->invoke);

  int input_sequence_size =
      golden_input_data_size / sizeof(int8_t) / (input_size * batch_size);
  for (int i = 0; i < input_sequence_size; ++i) {
    int8_t* input_batch_start = golden_input_data + i * input_size * batch_size;
    int8_t* input_batch_end = input_batch_start + input_size * batch_size;
    int8_t* tensor_data = tensors[0].data.int8;
    while (input_batch_start != input_batch_end) {
      *tensor_data++ = *input_batch_start++;
    }

    TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, registration->invoke(&context, &node));

    int output_idx = 0;
    int golden_idx = i * batch_size * num_units;
    for (int j = golden_idx; j < golden_idx + batch_size * num_units; ++j) {
      TF_LITE_MICRO_EXPECT_NEAR(expected_output[j], output_data[output_idx], 0);
      output_idx++;
    }
  }

  if (registration->free) {
    registration->free(&context, user_data);
  }
}

void TestSVDF(const int batch_size, const int num_units, const int input_size,
              const int memory_size, const int rank, float* input_data,
              float* weights_feature_data, float* weights_time_data,
              float* activation_state_data, float* scratch_data,
              float* output_data, float* golden_input_data,
              int golden_input_data_size, float* expected_output,
              float tolerance = 1e-5f) {
  const int num_filters = num_units * rank;

  const int input_dims_arg[] = {2, batch_size, input_size};
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_arg);

  const int weights_feature_dims_args[] = {2, num_filters, input_size};
  TfLiteIntArray* weights_feature_dims =
      IntArrayFromInts(weights_feature_dims_args);

  const int weights_time_dims_args[] = {2, num_filters, memory_size};
  TfLiteIntArray* weights_time_dims = IntArrayFromInts(weights_time_dims_args);

  const int activation_state_dims_args[] = {2, batch_size,
                                            memory_size * num_filters};
  TfLiteIntArray* activation_state_dims =
      IntArrayFromInts(activation_state_dims_args);

  // Scratch output is the same shape as output:
  const int scratch_dims_args[] = {2, batch_size, num_filters};
  TfLiteIntArray* scratch_dims = IntArrayFromInts(scratch_dims_args);

  const int output_dims_args[] = {2, batch_size, num_units};
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_args);

  const int tensor_count = 6;  // 4 inputs, 1 output, 1 scratch
  TfLiteTensor tensors[] = {
      CreateFloatTensor(input_data, input_dims, "input"),
      CreateFloatTensor(weights_feature_data, weights_feature_dims,
                        "weights_feature"),
      CreateFloatTensor(weights_time_data, weights_time_dims, "weights_time"),
      CreateFloatTensor(activation_state_data, activation_state_dims,
                        "activation_state", true /* is_variable */),
      CreateFloatTensor(output_data, output_dims, "output"),
      CreateFloatTensor(scratch_data, scratch_dims, "scratch"),
  };

  ValidateSVDFGoldens(batch_size, num_units, input_size, rank, tensors,
                      tensor_count, false /* is_hybrid */, golden_input_data,
                      golden_input_data_size, output_data, expected_output,
                      tolerance);
}

inline void TestHybridSVDFInt8(
    const int batch_size, const int num_units, const int input_size,
    const int memory_size, const int rank, float* input_data,
    float* weights_feature_data, int8_t* weights_feature_quantized_data,
    float* weights_time_data, int8_t* weights_time_quantized_data,
    float* activation_state_data, float* scratch_data,
    int8_t* scratch_input_quantized, float* scratch_scaling_factors,
    float* scratch_weights_time, float* output_data, float* golden_input_data,
    int golden_input_data_size, float* expected_output,
    float tolerance = 1e-5f) {
  const int num_filters = num_units * rank;

  const int input_dims_arg[] = {2, batch_size, input_size};
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_arg);

  const int weights_feature_dims_args[] = {2, num_filters, input_size};
  TfLiteIntArray* weights_feature_dims =
      IntArrayFromInts(weights_feature_dims_args);

  const int weights_time_dims_args[] = {2, num_filters, memory_size};
  TfLiteIntArray* weights_time_dims = IntArrayFromInts(weights_time_dims_args);

  const int activation_state_dims_args[] = {2, batch_size,
                                            memory_size * num_filters};
  TfLiteIntArray* activation_state_dims =
      IntArrayFromInts(activation_state_dims_args);

  // Scratch output is the same shape as output:
  const int scratch_dims_args[] = {2, batch_size, num_filters};
  TfLiteIntArray* scratch_dims = IntArrayFromInts(scratch_dims_args);

  const int scratch_scaling_factor_dims_args[] = {1, batch_size};
  TfLiteIntArray* scratch_scaling_factors_dims =
      IntArrayFromInts(scratch_scaling_factor_dims_args);

  const int output_dims_args[] = {2, batch_size, num_units};
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_args);

  const int tensor_count = 9;  // 4 inputs, 1 output, 4 scratch
  TfLiteTensor tensors[] = {
      CreateFloatTensor(input_data, input_dims, "input"),
      CreateQuantizedTensor(weights_feature_data,
                            weights_feature_quantized_data,
                            weights_feature_dims, "weights_feature"),
      CreateQuantizedTensor(weights_time_data, weights_time_quantized_data,
                            weights_time_dims, "weights_time"),
      CreateFloatTensor(activation_state_data, activation_state_dims,
                        "activation_state", true /* is_variable */),
      CreateFloatTensor(output_data, output_dims, "output"),
      CreateFloatTensor(scratch_data, scratch_dims, "scratch_tensor"),
      CreateQuantizedTensor(scratch_input_quantized, input_dims,
                            "scratch_input_quantized", 1 /* placeholder-min */,
                            2 /* placehnolder-max */),
      CreateFloatTensor(scratch_scaling_factors, scratch_scaling_factors_dims,
                        "scratch_scaling_factors"),
      CreateFloatTensor(scratch_weights_time, weights_time_dims, "scratch_4"),
  };

  ValidateSVDFGoldens(batch_size, num_units, input_size, rank, tensors,
                      tensor_count, true /* is_hybrid */, golden_input_data,
                      golden_input_data_size, output_data, expected_output,
                      tolerance);
}

inline void TestHybridSVDFUint8(
    const int batch_size, const int num_units, const int input_size,
    const int memory_size, const int rank, float* input_data,
    float* weights_feature_data, uint8_t* weights_feature_quantized_data,
    float* weights_time_data, uint8_t* weights_time_quantized_data,
    float* activation_state_data, float* scratch_data,
    uint8_t* scratch_input_quantized, float* scratch_scaling_factors,
    float* scratch_weights_time, float* output_data, float* golden_input_data,
    int golden_input_data_size, float* expected_output,
    float tolerance = 1e-5f) {
  const int num_filters = num_units * rank;

  const int input_dims_arg[] = {2, batch_size, input_size};
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_arg);

  const int weights_feature_dims_args[] = {2, num_filters, input_size};
  TfLiteIntArray* weights_feature_dims =
      IntArrayFromInts(weights_feature_dims_args);

  const int weights_time_dims_args[] = {2, num_filters, memory_size};
  TfLiteIntArray* weights_time_dims = IntArrayFromInts(weights_time_dims_args);

  const int activation_state_dims_args[] = {2, batch_size,
                                            memory_size * num_filters};
  TfLiteIntArray* activation_state_dims =
      IntArrayFromInts(activation_state_dims_args);

  // Scratch output is the same shape as output:
  const int scratch_dims_args[] = {2, batch_size, num_filters};
  TfLiteIntArray* scratch_dims = IntArrayFromInts(scratch_dims_args);

  const int scratch_scaling_factor_dims_args[] = {1, batch_size};
  TfLiteIntArray* scratch_scaling_factors_dims =
      IntArrayFromInts(scratch_scaling_factor_dims_args);

  const int output_dims_args[] = {2, batch_size, num_units};
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_args);

  const int tensor_count = 9;  // 4 inputs, 1 output, 4 scratch
  TfLiteTensor tensors[] = {
      CreateFloatTensor(input_data, input_dims, "input"),

      CreateQuantizedTensor(weights_feature_data,
                            weights_feature_quantized_data,
                            weights_feature_dims, "weights_feature"),
      CreateQuantizedTensor(weights_time_data, weights_time_quantized_data,
                            weights_time_dims, "weights_time"),

      CreateFloatTensor(activation_state_data, activation_state_dims,
                        "activation_state", true /* is_variable */),
      CreateFloatTensor(output_data, output_dims, "output"),
      CreateFloatTensor(scratch_data, scratch_dims, "scratch_tensor"),

      CreateQuantizedTensor(scratch_input_quantized, input_dims,
                            "scratch_input_quantized", 1 /* placeholder-min */,
                            2 /* placehnolder-max */),
      CreateFloatTensor(scratch_scaling_factors, scratch_scaling_factors_dims,
                        "scratch_scaling_factors"),
      CreateFloatTensor(scratch_weights_time, weights_time_dims, "scratch_4"),
  };

  ValidateSVDFGoldens(batch_size, num_units, input_size, rank, tensors,
                      tensor_count, true /* is_hybrid */, golden_input_data,
                      golden_input_data_size, output_data, expected_output,
                      tolerance);
}

inline void TestIntegerSVDF(
    const int batch_size, const int num_units, const int input_size,
    const int memory_size, const int rank, int8_t* input_data,
    float input_scale, int8_t* weights_feature_data,
    float weights_feature_scale, int16_t* weights_time_data,
    float weights_time_scale, int32_t* bias_data, float bias_scale,
    int16_t* activation_state_data, float activation_scale,
    int32_t* scratch_data, int32_t* scratch_output_data, int8_t* output_data,
    float output_scale, int32_t effective_scale_1_a,
    int32_t effective_scale_1_b, int32_t effective_scale_2_a,
    int32_t effective_scale_2_b, int8_t* golden_input_data,
    int golden_input_data_size, int8_t* expected_output) {
  const int num_filters = num_units * rank;

  const int input_dims_arg[] = {2, batch_size, input_size};
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_arg);

  const int weights_feature_dims_args[] = {2, num_filters, input_size};
  TfLiteIntArray* weights_feature_dims =
      IntArrayFromInts(weights_feature_dims_args);

  const int weights_time_dims_args[] = {2, num_filters, memory_size};
  TfLiteIntArray* weights_time_dims = IntArrayFromInts(weights_time_dims_args);

  const int bias_dims_data[] = {1, num_units};
  TfLiteIntArray* bias_dims = IntArrayFromInts(bias_dims_data);

  const int activation_state_dims_args[] = {2, batch_size,
                                            memory_size * num_filters};
  TfLiteIntArray* activation_state_dims =
      IntArrayFromInts(activation_state_dims_args);

  // Scratch output is the same shape as output:
  const int scratch_dims_args[] = {2, batch_size, num_filters};
  TfLiteIntArray* scratch_dims = IntArrayFromInts(scratch_dims_args);

  // Full integer requires one more scratch tensor:
  const int scratch_output_dims_args[] = {2, num_units, batch_size};
  TfLiteIntArray* scratch_output_dims =
      IntArrayFromInts(scratch_output_dims_args);

  const int output_dims_args[] = {2, batch_size, num_units};
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_args);

  // Tensor size is higher due to workarounds in micro buffer usage
  // (b/132070898) and re-working scale calculations (b/146029510).
  const int tensor_count = 9;  // 5 inputs, 1 output, 2 scratch, 1 temp

  const int effective_scale_dims_args[] = {1, 4};
  int32_t effective_scale_data[] = {effective_scale_1_a, effective_scale_1_b,
                                    effective_scale_2_a, effective_scale_2_b};
  TfLiteIntArray* effective_scale_dims =
      IntArrayFromInts(effective_scale_dims_args);

  TfLiteTensor tensors[] = {
      CreateQuantizedTensor(input_data, input_dims, input_scale,
                            0 /* zero-point */, "input"),
      CreateQuantizedTensor(weights_feature_data, weights_feature_dims,
                            weights_feature_scale, 0 /* zero-point */,
                            "weights_feature"),
      CreateQuantizedTensor(weights_time_data, weights_time_dims,
                            weights_time_scale, 0 /* zero-point */,
                            "weights_time"),
      CreateQuantized32Tensor(bias_data, bias_dims, "bias", bias_scale),
      CreateQuantizedTensor(activation_state_data, activation_state_dims,
                            activation_scale, 0 /* zero-point */,
                            "activation_state", true /* is_variable */),
      CreateQuantizedTensor(output_data, output_dims, output_scale,
                            0 /* zero-point */, "output"),
      CreateQuantized32Tensor(scratch_data, scratch_dims, "scratch",
                              1.f /* scale-placeholder */),
      CreateQuantized32Tensor(scratch_output_data, scratch_output_dims,
                              "scratch_output", 1.f /* scale-placeholder */),
      CreateTensor(effective_scale_data, effective_scale_dims,
                   "effective_scale"),
  };

  ValidateIntegerSVDFGoldens(
      batch_size, num_units, input_size, rank, tensors, tensor_count,
      golden_input_data, golden_input_data_size, output_data, expected_output);
}  // namespace

}  // namespace
}  // namespace testing
}  // namespace tflite

TF_LITE_MICRO_TESTS_BEGIN

TF_LITE_MICRO_TEST(BlackBoxTestRank1) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 1;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {-0.31930989, -0.36118156, 0.0079667,
                                  0.37613347,  0.22197971,  0.12416199,
                                  0.27901134,  0.27557442,  0.3905206,
                                  -0.36137494, -0.06634006, -0.10640851};

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657};

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestSVDF(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_time_data, activation_state_data,
      scratch_data, output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_1);
}

TF_LITE_MICRO_TEST(BlackBoxTestRank2) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 2;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {
      -0.31930989, 0.0079667,   0.39296314,  0.37613347, 0.12416199,
      0.15785322,  0.27901134,  0.3905206,   0.21931258, -0.36137494,
      -0.10640851, 0.31053296,  -0.36118156, -0.0976817, -0.36916667,
      0.22197971,  0.15294972,  0.38031587,  0.27557442, 0.39635518,
      -0.21580373, -0.06634006, -0.02702999, 0.27072677};

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657,

      -0.14884081, 0.19931212,  -0.36002168, 0.34663299,  -0.11405486,
      0.12672701,  0.39463779,  -0.07886535, -0.06384811, 0.08249187,

      -0.26816407, -0.19905911, 0.29211238,  0.31264046,  -0.28664589,
      0.05698794,  0.11613581,  0.14078894,  0.02187902,  -0.21781836,

      -0.15567942, 0.08693647,  -0.38256618, 0.36580828,  -0.22922277,
      -0.0226903,  0.12878349,  -0.28122205, -0.10850525, -0.11955214,

      0.27179423,  -0.04710215, 0.31069002,  0.22672787,  0.09580326,
      0.08682203,  0.1258215,   0.1851041,   0.29228821,  0.12366763};

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];
  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestSVDF(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_time_data, activation_state_data,
      scratch_data, output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_2);
}

TF_LITE_MICRO_TEST(BlackBoxTestHybridRank1Int8) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 1;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {-0.31930989, -0.36118156, 0.0079667,
                                  0.37613347,  0.22197971,  0.12416199,
                                  0.27901134,  0.27557442,  0.3905206,
                                  -0.36137494, -0.06634006, -0.10640851};
  const int weights_feature_dims_count = num_filters * input_size;
  int8_t weights_feature_data_quantized[weights_feature_dims_count];

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657};
  const int weights_time_dims_count = num_filters * memory_size;
  int8_t weights_time_data_quantized[weights_time_dims_count];

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  int8_t scratch_input_quantized[input_size_dims_count];
  float scratch_scaling_factors[batch_size];
  float scratch_weights_time[weights_time_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestHybridSVDFInt8(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_feature_data_quantized, weights_time_data,
      weights_time_data_quantized, activation_state_data, scratch_data,
      scratch_input_quantized, scratch_scaling_factors, scratch_weights_time,
      output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_1, 0.002945 /* tolerance */);
}

TF_LITE_MICRO_TEST(BlackBoxTestHybridRank2Int8) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 2;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {
      -0.31930989, 0.0079667,   0.39296314,  0.37613347, 0.12416199,
      0.15785322,  0.27901134,  0.3905206,   0.21931258, -0.36137494,
      -0.10640851, 0.31053296,  -0.36118156, -0.0976817, -0.36916667,
      0.22197971,  0.15294972,  0.38031587,  0.27557442, 0.39635518,
      -0.21580373, -0.06634006, -0.02702999, 0.27072677};

  const int weights_feature_dims_count = num_filters * input_size;
  int8_t weights_feature_data_quantized[weights_feature_dims_count];

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657,

      -0.14884081, 0.19931212,  -0.36002168, 0.34663299,  -0.11405486,
      0.12672701,  0.39463779,  -0.07886535, -0.06384811, 0.08249187,

      -0.26816407, -0.19905911, 0.29211238,  0.31264046,  -0.28664589,
      0.05698794,  0.11613581,  0.14078894,  0.02187902,  -0.21781836,

      -0.15567942, 0.08693647,  -0.38256618, 0.36580828,  -0.22922277,
      -0.0226903,  0.12878349,  -0.28122205, -0.10850525, -0.11955214,

      0.27179423,  -0.04710215, 0.31069002,  0.22672787,  0.09580326,
      0.08682203,  0.1258215,   0.1851041,   0.29228821,  0.12366763};
  const int weights_time_dims_count = num_filters * memory_size;
  int8_t weights_time_data_quantized[weights_time_dims_count];

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  int8_t scratch_input_quantized[scratch_dims_count];
  float scratch_scaling_factors[batch_size];
  float scratch_weights_time[weights_time_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestHybridSVDFInt8(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_feature_data_quantized, weights_time_data,
      weights_time_data_quantized, activation_state_data, scratch_data,
      scratch_input_quantized, scratch_scaling_factors, scratch_weights_time,
      output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_2, 0.00625109 /* tolerance */);
}

TF_LITE_MICRO_TEST(BlackBoxTestHybridRank1Uint8) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 1;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {-0.31930989, -0.36118156, 0.0079667,
                                  0.37613347,  0.22197971,  0.12416199,
                                  0.27901134,  0.27557442,  0.3905206,
                                  -0.36137494, -0.06634006, -0.10640851};
  const int weights_feature_dims_count = num_filters * input_size;
  uint8_t weights_feature_data_quantized[weights_feature_dims_count];

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657};
  const int weights_time_dims_count = num_filters * memory_size;
  uint8_t weights_time_data_quantized[weights_time_dims_count];

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  uint8_t scratch_input_quantized[scratch_dims_count];
  float scratch_scaling_factors[batch_size];
  float scratch_weights_time[weights_time_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestHybridSVDFUint8(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_feature_data_quantized, weights_time_data,
      weights_time_data_quantized, activation_state_data, scratch_data,
      scratch_input_quantized, scratch_scaling_factors, scratch_weights_time,
      output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_1, 0.002945 /* tolerance */);
}

TF_LITE_MICRO_TEST(BlackBoxTestHybridRank2Uint8) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 2;
  constexpr int num_filters = num_units * rank;

  float weights_feature_data[] = {
      -0.31930989, 0.0079667,   0.39296314,  0.37613347, 0.12416199,
      0.15785322,  0.27901134,  0.3905206,   0.21931258, -0.36137494,
      -0.10640851, 0.31053296,  -0.36118156, -0.0976817, -0.36916667,
      0.22197971,  0.15294972,  0.38031587,  0.27557442, 0.39635518,
      -0.21580373, -0.06634006, -0.02702999, 0.27072677};

  const int weights_feature_dims_count = num_filters * input_size;
  uint8_t weights_feature_data_quantized[weights_feature_dims_count];

  float weights_time_data[] = {
      -0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
      0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

      0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
      -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

      -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
      0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

      -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
      -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657,

      -0.14884081, 0.19931212,  -0.36002168, 0.34663299,  -0.11405486,
      0.12672701,  0.39463779,  -0.07886535, -0.06384811, 0.08249187,

      -0.26816407, -0.19905911, 0.29211238,  0.31264046,  -0.28664589,
      0.05698794,  0.11613581,  0.14078894,  0.02187902,  -0.21781836,

      -0.15567942, 0.08693647,  -0.38256618, 0.36580828,  -0.22922277,
      -0.0226903,  0.12878349,  -0.28122205, -0.10850525, -0.11955214,

      0.27179423,  -0.04710215, 0.31069002,  0.22672787,  0.09580326,
      0.08682203,  0.1258215,   0.1851041,   0.29228821,  0.12366763};
  const int weights_time_dims_count = num_filters * memory_size;
  uint8_t weights_time_data_quantized[weights_time_dims_count];

  const int input_size_dims_count = batch_size * input_size;
  float input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  float activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  float scratch_data[scratch_dims_count];

  uint8_t scratch_input_quantized[scratch_dims_count];
  float scratch_scaling_factors[batch_size];
  float scratch_weights_time[weights_time_dims_count];

  const int output_dims_count = batch_size * num_units;
  float output_data[output_dims_count];

  tflite::testing::TestHybridSVDFUint8(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      weights_feature_data, weights_feature_data_quantized, weights_time_data,
      weights_time_data_quantized, activation_state_data, scratch_data,
      scratch_input_quantized, scratch_scaling_factors, scratch_weights_time,
      output_data, tflite::testing::svdf_input,
      sizeof(tflite::testing::svdf_input),
      tflite::testing::svdf_golden_output_rank_2, 0.00625109 /* tolerance */);
}

TF_LITE_MICRO_TEST(BlackBoxTestIntegerRank1) {
  constexpr int batch_size = 2;
  constexpr int num_units = 4;
  constexpr int input_size = 3;
  constexpr int memory_size = 10;
  constexpr int rank = 1;
  constexpr int num_filters = num_units * rank;

  int8_t weights_feature_data[] = {-81, -92, 2,   96,  57,  32,
                                   71,  70,  100, -92, -17, -27};
  const int weights_feature_dims_count = num_filters * input_size;

  int16_t weights_time_data[] = {
      -10464, 12324, 9142,  -11842, -11836, 7273,  9029,  -2175, 260,   4067,
      12795,  -3488, -3202, 5011,   12987,  -887,  12875, 5171,  7185,  10174,
      -12098, 12461, -7072, 8870,   7739,   11447, 5954,  11765, -5733, 10643,
      -3534,  8912,  4693,  -7761,  -8886,  -519,  -4898, 5067,  3205,  -1107,
  };
  const int weights_time_dims_count = num_filters * memory_size;

  int32_t bias_data[] = {-409707, 641518, 1662434, -113372};

  int8_t input_sequences_data[] = {
      64, 25,   34,   23,  68, -99, 16, -59,  -114, 46,  47, 94,
      18, -128, -96,  -73, 16, 96,  64, 25,   34,   23,  68, -99,
      16, -59,  -114, 46,  47, 94,  18, -128, -96,  -73, 16, 96,
      64, 25,   34,   23,  68, -99, 16, -59,  -114, 46,  47, 94,
      18, -128, -96,  -73, 16, 96,  64, 25,   34,   23,  68, -99,
      16, -59,  -114, 46,  47, 94,  18, -128, -96,  -73, 16, 96,
  };

  int8_t expected_output[] = {
      -9,  24,  31,   1,   -10, 10,  -3,  0,   2,   4,   -44, -7,  -10,  32,
      52,  1,   12,   -17, 9,   -8,  7,   16,  -11, -8,  -26, 29,  28,   16,
      -23, 26,  30,   -6,  -8,  -25, -86, -5,  -44, 59,  81,  15,  62,   -16,
      -37, 3,   27,   14,  34,  -10, 1,   24,  -25, 23,  31,  61,  67,   11,
      -64, -65, -128, -25, -53, 59,  127, 20,  20,  -29, -20, -15, -28,  0,
      8,   -27, 54,   61,  -67, 38,  38,  64,  115, 0,   -44, -75, -128, -20,
      -19, 93,  101,  35,  -5,  -56, 30,  -18, -40, -9,  -8,  -31,
  };

  const int input_size_dims_count = batch_size * input_size;
  int8_t input_data[input_size_dims_count];

  const int activation_state_dims_count =
      batch_size * memory_size * num_filters;
  int16_t activation_state_data[activation_state_dims_count];

  const int scratch_dims_count = batch_size * num_filters;
  int32_t scratch_data[scratch_dims_count];

  const int scratch_output_dims_count = batch_size * num_units;
  int32_t scratch_output_data[scratch_output_dims_count];

  const int output_dims_count = batch_size * num_units;
  int8_t output_data[output_dims_count];

  float input_scale = 1.f / INT8_MAX;            // Range  is [-1, 1]
  float weights_feature_scale = 0.5 / INT8_MAX;  // Range is [-0.5, 0.5]
  float weights_time_scale = 1 / INT16_MAX;      // Range is [-1, 1]
  float activation_scale = 16.f / INT16_MAX;     // Range is [-16, 16]
  float bias_scale = 512 / INT32_MAX;            // Range is [-512, 512]
  float output_scale = 0.5f / INT8_MAX;          // Range is [-0.5, 0.5]

  int32_t effective_scale_1_a = 1082163456;
  int32_t effective_scale_1_b = -3;
  int32_t effective_scale_2_a = 2139160192;
  int32_t effective_scale_2_b = -18;

  tflite::testing::TestIntegerSVDF(
      batch_size, num_units, input_size, memory_size, rank, input_data,
      input_scale, weights_feature_data, weights_feature_scale,
      weights_time_data, weights_time_scale, bias_data, bias_scale,
      activation_state_data, activation_scale, scratch_data,
      scratch_output_data, output_data, output_scale, effective_scale_1_a,
      effective_scale_1_b, effective_scale_2_a, effective_scale_2_b,
      input_sequences_data, sizeof(input_sequences_data), expected_output);
}

TF_LITE_MICRO_TESTS_END
