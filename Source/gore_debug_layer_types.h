#ifndef GORE_DEBUG_LAYER_TYPES_INTERFACE_H_INCLUDED
#define GORE_DEBUG_LAYER_TYPES_INTERFACE_H_INCLUDED

struct voxel_generation_statistics{
	int HashTableCollisionCount;
	int HashTableInsertedElements;

	int FreeChunkThreadworksCount;
	int TotalChunkThreadworksCount;

	int FreeGenThreadworksCount;
	int TotalGenThreadworksCount;

	v3 CameraPos;
	int CurrentChunkX;
	int CurrentChunkY;
	int CurrentCHunkZ;

	int ChunksViewDistance;
	int BlocksViewDistance;

	int MeshGenerationsStartedThisFrame;

	int ChunksLoaded;
	int ChunksPushed;
	int TrianglesPushed;
	int TrianglesLoaded;

	platform_thread_queue* Queue;

	int HashTableMemUsed;
	int GenTasksMemUsed;
	int ChunkTasksMemUsed;
	int MeshTasksMemUsed;
	stacked_memory* GenerationMem;
};

#endif