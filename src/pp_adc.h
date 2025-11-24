// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#ifndef _PICOPORTS_PP_ADC_H_
#define _PICOPORTS_PP_ADC_H_

void pp_adc_init(void);
void pp_adc_task(void);
bool pp_adc_handle_request(uint16_t cmd, uint8_t const *data_in,
			   uint16_t data_in_len, uint8_t *data_out,
			   uint16_t *data_out_len);

#endif /* _PICOPORTS_PP_ADC_H_ */
