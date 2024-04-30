/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ama-scaler.h"

int32_t scaler_create(obs_data_t *settings, obs_encoder_t *encoder,
		      ScalerCtx scl_ctx)
{
	(void)encoder;
	(void)settings;
	(void)scl_ctx;
	return 0;
}

int32_t scaler_process_frame()
{
	return 0;
}

int32_t scaler_destroy()
{
	return 0;
}
