////////////////////////////////////////////////////////////////////////////////
// Filename: zoneclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _ZONECLASS_H_
#define _ZONECLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "inputclass.h"
#include "shadermanagerclass.h"
#include "timerclass.h"
#include "userinterfaceclass.h"
#include "cameraclass.h"
#include "positionclass.h"
#include "terrainclass.h"
#include "texturemanagerclass.h"
#include "lightclass.h"
#include "skydomeclass.h"
#include "frustumclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ZoneClass
////////////////////////////////////////////////////////////////////////////////
class ZoneClass
{
public:
	ZoneClass();
	ZoneClass(const ZoneClass&);
	~ZoneClass();

	bool Initialize(D3DClass*, HWND, int, int, float);
	void Shutdown();
	bool Frame(D3DClass*, InputClass*, ShaderManagerClass* , TextureManagerClass* , float, int);

private:
	void HandleMovementInput(InputClass*, float);
	bool Render(D3DClass*, ShaderManagerClass* , TextureManagerClass*);
private:
	UserInterfaceClass* m_UserInterface;
	CameraClass* m_Camera;
	PositionClass* m_Position;
	TerrainClass* m_Terrain;
	LightClass* m_Light;
	SkyDomeClass* m_SkyDome;
	FrustumClass* m_Frustum;
	bool m_displayUI , m_wireFrame , m_cellLines;
	int m_screenWidth, m_screenHeight;
};

#endif