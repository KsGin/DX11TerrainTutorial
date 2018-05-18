////////////////////////////////////////////////////////////////////////////////
// Filename: zoneclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "zoneclass.h"


ZoneClass::ZoneClass()
{
	m_UserInterface = 0;
	m_Camera = 0;
	m_Position = 0;
	m_Terrain = 0;
	m_Light = 0;
	m_SkyDome = 0;
	m_Frustum = 0;
	m_SkyPlane = 0;
}


ZoneClass::ZoneClass(const ZoneClass& other)
{
}


ZoneClass::~ZoneClass()
{
}


bool ZoneClass::Initialize(D3DClass* Direct3D, HWND hwnd, int screenWidth, int screenHeight, float screenDepth)
{
	bool result;

	// Create the user interface object.
	m_UserInterface = new UserInterfaceClass;
	if(!m_UserInterface)
	{
		return false;
	}

	// Initialize the user interface object.
	result = m_UserInterface->Initialize(Direct3D, screenHeight, screenWidth);
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the user interface object.", "Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera and build the matrices needed for rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();
	m_Camera->RenderBaseViewMatrix();

	// Create the light object.
	m_Light = new LightClass;
	if(!m_Light)
	{
		return false;
	}

	// Initialize the light object.
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(-0.5f, -1.0f, -0.5f);

	// Create the position object.
	m_Position = new PositionClass;
	if(!m_Position)
	{
		return false;
	}

	// Set the initial position and rotation.
	m_Position->SetPosition(128.0f, 5.0f, -10.0f);
	m_Position->SetRotation(0.0f, 0.0f, 0.0f);

	// Create the terrain object.
	m_Terrain = new TerrainClass;
	if(!m_Terrain)
	{
		return false;
	}

	CHAR setup_txt[] = "./data/setup.txt";
	// Initialize the terrain object.
	result = m_Terrain->Initialize(Direct3D->GetDevice() , setup_txt);
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the terrain object.", "Error", MB_OK);
		return false;
	}

	// Create the skydome object
	m_SkyDome = new SkyDomeClass;
	if (!m_SkyDome) {
		return false;
	}

	// Initialize the skydome object
	result = m_SkyDome->Initialize(Direct3D->GetDevice());
	if (!result) {
		return false;
	}
	
	// Create the skyplane object
	m_SkyPlane = new SkyPlaneClass;
	if (!m_SkyPlane) {
		return false;
	}

	CHAR cloud001[] = "./data/cloud001.dds";
	CHAR cloud002[] = "./data/cloud002.dds";
	// Initialize the skyplane object
	result = m_SkyPlane->Initialize(Direct3D->GetDevice(), Direct3D->GetDeviceContext(), cloud001, cloud002);
	if (!result) {
		return false;
	}

	// Create the frustum object.
	m_Frustum = new FrustumClass;
	if(!m_Frustum)
	{
		return false;
	}

	// Initialize the frustum object.
	m_Frustum->Initialize(screenDepth);

	// Set the UI to display by default.
	m_displayUI = true;
	m_wireFrame = false;
	// Set the rendering of cell lines initially to enabled.
	m_cellLines = false;
	// Set the user locked to the terrain height for movement.
	m_heightLocked = false;

	// Set the screen size
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	return true;
}


void ZoneClass::Shutdown()
{
	// Release the terrain object.
	if(m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	// Release the frustum object.
	if(m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	// Release the position object.
	if(m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the light object.
	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the user interface object.
	if(m_UserInterface)
	{
		m_UserInterface->Shutdown();
		delete m_UserInterface;
		m_UserInterface = 0;
	}

	// Release the skydome object
	if (m_SkyDome) {
		m_SkyDome->Shutdown();
		delete m_SkyDome;
		m_SkyDome = 0;
	}

	// Release the skyplane object
	if (m_SkyPlane) {
		m_SkyPlane->Shutdown();
		delete m_SkyPlane;
		m_SkyPlane = 0;
	}

	return;
}


bool ZoneClass::Frame(D3DClass* Direct3D, InputClass* Input, ShaderManagerClass* ShaderManager , TextureManagerClass* TextureManager, float frameTime, int fps)
{
	bool result;
	float posX, posY, posZ, rotX, rotY, rotZ , height;
	bool foundHeight;
	static float prevHeight = 0;

	// Do the frame input processing.
	HandleMovementInput(Input, frameTime);

	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Do the frame processing for the user interface.
	result = m_UserInterface->Frame(Direct3D->GetDeviceContext(), fps, posX, posY, posZ, rotX, rotY, rotZ);
	if(!result)
	{
		return false;
	}

	// Do the sky plane frame processing.
	m_SkyPlane->Frame();

	// Do the terrain frame processing.
	m_Terrain->Frame();

	// If the height is locked to the terrain then position the camera on top of it.
	if(m_heightLocked)
	{
		posY -= prevHeight;
		// Get the height of the triangle that is directly underneath the given camera position.
		foundHeight = m_Terrain->GetHeightAtPosition(posX, posZ, height);
		if(foundHeight)
		{
			// If there was a triangle under the camera then position the camera just above it by three meters.
			m_Position->SetPosition(posX, height + posY, posZ);
			m_Camera->SetPosition(posX, height + posY, posZ);
			prevHeight = height;
		}
	}

	// Render the graphics.
	result = Render(Direct3D, ShaderManager , TextureManager);
	if(!result)
	{
		return false;
	}

	return true;
}


void ZoneClass::HandleMovementInput(InputClass* Input, float frameTime)
{
	bool keyDown;
	float posX, posY, posZ, rotX, rotY, rotZ;
	static int mouseLocationX , mouseLocationY;
	static int preMouseLocationX = 0, preMouseLocationY = 0;
	static bool isStart = false;

	Input->GetMouseLocation(mouseLocationX, mouseLocationY);

	if (isStart) {
		m_Position->Look(mouseLocationX - preMouseLocationX , mouseLocationY - preMouseLocationY);
	} else {
		isStart = true;
	}

	mouseLocationX = m_screenWidth / 2;
	mouseLocationY = m_screenWidth / 2;
	preMouseLocationX = mouseLocationX;
	preMouseLocationY = mouseLocationY;
	Input->SetMouseLocation(mouseLocationX, mouseLocationY);

	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(frameTime);

	// Handle the keyboard input.
	keyDown = Input->IsAPressed();
	m_Position->MoveLeft(keyDown);

	keyDown = Input->IsDPressed();
	m_Position->MoveRight(keyDown);

	keyDown = Input->IsWPressed();
	m_Position->MoveForward(keyDown);

	keyDown = Input->IsSPressed();
	m_Position->MoveBackward(keyDown);

	keyDown = Input->IsSpacePressed();
	m_Position->MoveUpward(keyDown);

	keyDown = Input->IsLeftCtrlPressed();
	m_Position->MoveDownward(keyDown);

	keyDown = Input->IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = Input->IsRightPressed();
	m_Position->TurnRight(keyDown);

	keyDown = Input->IsUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = Input->IsDownPressed();
	m_Position->LookDownward(keyDown);

	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_Camera->SetPosition(posX, posY, posZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

	// Determine if the user interface should be displayed or not.
	if(Input->IsF1Toggled())
	{
		m_displayUI = !m_displayUI;
	}

	if (Input->IsF2Toggled()) {
		m_wireFrame = !m_wireFrame;
	}

	if (Input->IsF3Toggled()) {
		m_cellLines = !m_cellLines;
	}

	// Determine if we should be locked to the terrain height when we move around or not.
	if(Input->IsF4Toggled())
	{
		m_heightLocked = !m_heightLocked;
	}


	return;
}


bool ZoneClass::Render(D3DClass* Direct3D, ShaderManagerClass* ShaderManager , TextureManagerClass* TextureManager)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, baseViewMatrix, orthoMatrix;
	bool result;
	XMFLOAT3 cameraPosition;
	
	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	Direct3D->GetOrthoMatrix(orthoMatrix);
	
	// Get the position of the camera.
	cameraPosition = m_Camera->GetPosition();

	// Construct the frustum.
	m_Frustum->ConstructFrustum(projectionMatrix, viewMatrix);

	// Clear the buffers to begin the scene.
	Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Turn off back face culling and turn off the Z buffer.
	Direct3D->TurnOffCulling();
	Direct3D->TurnZBufferOff();

	// Translate the sky dome to be centered around the camera position.
	worldMatrix = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Render the sky dome using the sky dome shader.
	m_SkyDome->Render(Direct3D->GetDeviceContext());
	result = ShaderManager->RenderSkydomeShader(Direct3D->GetDeviceContext(), m_SkyDome->GetIndexCount(), worldMatrix, viewMatrix, 
						    projectionMatrix, m_SkyDome->GetApexColor(), m_SkyDome->GetCenterColor());
	if(!result)
	{
		return false;
	}

	Direct3D->EnableAlphaBlending();

	m_SkyPlane->Render(Direct3D->GetDeviceContext());
	ShaderManager->RenderSkyPlaneShader(Direct3D->GetDeviceContext(), m_SkyPlane->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix
		, m_SkyPlane->GetCloudTexture1(), m_SkyPlane->GetCloudTexture2(), 
		m_SkyPlane->GetTranslation(0), m_SkyPlane->GetTranslation(1), m_SkyPlane->GetTranslation(2), m_SkyPlane->GetTranslation(3), m_SkyPlane->GetBrightness());

	Direct3D->DisableAlphaBlending();

	// Reset the world matrix.
	Direct3D->GetWorldMatrix(worldMatrix);

	// Turn the Z buffer back and back face culling on.
	Direct3D->TurnZBufferOn();
	Direct3D->TurnOnCulling();

	if (m_wireFrame) {
		Direct3D->EnableWireframe();
	}

	// Render the terrain cells (and cell lines if needed).
	for(auto i=0; i<m_Terrain->GetCellCount(); i++)
	{
		// Render each terrain cell if it is visible only.
		result = m_Terrain->RenderCell(Direct3D->GetDeviceContext(), i, m_Frustum);
		if(result)
		{
			// Render the cell buffers using the terrain shader.
			result = ShaderManager->RenderTerrainShader(Direct3D->GetDeviceContext(), m_Terrain->GetCellIndexCount(i), worldMatrix, viewMatrix,
								    projectionMatrix, TextureManager->GetTexture(0), TextureManager->GetTexture(1), TextureManager->GetTexture(2),
								     TextureManager->GetTexture(3) , m_Light->GetDirection(), m_Light->GetDiffuseColor());
			if(!result)
			{
				return false;
			}

			// If needed then render the bounding box around this terrain cell using the color shader. 
			if(m_cellLines)
			{
				m_Terrain->RenderCellLines(Direct3D->GetDeviceContext(), i);
				ShaderManager->RenderColorShader(Direct3D->GetDeviceContext(), m_Terrain->GetCellLinesIndexCount(i), worldMatrix, 
								 viewMatrix, projectionMatrix);
				if(!result)
				{
					return false;
				}
			}
		}
	}

	if (m_wireFrame) {
		Direct3D->DisableWireframe();
	}

	// Update the render counts in the UI.
	result = m_UserInterface->UpdateRenderCounts(Direct3D->GetDeviceContext(), m_Terrain->GetRenderCount(), m_Terrain->GetCellsDrawn(), 
						     m_Terrain->GetCellsCulled());
	if(!result)
	{
		return false;
	}


	// Render the user interface.
	if(m_displayUI)
	{
		result = m_UserInterface->Render(Direct3D, ShaderManager, worldMatrix, baseViewMatrix, orthoMatrix);
		if(!result)
		{
			return false;
		}
	}

	
	// Present the rendered scene to the screen.
	Direct3D->EndScene();

	return true;
}