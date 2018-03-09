#ifndef GORE_RENDERER_H
#define GORE_RENDERER_H

#include "gore_game_layer.h"
#include "gore_render_stack.h"

extern void SoftwareRenderStackToOutput(render_stack* Stack, rgba_buffer* Buffer, rect2 ClipRect);
extern void RenderMultithreaded(thread_queue* Queue, render_stack* Stack, rgba_buffer* Buffer);
extern void RenderDickInjection(render_stack* Stack, rgba_buffer* Buffer);

extern void RenderRectFast(
	rgba_buffer* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);

extern void RenderRect(
	rgba_buffer* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);
#endif