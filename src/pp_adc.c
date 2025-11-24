// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#include "tusb.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "byte_ops.h"
#include "dln2.h"
#include "main.h"

static const uint8_t adc_gpios[] = {
	26, 27, 28,
	29 // 1/3 voltage divider on VSYS
	// Channel 5 is the internal temperature sensor (no GPIO).
};

#define NUM_PP_ADC_CHANNELS (TU_ARRAY_SIZE(adc_gpios) + 1)

TU_VERIFY_STATIC(NUM_PP_ADC_CHANNELS <= DLN2_ADC_MAX_CHANNELS);

static const char *adc_cmd2str(uint16_t cmd)
{
	// clang-format off
	switch (cmd) {
	case DLN2_ADC_GET_CHANNEL_COUNT: return "GET_CHANNEL_COUNT";
	case DLN2_ADC_ENABLE: return "ENABLE";
	case DLN2_ADC_DISABLE: return "DISABLE";
	case DLN2_ADC_CHANNEL_ENABLE: return "CHANNEL_ENABLE";
	case DLN2_ADC_CHANNEL_DISABLE: return "CHANNEL_DISABLE";
	case DLN2_ADC_SET_RESOLUTION: return "SET_RESOLUTION";
	case DLN2_ADC_CHANNEL_GET_VAL: return "CHANNEL_GET_VAL";
	case DLN2_ADC_CHANNEL_GET_ALL_VAL: return "CHANNEL_GET_ALL_VAL";
	case DLN2_ADC_CHANNEL_SET_CFG: return "CHANNEL_SET_CFG";
	case DLN2_ADC_CHANNEL_GET_CFG: return "CHANNEL_GET_CFG"; /* Unused by kernel driver */
	case DLN2_ADC_CONDITION_MET_EV: return "CONDITION_MET_EV";
	default: return "???";
	}
	// clang-format on
}

struct {
	enum {
		PP_ADC_MODE_IDLE = 0,
		PP_ADC_MODE_ACTIVE,
		PP_ADC_MODE_ACTIVE_STREAMING,
	} mode;

	uint8_t channel_mask;
	uint16_t samples[NUM_PP_ADC_CHANNELS];

	uint64_t period_us;
	uint64_t next_sample_time_us;
} pp_adc;

static uint16_t pp_adc_read(uint8_t chan)
{
	adc_select_input(chan);
	uint16_t val = adc_read();

	// Pico has 12-bit ADC, kernel driver expects 10-bit ADC
	val = val >> 2;

	return val;
}

bool pp_adc_handle_request(uint16_t cmd, uint8_t const *data_in,
			   uint16_t data_in_len, uint8_t *data_out,
			   uint16_t *data_out_len)
{
	TU_VERIFY(data_in_len >= 1);

	uint8_t port = data_in[0];
	TU_VERIFY(port == 0); // always 0 in kernel driver

	switch (cmd) {
	case DLN2_ADC_GET_CHANNEL_COUNT:
		TU_ASSERT(*data_out_len >= 1);
		TU_LOG3("ADC: Getting number of channels\r\n");
		data_out[0] = NUM_PP_ADC_CHANNELS;
		*data_out_len = 1;
		break;
	case DLN2_ADC_SET_RESOLUTION:
		TU_VERIFY(data_in_len >= 2);
		TU_VERIFY(data_in[1] == DLN2_ADC_DATA_BITS);
		TU_LOG3("ADC: Setting resolution\r\n");
		*data_out_len = 0;
		break;
	case DLN2_ADC_CHANNEL_ENABLE: {
		TU_VERIFY(data_in_len >= 2);
		uint8_t chan = data_in[1];
		TU_LOG3("ADC: Enabling channel %u\r\n", chan);
		if (pp_adc.mode != PP_ADC_MODE_IDLE) {
			TU_LOG1("ADC: Failed to enable channel. Device is busy.\r\n");
			return false;
		}
		pp_adc.channel_mask |= (1 << chan);
		*data_out_len = 0;
		break;
	}
	case DLN2_ADC_CHANNEL_DISABLE: {
		TU_VERIFY(data_in_len >= 2);
		uint8_t chan = data_in[1];
		TU_LOG3("ADC: Disabling channel %u\r\n", chan);
		(void)chan;
		if (pp_adc.mode != PP_ADC_MODE_IDLE) {
			TU_LOG1("ADC: Failed to disable channel. Device is busy.\r\n");
			return false;
		}
		pp_adc.channel_mask &= ~(1 << chan);
		*data_out_len = 0;
		break;
	}
	case DLN2_ADC_ENABLE: {
		TU_ASSERT(*data_out_len >= 2);
		TU_LOG3("ADC: Enabling\r\n");
		if (pp_adc.mode != PP_ADC_MODE_IDLE) {
			TU_LOG1("ADC: Failed to enable. Device is busy.\r\n");
			return false;
		}
		pp_adc.mode = PP_ADC_MODE_ACTIVE;
		u16_to_buf_le(&data_out[0], 0); // no conflict
		*data_out_len = 2;
		break;
	}
	case DLN2_ADC_DISABLE: {
		TU_ASSERT(*data_out_len >= 2);
		TU_LOG3("ADC: Disabling\r\n");
		if (pp_adc.mode == PP_ADC_MODE_ACTIVE_STREAMING) {
			TU_LOG1("ADC: Failed to disable. Device is streaming.\r\n");
			return false;
		}
		pp_adc.mode = PP_ADC_MODE_IDLE;
		u16_to_buf_le(&data_out[0], 0); // no conflict
		*data_out_len = 2;
		break;
	}
	case DLN2_ADC_CHANNEL_GET_VAL: {
		TU_VERIFY(data_in_len >= 2);
		TU_ASSERT(*data_out_len >= 2);
		if (pp_adc.mode != PP_ADC_MODE_ACTIVE) {
			TU_LOG1("ADC: Failed to get value. Device not enabled or busy.\r\n");
			return false;
		}
		uint8_t chan = data_in[1];
		uint16_t val = pp_adc_read(chan);
		TU_LOG3("ADC: Getting channel %u value: %u\r\n", chan, val);
		u16_to_buf_le(&data_out[0], val);
		*data_out_len = 2;
		break;
	}
	case DLN2_ADC_CHANNEL_GET_ALL_VAL: {
		TU_LOG3("ADC: DLN2_ADC_CHANNEL_GET_ALL_VAL\r\n");
		TU_ASSERT(*data_out_len >= 2 + 2 * DLN2_ADC_MAX_CHANNELS);

		u16_to_buf_le(&data_out[0], 0); // unused by kernel driver

		int i;
		for (i = 0; i < NUM_PP_ADC_CHANNELS; i++) {
			u16_to_buf_le(&data_out[2 + 2 * i], pp_adc.samples[i]);
		}
		/* continuing */
		for (; i < DLN2_ADC_MAX_CHANNELS; i++) {
			u16_to_buf_le(&data_out[2 + 2 * i], 0);
		}

		*data_out_len = 2 + 2 * DLN2_ADC_MAX_CHANNELS;
		break;
	}
	case DLN2_ADC_CHANNEL_SET_CFG: {
		TU_VERIFY(data_in_len >= 5);
		// We don't need channel or type.
		uint16_t new_period_ms = u16_from_buf_le(&data_in[3]);
		TU_LOG3("ADC: Setting config (period=%" PRIu16 "ms)\r\n",
			new_period_ms);

		switch (pp_adc.mode) {
		case PP_ADC_MODE_IDLE:
			TU_LOG1("ADC: Setting config ignored. Device idle.\r\n");
			break;
		case PP_ADC_MODE_ACTIVE:
			if (new_period_ms != 0) {
				TU_LOG3("ADC: Start streaming (channel mask=0x%02" PRIx8
					")\r\n",
					pp_adc.channel_mask);

				pp_adc.period_us =
					(uint64_t)new_period_ms * 1000;

				uint64_t now =
					to_us_since_boot(get_absolute_time());
				pp_adc.next_sample_time_us =
					now + pp_adc.period_us;

				memset(pp_adc.samples, 0,
				       sizeof(pp_adc.samples));

				pp_adc.mode = PP_ADC_MODE_ACTIVE_STREAMING;
			} else {
				TU_LOG1("ADC: Not streaming. Sample rate is zero.\r\n");
			}
			break;
		case PP_ADC_MODE_ACTIVE_STREAMING:
			if (new_period_ms == 0) {
				TU_LOG3("ADC: End streaming\r\n");

				pp_adc.mode = PP_ADC_MODE_ACTIVE;
			} else {
				TU_LOG1("ADC: Cannot change sample rate during streaming.\r\n");
				return false;
			}
			break;
		}
		*data_out_len = 0;
		break;
	}
	default:
		TU_LOG1("ADC: Command not implemented: %s (%u)\r\n",
			adc_cmd2str(cmd), cmd);
		TU_VERIFY(false);
	}

	return true;
}

void pp_adc_task(void)
{
	uint64_t now = to_us_since_boot(get_absolute_time());
	if (pp_adc.mode == PP_ADC_MODE_ACTIVE_STREAMING &&
	    now > pp_adc.next_sample_time_us) {
		for (uint8_t chan = 0; chan < NUM_PP_ADC_CHANNELS; chan++) {
			if (((1 << chan) & pp_adc.channel_mask) == 0)
				continue;

			pp_adc.samples[chan] = pp_adc_read(chan);
		}

		// unsolicited message, so no echo code
		send_message_delayed(DLN2_ADC_CONDITION_MET_EV, 0,
				     DLN2_HANDLE_EVENT, NULL, 0);

		while (now > pp_adc.next_sample_time_us) {
			pp_adc.next_sample_time_us += pp_adc.period_us;
		}
	}
}

void pp_adc_init(void)
{
	adc_init();
	for (size_t i = 0; i < TU_ARRAY_SIZE(adc_gpios); i++) {
		adc_gpio_init(adc_gpios[i]);
	}
	adc_set_temp_sensor_enabled(true);
}
