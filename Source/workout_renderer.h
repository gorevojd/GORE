#ifndef WORKOUT_RENDERER_H
#define WORKOUT_RENDERER_H

#include "workout_game_layer.h"
#include "workout_render_stack.h"

#include "workout_debug.h"

extern void SoftwareRenderStackToOutput(render_stack* Stack, rgba_buffer* Buffer);

#endif