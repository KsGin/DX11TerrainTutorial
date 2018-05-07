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
	
	// Set the UI to display by default.
	m_displayUI = true;
	m_wireFrame = true;

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

	return;
}


bool ZoneClass::Frame(D3DClass* Direct3D, InputClass* Input, ShaderManagerClass* ShaderManager , TextureManagerClass* TextureManager, float frameTime, int fps)
{
	bool result;
	float posX, posY, posZ, rotX, rotY, rotZ;

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

	// Reset the world matrix.
	Direct3D->GetWorldMatrix(worldMatrix);

	// Turn the Z buffer back and back face culling on.
	Direct3D->TurnZBufferOn();
	Direct3D->TurnOnCulling();

	if (m_wireFrame) {
		Direct3D->EnableWireframe();
	}

	// Render the terrain grid using the texture shader.
	m_Terrain->Render(Direct3D->GetDeviceContext());
	result = ShaderManager->RenderTerrainShader(Direct3D->GetDeviceContext(), m_Terrain->GetIndexCount(), worldMatrix, viewMatrix, 
						    projectionMatrix, TextureManager->GetTexture(0),  TextureManager->GetTexture(1), m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if(!result)
	{
		return false;
	}

	if (m_wireFrame) {
		Direct3D->DisableWireframe();
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