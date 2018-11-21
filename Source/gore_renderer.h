#ifndef GORE_RENDERER_H
#define GORE_RENDERER_H

#include "gore_render_state.h"

void SoftwareRenderStackToOutput(render_stack* Stack, bitmap_info* Buffer, rect2 ClipRect);
void RenderMultithreaded(platform_thread_queue* Queue, render_stack* Stack, bitmap_info* Buffer);
void RenderDickInjection(render_stack* Stack, bitmap_info* Buffer);

void RenderRectFast(
	bitmap_info* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);

void RenderRect(
	bitmap_info* Buffer,
	v2 P,
	v2 Dim,
	v4 ModulationColor01,
	rect2 ClipRect);


#endif