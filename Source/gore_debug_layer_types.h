#ifndef GORE_DEBUG_LAYER_TYPES_INTERFACE_H_INCLUDED
#define GORE_DEBUG_LAYER_TYPES_INTERFACE_H_INCLUDED

struct voxel_generation_statistics{
	int HashTableCollisionCount;
	int HashTableInsertedElements;

	int FreeWorkThreadworksCount;
	int TotalWorkThreadworksCount;

	int FreeGenThreadworksCount;
	int TotalGenThreadworksCount;

	v3 CameraPos;
	int CurrentChunkX;
	int CurrentChunkY;
	int CurrentCHunkZ;

	int ChunksViewDistance;
	int BlocksViewDistance;

	int MeshGenerationsStartedThisFrame;

	int ChunksPushedToRender;

	platform_thread_queue* Queue;
};

#endif