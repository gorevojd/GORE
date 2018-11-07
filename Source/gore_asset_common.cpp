#include "gore_asset_common.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

bitmap_info AssetAllocateBitmapInternal(u32 Width, u32 Height, void* PixelsData) {
	bitmap_info Result = {};

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;

	Result.WidthOverHeight = (float)Width / (float)Height;

	Result.Pixels = (u8*)PixelsData;

	return(Result);
}

bitmap_info AssetAllocateBitmap(u32 Width, u32 Height) {
	u32 BitmapDataSize = Width * Height * 4;
	void* PixelsData = malloc(BitmapDataSize);

	memset(PixelsData, 0, BitmapDataSize);

	bitmap_info Result = AssetAllocateBitmapInternal(Width, Height, PixelsData);

	return(Result);
}

void AssetCopyBitmapData(bitmap_info* Dst, bitmap_info* Src) {
	Assert(Dst->Width == Src->Width);
	Assert(Dst->Height == Src->Height);

	u32* DestOut = (u32*)Dst->Pixels;
	u32* ScrPix = (u32*)Src->Pixels;
	for (int j = 0; j < Src->Height; j++) {
		for (int i = 0; i < Src->Width; i++) {
			*DestOut++ = *ScrPix++;
		}
	}
}

void AssetDeallocateBitmap(bitmap_info* Buffer) {
	if (Buffer->Pixels) {
		free(Buffer->Pixels);
	}
}

bitmap_info AssetLoadIMG(char* Path) {
	bitmap_info Result = {};

	int Width;
	int Height;
	int Channels;
	u8* ImageMemory = stbi_load(Path, &Width, &Height, &Channels, 4);

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;
	Result.WidthOverHeight = (float)Width / (float)Height;

	u32 RawImageSize = Width * Height * 4;
	u32 PixelsCount = Width * Height;
	Result.Pixels = (u8*)malloc(RawImageSize);

	for (u32 PixelIndex = 0;
		PixelIndex < PixelsCount;
		PixelIndex++)
	{
		u32 Value = *((u32*)ImageMemory + PixelIndex);

		u32 Color =
			((Value >> 24) & 0x000000FF) |
			((Value & 0xFF) << 24) |
			((Value & 0xFF00) << 8) |
			((Value & 0x00FF0000) >> 8);

		v4 Col = UnpackRGBA(Color);
		Col.r *= Col.a;
		Col.g *= Col.a;
		Col.b *= Col.a;
		Color = PackRGBA(Col);

		*((u32*)Result.Pixels + PixelIndex) = Color;
	}

	stbi_image_free(ImageMemory);

	return(Result);
}

bitmap_info AssetGenerateCheckerboardBitmap(
	int Width,
	int CellCountPerWidth,
	v3 FirstColor,
	v3 SecondColor)
{
	bitmap_info Result = {};

	if (CellCountPerWidth <= 1) {
		CellCountPerWidth = 2;
	}

	int OneCellWidth = Width / CellCountPerWidth;

	/*
	NOTE(dima): Result color has alpha of 1.
	So color don't need to be premultiplied
	*/
	u32 FirstColorPacked = PackRGBA(V4(FirstColor, 1.0f));
	u32 SecondColorPacked = PackRGBA(V4(SecondColor, 1.0f));

	Result.Pixels = (u8*)malloc(Width * Width * 4);
	Result.Pitch = Width * 4;
	Result.Width = Width;
	Result.Height = Width;
	Result.TextureHandle = 0;
	Result.WidthOverHeight = 1.0f;

	int X, Y;
	for (Y = 0; Y < Width; Y++) {

		int VertIndex = Y / OneCellWidth;

		for (X = 0; X < Width; X++) {
			int HorzIndex = X / OneCellWidth;

			u32 CellColor = ((HorzIndex + VertIndex) & 1) ? SecondColorPacked : FirstColorPacked;

			u32* Out = (u32*)Result.Pixels + Width * Y + X;

			*Out = CellColor;
		}
	}

	return(Result);
}

inline vertex_info LoadMeshVertex(float* Pos, float* Tex, float* Norm) {
	vertex_info Result = {};

	if (Pos) {
		Result.P.x = *Pos;
		Result.P.y = *(Pos + 1);
		Result.P.z = *(Pos + 2);
	}

	if (Tex) {
		Result.UV.x = *Tex;
		Result.UV.y = *(Tex + 1);
	}

	if (Norm) {
		Result.N.x = *Norm;
		Result.N.y = *(Norm + 1);
		Result.N.z = *(Norm + 2);
	}

	return(Result);
}

mesh_info AssetLoadMeshFromVertices(
	float* Verts, u32 VertsCount,
	u32* Indices, u32 IndicesCount,
	u32 VertexLayout,
	b32 CalculateNormals,
	b32 CalculateTangents)
{
	mesh_info Result = {};

	Result.Handle = 0;
	Result.IndicesCount = IndicesCount;
	Result.Indices = (u32*)malloc(IndicesCount * sizeof(u32));
	for (int IndexIndex = 0;
		IndexIndex < IndicesCount;
		IndexIndex++)
	{
		Result.Indices[IndexIndex] = Indices[IndexIndex];
	}

	u32 Increment = 0;
	switch (VertexLayout) {
		case MeshVertexLayout_PUV: {
			Increment = 5;
			Result.MeshType = MeshType_Simple;
		}break;

		case MeshVertexLayout_PNUV:
		case MeshVertexLayout_PUVN: {
			Increment = 8;
			Result.MeshType = MeshType_Simple;
		}break;

		case MeshVertexLayout_PUVNC:
		case MeshVertexLayout_PNUVC: {
			Increment = 11;
			Result.MeshType = MeshType_Simple;
		}break;
	}

	Result.VerticesCount = VertsCount;
	Result.Vertices = (vertex_info*)malloc(sizeof(vertex_info) * VertsCount);

	float *VertexAt = Verts;
	for (int VertexIndex = 0;
		VertexIndex < VertsCount;
		VertexIndex++)
	{

		vertex_info* ToLoad = Result.Vertices + VertexIndex;

		switch (VertexLayout) {
			case MeshVertexLayout_PUV: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, 0);
			}break;

			case MeshVertexLayout_PUVN: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PNUV: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PUVNC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PNUVC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;
		}

		VertexAt += Increment;
	}

	if (CalculateTangents || CalculateNormals) {
		for (int Index = 0;
			Index < Result.IndicesCount;
			Index += 3)
		{
			int Index0 = Result.Indices[Index];
			int Index1 = Result.Indices[Index + 1];
			int Index2 = Result.Indices[Index + 2];

			v3 P0 = Result.Vertices[Index0].P;
			v3 P1 = Result.Vertices[Index1].P;
			v3 P2 = Result.Vertices[Index2].P;

			v2 Tex0 = Result.Vertices[Index0].UV;
			v2 Tex1 = Result.Vertices[Index1].UV;
			v2 Tex2 = Result.Vertices[Index2].UV;

			v3 Edge1 = P1 - P0;
			v3 Edge2 = P2 - P0;

			if (CalculateTangents) {
				v2 DeltaTex1 = Tex1 - Tex0;
				v2 DeltaTex2 = Tex2 - Tex0;

				float InvDet = 1.0f / (DeltaTex1.x * DeltaTex2.y - DeltaTex2.x * DeltaTex1.y);

				v3 T = InvDet * (DeltaTex2.y * Edge1 - DeltaTex1.y * Edge2);
				v3 B = InvDet * (DeltaTex1.x * Edge2 - DeltaTex2.x * Edge1);

				T = Normalize(T);
				/*
				NOTE(dima): bitangent calculation is implemented
				but not used...
				*/
				B = Normalize(B);

				//NOTE(dima): Setting the calculating tangent to the vertex;
				Result.Vertices[Index0].T = T;
				Result.Vertices[Index1].T = T;
				Result.Vertices[Index2].T = T;
			}

			//NOTE(dima): Normals calculation and setting
			if (CalculateNormals) {
				v3 TriNormal = Normalize(Cross(Edge2, Edge1));

				Result.Vertices[Index0].N = TriNormal;
				Result.Vertices[Index1].N = TriNormal;
				Result.Vertices[Index2].N = TriNormal;
			}
		}
	}

	return(Result);
}

inline void ASSETGenerateWriteVertex(float* Vertex, v3 P, v2 UV) {
	//NOTE(dima): Writing position
	*Vertex = P.x;
	*(Vertex + 1) = P.y;
	*(Vertex + 2) = P.z;


	//NOTE(dima): Writing texture coordinate
	*(Vertex + 3) = UV.x;
	*(Vertex + 4) = UV.y;
}

mesh_info AssetGenerateSphere(int Segments, int Rings) {
	mesh_info Result = {};

	float Radius = 0.5f;

	Segments = Max(Segments, 3);
	Rings = Max(Rings, 2);

	//NOTE(dima): 2 top and bottom triangle fans + 
	int VerticesCount = (Segments * 3) * 2 + (Segments * (Rings - 2)) * 4;
	int IndicesCount = (Segments * 3) * 2 + (Segments * (Rings - 2)) * 6;

	//NOTE(dima): 5 floats per vertex
	float* Vertices = (float*)malloc(VerticesCount * 5 * sizeof(float));
	u32* Indices = (u32*)malloc(IndicesCount * sizeof(u32));

	float AngleVert = GORE_PI / (float)Rings;
	float AngleHorz = GORE_TWO_PI / (float)Segments;

	int VertexAt = 0;
	int IndexAt = 0;

	for (int VertAt = 1; VertAt <= Rings; VertAt++) {
		float CurrAngleVert = (float)VertAt * AngleVert;
		float PrevAngleVert = (float)(VertAt - 1) * AngleVert;

		float PrevY = Cos(PrevAngleVert) * Radius;
		float CurrY = Cos(CurrAngleVert) * Radius;

		float SinVertPrev = Sin(PrevAngleVert);
		float SinVertCurr = Sin(CurrAngleVert);

		for (int HorzAt = 1; HorzAt <= Segments; HorzAt++) {
			float CurrAngleHorz = (float)HorzAt * AngleHorz;
			float PrevAngleHorz = (float)(HorzAt - 1) * AngleHorz;

			v3 P0, P1, C0, C1;
			v2 P0uv, P1uv, C0uv, C1uv;

			P0.y = PrevY;
			P1.y = PrevY;

			C0.y = CurrY;
			C1.y = CurrY;

			//TODO(dima): handle triangle fan case

			P0.x = Cos(PrevAngleHorz) * SinVertPrev * Radius;
			P1.x = Cos(CurrAngleHorz) * SinVertPrev * Radius;

			P0.z = Sin(PrevAngleHorz) * SinVertPrev * Radius;
			P1.z = Sin(CurrAngleHorz) * SinVertPrev * Radius;

			C0.x = Cos(PrevAngleHorz) * SinVertCurr * Radius;
			C1.x = Cos(CurrAngleHorz) * SinVertCurr * Radius;

			C0.z = Sin(PrevAngleHorz) * SinVertCurr * Radius;
			C1.z = Sin(CurrAngleHorz) * SinVertCurr * Radius;

			P0uv = V2(0.0f, 0.0f);
			P1uv = V2(0.0f, 0.0f);
			C0uv = V2(0.0f, 0.0f);
			C1uv = V2(0.0f, 0.0f);

			float* V0 = Vertices + VertexAt * 5;
			float* V1 = Vertices + (VertexAt + 1) * 5;
			float* V2 = Vertices + (VertexAt + 2) * 5;
			float* V3 = Vertices + (VertexAt + 3) * 5;

			if (VertAt == 1) {
				ASSETGenerateWriteVertex(V0, P0, P0uv);
				ASSETGenerateWriteVertex(V1, C0, C0uv);
				ASSETGenerateWriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else if (VertAt == Rings) {
				ASSETGenerateWriteVertex(V0, P1, P1uv);
				ASSETGenerateWriteVertex(V1, P0, P0uv);
				ASSETGenerateWriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else {
				ASSETGenerateWriteVertex(V0, P1, P1uv);
				ASSETGenerateWriteVertex(V1, P0, P0uv);
				ASSETGenerateWriteVertex(V2, C0, C0uv);
				ASSETGenerateWriteVertex(V3, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;
				Ind[3] = VertexAt;
				Ind[4] = VertexAt + 2;
				Ind[5] = VertexAt + 3;

				IndexAt += 6;
				VertexAt += 4;
			}
		}
	}

	Result = AssetLoadMeshFromVertices(Vertices, VerticesCount, Indices, IndicesCount, MeshVertexLayout_PUV, 1, 1);

	free(Vertices);
	free(Indices);

	return(Result);
}

mesh_info AssetGenerateCylynder(float Height, float Radius, int SidesCount) {
	mesh_info Result = {};

	SidesCount = Max(3, SidesCount);

	int VerticesCount = SidesCount * 4 + SidesCount * 2 * 3;
	int IndicesCount = SidesCount * 6 + SidesCount * 2 * 3;

	float Angle = GORE_TWO_PI / (float)SidesCount;

	int IndexAt = 0;
	int VertexAt = 0;

	//NOTE(dima): 5 floats per vertex
	float* Vertices = (float*)malloc(sizeof(float) * 5 * VerticesCount);
	u32* Indices = (u32*)malloc(sizeof(u32) * IndicesCount);

	//NOTE(dima): Building top triangle fans
	float TopY = Height * 0.5f;
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		float TopY = Height * 0.5f;

		v3 CurrP;
		v3 PrevP;
		v3 Center = V3(0.0f, 0.0f, 0.0f);

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = TopY;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = TopY;
		PrevP.z = Sin(PrevAngle) * Radius;

		v2 CurrUV = V2(0.0f, 0.0f);
		v2 PrevUV = V2(0.0f, 0.0f);
		v2 CentUV = V2(0.0f, 0.0f);

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;

		Center.y = TopY;
		ASSETGenerateWriteVertex(V0, PrevP, PrevUV);
		ASSETGenerateWriteVertex(V1, CurrP, CurrUV);
		ASSETGenerateWriteVertex(V2, Center, CentUV);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;

		VertexAt += 3;
		IndexAt += 3;
	}

	//NOTE(dima): Building bottom triangle fans
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		float BotY = -Height * 0.5f;

		v3 CurrP;
		v3 PrevP;
		v3 Center = V3(0.0f, 0.0f, 0.0f);

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = BotY;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = BotY;
		PrevP.z = Sin(PrevAngle) * Radius;

		v2 CurrUV = V2(0.0f, 0.0f);
		v2 PrevUV = V2(0.0f, 0.0f);
		v2 CentUV = V2(0.0f, 0.0f);

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;

		Center.y = BotY;
		ASSETGenerateWriteVertex(V0, CurrP, CurrUV);
		ASSETGenerateWriteVertex(V1, PrevP, PrevUV);
		ASSETGenerateWriteVertex(V2, Center, CentUV);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;

		VertexAt += 3;
		IndexAt += 3;
	}

	//NOTE(dima): Building sides
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		v3 CurrP;
		v3 PrevP;

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = 0.0f;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = 0.0f;
		PrevP.z = Sin(PrevAngle) * Radius;

		v3 TopC, TopP;
		v3 BotC, BotP;

		v2 TopCuv = V2(0.0f, 0.0f);
		v2 TopPuv = V2(0.0f, 0.0f);
		v2 BotCuv = V2(0.0f, 0.0f);
		v2 BotPuv = V2(0.0f, 0.0f);

		TopC = CurrP;
		BotC = CurrP;
		TopP = PrevP;
		BotP = PrevP;

		TopC.y = Height * 0.5f;
		TopP.y = Height * 0.5f;
		BotC.y = -Height * 0.5f;
		BotP.y = -Height * 0.5f;

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;
		float* V3 = Vertices + (VertexAt + 3) * 5;

		ASSETGenerateWriteVertex(V0, TopC, TopCuv);
		ASSETGenerateWriteVertex(V1, TopP, TopPuv);
		ASSETGenerateWriteVertex(V2, BotP, BotPuv);
		ASSETGenerateWriteVertex(V3, BotC, BotCuv);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;
		Ind[3] = VertexAt;
		Ind[4] = VertexAt + 2;
		Ind[5] = VertexAt + 3;

		VertexAt += 4;
		IndexAt += 6;
	}

	Result = AssetLoadMeshFromVertices(Vertices, VerticesCount, Indices, IndicesCount, MeshVertexLayout_PUV, 1, 1);

	free(Vertices);
	free(Indices);

	return(Result);
}