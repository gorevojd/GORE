#ifndef GORE_RENDERER_H
#define GORE_RENDERER_H

#include "gore_render_state.h"

extern void SoftwareRenderStackToOutput(render_state* Stack, rgba_buffer* Buffer, rect2 ClipRect);
extern void RenderMultithreaded(platform_thread_queue* Queue, render_state* Stack, rgba_buffer* Buffer);
extern void RenderDickInjection(render_state* Stack, rgba_buffer* Buffer);

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