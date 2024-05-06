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

#ifndef _AMA_ENCODER_H_
#define _AMA_ENCODER_H_

#include <obs-module.h>
#include <xma.h>
#include <ama-context.h>

int32_t encoder_reserve(AmaCtx *ctx);

int32_t encoder_create(AmaCtx *ctx);

int32_t encoder_process_frame(struct encoder_packet *packet,
			      bool *received_packet, AmaCtx *ctx);

int32_t encoder_destroy(AmaCtx *ctx);

#endif // _AMA_ENCODER_H_
