/*
 * Copyright (c) 2021 Stephanos Ioannidis <root@stephanos.io>
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <stdlib.h>
#include <arm_math.h>
#include "../../common/test_common.h"

#include "decim_q15.pat"

#define SNR_ERROR_THRESH		((float32_t)70)
#define ABS_ERROR_THRESH_Q15		((q15_t)5)
#define STATE_BUF_LEN			(16 + 768 - 1)

static void test_arm_fir_decimate_q15(void)
{
	uint32_t decim_factor, tap_count;
	size_t sample_index, block_size, ref_size;
	size_t sample_count = ARRAY_SIZE(in_config_decim) / 4;
	size_t length = ARRAY_SIZE(ref_decim);
	const uint32_t *config = in_config_decim;
	const q15_t *input = in_val_decim;
	const q15_t *coeff = in_coeff_decim;
	const q15_t *ref = ref_decim;
	q15_t *state, *output_buf, *output;
	arm_status status;
	arm_fir_decimate_instance_q15 inst;

	/* Allocate buffers */
	state = malloc(STATE_BUF_LEN * sizeof(q15_t));
	zassert_not_null(state, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output_buf = malloc(length * sizeof(q15_t));
	zassert_not_null(output_buf, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output = output_buf;

	/* Enumerate samples */
	for (sample_index = 0; sample_index < sample_count; sample_index++) {
		/* Resolve sample configurations */
		decim_factor = config[0];
		tap_count = config[1];
		block_size = config[2];
		ref_size = config[3];

		/* Initialise instance */
		status = arm_fir_decimate_init_q15(&inst, tap_count,
						   decim_factor, coeff, state,
						   block_size);

		zassert_equal(status, ARM_MATH_SUCCESS,
			      ASSERT_MSG_INCORRECT_COMP_RESULT);

		/* Run test function */
		arm_fir_decimate_q15(&inst, input, output, block_size);

		/* Increment pointers */
		input += block_size;
		output += ref_size;
		coeff += tap_count;
		config += 4;
	}

	/* Validate output */
	zassert_true(
		test_snr_error_q15(length, output_buf, ref, SNR_ERROR_THRESH),
		ASSERT_MSG_SNR_LIMIT_EXCEED);

	zassert_true(
		test_near_equal_q15(length, output_buf, ref,
			ABS_ERROR_THRESH_Q15),
		ASSERT_MSG_ABS_ERROR_LIMIT_EXCEED);

	/* Free buffers */
	free(state);
	free(output_buf);
}

static void test_arm_fir_interpolate_q15(void)
{
	uint32_t filter_length, tap_count;
	size_t sample_index, block_size, ref_size;
	size_t sample_count = ARRAY_SIZE(in_config_interp) / 4;
	size_t length = ARRAY_SIZE(ref_interp);
	const uint32_t *config = in_config_interp;
	const q15_t *input = in_val_interp;
	const q15_t *coeff = in_coeff_interp;
	const q15_t *ref = ref_interp;
	q15_t *state, *output_buf, *output;
	arm_status status;
	arm_fir_interpolate_instance_q15 inst;

	/* Allocate buffers */
	state = malloc(STATE_BUF_LEN * sizeof(q15_t));
	zassert_not_null(state, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output_buf = malloc(length * sizeof(q15_t));
	zassert_not_null(output_buf, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output = output_buf;

	/* Enumerate samples */
	for (sample_index = 0; sample_index < sample_count; sample_index++) {
		/* Resolve sample configurations */
		filter_length = config[0];
		tap_count = config[1];
		block_size = config[2];
		ref_size = config[3];

		/* Initialise instance */
		status = arm_fir_interpolate_init_q15(&inst, filter_length,
						      tap_count, coeff,
						      state, block_size);

		zassert_equal(status, ARM_MATH_SUCCESS,
			      ASSERT_MSG_INCORRECT_COMP_RESULT);

		/* Run test function */
		arm_fir_interpolate_q15(&inst, input, output, block_size);

		/* Increment pointers */
		input += block_size;
		output += ref_size;
		coeff += tap_count;
		config += 4;
	}

	/* Validate output */
	zassert_true(
		test_snr_error_q15(length, output_buf, ref, SNR_ERROR_THRESH),
		ASSERT_MSG_SNR_LIMIT_EXCEED);

	zassert_true(
		test_near_equal_q15(length, output_buf, ref,
			ABS_ERROR_THRESH_Q15),
		ASSERT_MSG_ABS_ERROR_LIMIT_EXCEED);

	/* Free buffers */
	free(state);
	free(output_buf);
}

void test_filtering_decim_q15(void)
{
	ztest_test_suite(filtering_decim_q15,
		ztest_unit_test(test_arm_fir_decimate_q15),
		ztest_unit_test(test_arm_fir_interpolate_q15)
		);

	ztest_run_test_suite(filtering_decim_q15);
}
