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

#ifndef _AMA_SCALER_H_
#define _AMA_SCALER_H_

#include <obs-module.h>
#include <ama-context.h>

#define MAX_SCALER_WIDTH 3840
#define MAX_SCALER_HEIGHT 2160
#define MIN_SCALER_WIDTH 284
#define MIN_SCALER_HEIGHT 160
#define MAX_SCALER_PARAMS 2

typedef enum { PLANE_Y = 0, PLANE_U, PLANE_V } PlaneId;

int32_t scaler_reserve(AmaCtx *ctx);

int32_t scaler_create(AmaCtx *ctx);

int32_t scaler_process_frame(struct encoder_frame *frame, AmaCtx *ctx);

int32_t scaler_destroy(AmaCtx *ctx);

#endif // _AMA_scaler_H_
