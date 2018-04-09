#ifndef GORE_RENDERER_H
#define GORE_RENDERER_H

#include "gore_render_state.h"

extern void SoftwareRenderStackToOutput(render_state* Stack, bitmap_info* Buffer, rect2 ClipRect);
extern void RenderMultithreaded(platform_thread_queue* Queue, render_state* Stack, bitmap_info* Buffer);
extern void RenderDickInjection(render_state* Stack, bitmap_info* Buffer);

extern void RenderRectFast(
	bitmap_info* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);

extern void RenderRect(
	bitmap_info* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);
#endif