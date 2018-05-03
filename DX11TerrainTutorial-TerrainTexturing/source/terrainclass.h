////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <D3D11.h>
#include <fstream>
#include <directxmath.h>
#include "../../DX11TerrainTutorial-BitmapHeightMaps/source/textureclass.h"
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texcoord;
	};

	struct HeightMapType {
		float x, y, z;
	};

	struct ModelType {
		float x, y, z;
		float tu, tv;
	};

public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device* , char*);
	void Shutdown();
	bool Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadSetupFile(CHAR*);
	bool LoadBitmapHeightMap();
	void ShutdownHeightMap();
	void SetTerrainCoordinates();
	bool BuildTerrainModel();
	void ShutdownTerrainModel();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;

	int m_terrainHeight , m_terrainWidth;
	float m_heightScale;
	LPSTR m_terrainFilename;
	HeightMapType* m_heightMap;
	ModelType* m_terrainModel;
};

#endif