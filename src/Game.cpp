#include <map>
#include <list>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <d3d11.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <windows.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "dxut.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "d3dx11effect.h"
#include "DXUTsettingsDlg.h"

#include "Gun.h"
#include "Mesh.h"
#include "debug.h"
#include "Random.h"
#include "Skybox.h"
#include "Terrain.h"
#include "Explosion.h"
#include "Animation.h"
#include "GameEffect.h"
#include "Keybindings.h"
#include "AbsAnimation.h"
#include "ConfigParser.h"
#include "SpriteRenderer.h"
#include "EnemySpawnModel.h"
#include "ComplexAnimation.h"

// Helpwe macros
#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

// Terrain scaling parameters
struct ScalingParam
{
	float width;
	float height;
	float depth;
}                                        g_terrainScalingParams;

// Camera
struct CAMERAPARAMS {
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}                                        g_cameraParams;

float                                    g_cameraMoveScaler   = 100.f;
float                                    g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                       g_camera;						    // A first person camera

																		    // User Interface
CDXUTDialog                              g_hud;							    // Dialog for standard controls
CDXUTDialog                              g_sampleUI;						// Dialog for sample specific controls
CD3DSettingsDlg                          g_settingsDlg;					    // Device settings dialog
CDXUTTextHelper*                         g_txtHelper = NULL;
CDXUTDialogResourceManager               g_dialogResourceManager;		    // Manager for shared resources of dialogs

XMMATRIX								 g_meshWorld;
XMMATRIX                                 g_terrainWorld;					// Object to World Space transformation
bool									 g_cameraMovement  = false;
bool                                     g_terrainSpinning = false;

// Scene information
Terrain									 g_terrain;
XMVECTOR                                 g_lightDir;
GameEffect								 g_gameEffect;					    // CPU part of Shader

Skybox*									 g_skybox;
																		    // Utilities
KeyBindings*                             g_keys;
ConfigParser						     g_configParser;					// Config parser for game.cfg
SpriteRenderer*							 g_spriteRenderer;

EnemySpawnModel							 g_spawner;
float									 g_cBorder    = 0;
float									 g_spawnTimer = 0;

std::default_random_engine               g_randomGenerator;

std::map<std::string, Mesh*>			 g_meshes;
std::map<std::string, AbsAnimation*>	 g_animations;

std::list<Gun>						     g_gun;
std::list<Explosion>                     g_explosions;
std::list<Projectile>					 g_projectiles;
std::list<EnemyInstance>				 g_enemyInstances;
std::list<ExplosionParticle>             g_explosionParticles;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);

void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *, DXGI_FORMAT, bool, void*);
void CALLBACK OnMouseEvent(bool leftDown, bool rightDown, bool middleDown, bool side1Down, bool side2Down, int wheelDelta, int x, int y, void* userContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);

void InitApp();
void DeinitApp();
void RenderText();
void ReleaseShader();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Old Direct3D Documentation:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

	// DXUT Documentaion:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT

	// New Direct3D Documentaion (just for reference, use old documentation to find explanations):
	// http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx


	// Initialize COM library for windows imaging components
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (hr != S_OK)
	{
	MessageBox(0, L"Error calling CoInitializeEx", L"Error", MB_OK | MB_ICONERROR);
	exit(-1);
	}

	// Set DXUT callbacks
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	
	DXUTSetIsInGammaCorrectMode(false);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);

	InitApp();
	DXUTInit(true, true, NULL);			    // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"Wrath of Aliens");   // Title of the game

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 1280, 720);

	DXUTMainLoop();						    // Enter into the DXUT render loop
	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize and Deinitialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	HRESULT hr;
	
	size_t size;
	char   pathA[MAX_PATH];
	WCHAR  path[MAX_PATH];

	// Parse the config file
	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	wcstombs_s(&size, pathA, path, MAX_PATH);
	
	// Parse the game configuration file
	g_configParser.load(pathA);

	// Set scaling parameters for game terrain
	g_terrainScalingParams.height = g_configParser.getTerrainHeight();
	g_terrainScalingParams.width  = g_configParser.getTerrainWidth();
	g_terrainScalingParams.depth  = g_configParser.getTerrainDepth();

	// Intialize the user interface
	g_settingsDlg.Init(&g_dialogResourceManager);
	g_hud.Init(&g_dialogResourceManager);
	
	g_sampleUI.Init(&g_dialogResourceManager);
	g_hud.SetCallback(OnGUIEvent);

	
	int iY = 30, iYo = 26;
	
	g_hud.AddButton(IDC_TOGGLEFULLSCREEN,  L"Toggle full screen",  0, iY,        170, 22);
	g_hud.AddButton(IDC_TOGGLEREF,         L"Toggle REF (F3)",     0, iY += iYo, 170, 22, VK_F3);
	g_hud.AddButton(IDC_CHANGEDEVICE,      L"Change device (F2)",  0, iY += iYo, 170, 22, VK_F2);
	g_hud.AddButton(IDC_RELOAD_SHADERS,    L"Reload shaders (F5)", 0, iY += 24,  170, 22, VK_F5);

	g_sampleUI.SetCallback(OnGUIEvent); iY = 10; iY += 24;
	g_sampleUI.AddCheckBox(IDC_TOGGLESPIN, L"Toggle Spinning",     0, iY += 24,  125, 22, g_terrainSpinning);
	
	// Parse mesh infos from configuration and initialize them
	const std::vector<MeshInfo> meshes = g_configParser.getMeshInfo();
	for (auto it : meshes) 
	{
		Mesh* mesh = new Mesh(it.filename_t3d,          it.filename_dds_diffuse,
							  it.filename_dds_specular, it.filename_dds_glow);
		g_meshes.emplace(it.myName, mesh);
	}
	
	// Initialize Border Circle for Enemy Spawn Model and initialize it
	g_cBorder = (g_configParser.getTerrainWidth() + g_configParser.getTerrainDepth()) / float(1.50);
	g_spawner =  EnemySpawnModel(g_configParser);
	
	// Animation & Complex Animation objects are parsed and allocated dynamically
	const std::vector<AnimationInfo> animations = g_configParser.getAnimationInfo();
	for (auto animation : animations)
	{
		XMFLOAT3 animationVector = XMFLOAT3(animation.vectorX, animation.vectorY, animation.vectorZ);
		if (animation.animationType == AnimationInfo::Type::ROTATION)
		{
			Animation* g_animation = new Animation(animation.meshName, Type::ROTATION, animationVector, animation.duration);
			g_animations.emplace(animation.animationName, g_animation);
		}
		else if (animation.animationType == AnimationInfo::Type::TRANSLATION)
		{
			Animation* g_animation = new Animation(animation.meshName, Type::TRANSLATION, animationVector, animation.duration);
			g_animations.emplace(animation.animationName, g_animation);
		}
	}

	const std::vector<ComplexAnimationInfo> complexAnimations = g_configParser.getComplexAnimationInfo();
	for (auto complexAnim : complexAnimations)
	{
		std::vector<AbsAnimation*> subAnimations;
		for (auto subAnimName : complexAnim.animationList)
		{
			AbsAnimation* animation = g_animations.at(subAnimName);
			subAnimations.push_back(animation);
		}
		ComplexAnimation* g_animation = new ComplexAnimation(complexAnim.numberOfSubAnimations, subAnimations);
		g_animations.emplace(complexAnim.animationName, g_animation);
	}

	// Parse gun infos and initialize them.
	const std::vector<GunInfo> guns = g_configParser.getGunInfo();
	for (auto gun : guns)
	{
		Gun new_gun = Gun(gun, g_keys);
		if (gun.trigger != "-")
		{
			if (g_animations.find(gun.trigger) != g_animations.end()) 
			{
				AbsAnimation* animation = g_animations.at(gun.trigger);
				new_gun.setAnimation(animation);
			}
		}
		g_gun.push_back(new_gun);
	}

	// Initialize Sprite Renderer
	g_spriteRenderer =  new SpriteRenderer(g_configParser.getSpriteFiles());
	g_skybox		 =  new Skybox(g_configParser);
}

void DeinitApp() 
{	
	KeyBindings::destroyBindings();

	for (auto it : g_meshes) 
	{
		Mesh* elementToDelete = it.second;
		SAFE_DELETE(elementToDelete);
	}

	for (auto g_animation : g_animations)
	{		
		AbsAnimation* animationToDelete = g_animation.second;
		SAFE_DELETE(animationToDelete);
	}
	g_animations.clear();

	SAFE_DELETE(g_spriteRenderer);
	SAFE_DELETE(g_skybox);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_txtHelper->Begin();
	g_txtHelper->SetInsertionPos(5, 5);
	g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
	g_txtHelper->DrawTextLine(DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
	g_txtHelper->DrawTextLine(DXUTGetDeviceStats());
	g_txtHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *, DXGI_FORMAT, bool, void*)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pDeviceSettings);
	UNREFERENCED_PARAMETER(pUserContext);

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			DXUTDisplaySwitchingToREFWarning();
		}
	}
	//// Enable anti aliasing
	pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pBackBufferSurfaceDesc);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
	
	V_RETURN(g_dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	V_RETURN(g_settingsDlg.OnD3D11CreateDevice(pd3dDevice));

	g_txtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15);

	V_RETURN(ReloadShader(pd3dDevice));

	// Initialize the camera
	float LookAt    = g_configParser.getTerrainDepth() / 2;
	float EyeHeight = g_configParser.getTerrainHeight() * float(1.25);
	XMVECTOR vAt    = XMVectorSet(0.0f, 0.0f, LookAt, 1.0f);          // ... facing at this position
	XMVECTOR vEye   = XMVectorSet(0.0f, EyeHeight, 0.0f, 0.0f);       // Camera eye is here
	g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	
	// Create the terrain
	V_RETURN(g_terrain.create(pd3dDevice, &g_configParser));

	// Create meshes
	for (auto it : g_meshes) 
	{
		Mesh* elementToCreate = it.second;
		V_RETURN(elementToCreate->create(pd3dDevice));
	}

	V_RETURN(Mesh::createInputLayout(pd3dDevice, g_gameEffect.meshPass1));

	// Create sprite renderer
	V_RETURN(g_spriteRenderer->create(pd3dDevice));

	V_RETURN(g_skybox->create(pd3dDevice));

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	g_dialogResourceManager.OnD3D11DestroyDevice();
	g_settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	// Destroy the terrain
	g_terrain.destroy();
	
	// Destroy meshes
	for (auto it : g_meshes) 
	{
		Mesh* elementToDestroy = it.second;
		elementToDestroy->destroy();
	}
	Mesh::destroyInputLayout();

	// Destroy sprites
	g_spriteRenderer->destroy();
	
	g_skybox->destroy();

	SAFE_DELETE(g_txtHelper);

	ReleaseShader();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pSwapChain);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// Intialize the user interface

	V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(g_settingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	g_hud.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_hud.SetSize(170, 170);
	g_sampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_sampleUI.SetSize(170, 300);

	// Initialize the camera

	g_cameraParams.aspect = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_cameraParams.fovy = 0.785398f;
	g_cameraParams.nearPlane = 1.f;
	g_cameraParams.farPlane = 5000.f;

	g_camera.SetProjParams(g_cameraParams.fovy, g_cameraParams.aspect, g_cameraParams.nearPlane, g_cameraParams.farPlane);
	g_camera.SetEnablePositionMovement(false);
	g_camera.SetRotateButtons(false, false, false, true);
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	g_camera.SetResetCursorAfterMove(true);
	g_camera.SetDrag(true);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	HRESULT hr;

	ReleaseShader();

	V_RETURN(g_gameEffect.create(pd3dDevice));
	
	V_RETURN(g_spriteRenderer->reloadShader(pd3dDevice));

	V_RETURN(g_skybox->reloadShader(pd3dDevice));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	g_gameEffect.destroy();

	g_spriteRenderer->releaseShader();

	g_skybox->releaseShader();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass messages to settings dialog if its active
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_hud.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = g_sampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Use the mouse weel to control the movement speed
	if (uMsg == WM_MOUSEWHEEL) {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_cameraMoveScaler *= (1 + zDelta / 500.0f);
		if (g_cameraMoveScaler < 0.1f)
			g_cameraMoveScaler = 0.1f;
		g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nChar);
	UNREFERENCED_PARAMETER(bKeyDown);
	UNREFERENCED_PARAMETER(bAltDown);
	UNREFERENCED_PARAMETER(pUserContext);
	if (nChar == 'C' && bKeyDown)
	{
		if (g_cameraMovement)
		{
			g_cameraMovement = false;
			g_camera.SetEnablePositionMovement(g_cameraMovement);
		}
		else
		{
			g_cameraMovement = true;
			g_camera.SetEnablePositionMovement(g_cameraMovement);
		}
	}
	
	for (auto gun = g_gun.begin(); gun != g_gun.end(); )
	{
		if (nChar == gun->gunKey())
		{
			gun->gunKeyState(bKeyDown);
		}
		gun++;
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nEvent);
	UNREFERENCED_PARAMETER(pControl);
	UNREFERENCED_PARAMETER(pUserContext);

	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		g_settingsDlg.SetActive(!g_settingsDlg.IsActive()); break;
	case IDC_TOGGLESPIN:
		g_terrainSpinning = g_sampleUI.GetCheckBox(IDC_TOGGLESPIN)->GetChecked();
		break;
	case IDC_RELOAD_SHADERS:
		ReloadShader(DXUTGetD3D11Device());
		break;
	}
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	// Update the camera's position based on user input 
	g_camera.FrameMove(fElapsedTime);

	// Initialize the terrain world matrix
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx

	// Start with identity matrix
	g_terrainWorld = XMMatrixIdentity();
	g_terrainWorld *= XMMatrixScaling(g_terrainScalingParams.width, g_terrainScalingParams.height, g_terrainScalingParams.depth);

	if (g_terrainSpinning)
	{
		// If spinng enabled, rotate the world matrix around the y-axis
		g_terrainWorld *= XMMatrixRotationY(10.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
	}

	// Set the light vector
	g_lightDir = XMVectorSet(1, 1, 1, 0); // Direction to the directional light in world space    
	g_lightDir = XMVector3Normalize(g_lightDir);

	// Spawn new enemies - Check life cycle - Check hit points and create explosion - Update positions
	g_spawnTimer -= fElapsedTime;

	if (g_spawnTimer < 0)
	{
		g_spawnTimer += g_configParser.getSpawnInfo().timeInfo;
		
		EnemyInstance anInstance;
		g_spawner.generateEnemyInstance(anInstance);
		
		g_enemyInstances.push_back(anInstance);
	}

	for (auto enemy = g_enemyInstances.begin(); enemy != g_enemyInstances.end(); ) {
		float posRadius = (enemy->pos.x * enemy->pos.x + enemy->pos.z * enemy->pos.z);
		if (g_cBorder*g_cBorder < posRadius) 
		{
			auto enemy_remove = enemy;
			enemy++;
			g_enemyInstances.erase(enemy_remove);
			continue;
		}

		for (auto projectile = g_projectiles.begin(); projectile != g_projectiles.end(); )
		{
			double projEnemyDistance = (projectile->position.x - enemy->pos.x) * (projectile->position.x - enemy->pos.x) + \
					   				   (projectile->position.y - enemy->pos.y) * (projectile->position.y - enemy->pos.y) + \
									   (projectile->position.z - enemy->pos.z) * (projectile->position.z - enemy->pos.z);
			double touchDistane = enemy->enemyInfo.unitSize + projectile->radius;

			if (projEnemyDistance <= touchDistane*touchDistane)
			{
				enemy->enemyInfo.health -= projectile->info->damage;
				auto proj_remove = projectile;
				projectile++;
				g_projectiles.erase(proj_remove);
			}
			else
			{
				projectile++;
			}
		}

		if (enemy->enemyInfo.health <= 0)
		{
			// Start explosion
			XMFLOAT3 position(enemy->pos.x, enemy->pos.y, enemy->pos.z);
			Explosion explosion(position, g_configParser.getExplosionInfo());
			explosion.generateParticles(50, 200, g_explosionParticles);
			g_explosions.push_back(explosion);

			auto enemy_remove = enemy;
			enemy++;
			g_enemyInstances.erase(enemy_remove);
		}
		else 
		{ 
			enemy++; 
		}
	}

	for (auto it = g_enemyInstances.begin(); it != g_enemyInstances.end(); ) {
		it->pos.x += fElapsedTime * it->vel.x * it->enemyInfo.speed;
		it->pos.y += fElapsedTime * it->vel.y * it->enemyInfo.speed;
		it->pos.z += fElapsedTime * it->vel.z * it->enemyInfo.speed;
		it++;
	}
	// End of enemy instance checks

	// Update explosions and particles
	for (auto explosion = g_explosions.begin(); explosion != g_explosions.end(); )
	{
		explosion->update(fElapsedTime);
		if (explosion->isStale())
		{
			auto explosion_remove = explosion;
			explosion++;
			g_explosions.erase(explosion_remove);
		}
		else
		{
			explosion++;
		}
	}

	for (auto particle = g_explosionParticles.begin(); particle!= g_explosionParticles.end(); )
	{
		if (particle->updateParticle(fElapsedTime))
		{
			auto particle_remove = particle;
			particle++;
			g_explosionParticles.erase(particle_remove);
		}
		else
		{
			particle++;
		}
	}
	// End of explosion and particle update

	// Update projectiles generated by Gun class
	for (auto projectile = g_projectiles.begin(); projectile != g_projectiles.end(); )
	{
		if (projectile->updateProjectile(fElapsedTime))
		{
			auto proj_remove = projectile;
			projectile++;
			g_projectiles.erase(proj_remove);
		}
		else
		{
			projectile++;
		}
	}
	// End of projectile update

	// Fire guns
	for (auto gun = g_gun.begin(); gun != g_gun.end(); )
	{
		if (gun->fire(fElapsedTime, &g_camera, g_projectiles))
		{	
			#if defined(DEBUG) | defined(_DEBUG)			
				if (gun->info().type == GunInfo::Type::GATLING)
					std::cout << "Gatling gun is fired!" << std::endl;
				else if(gun->info().type == GunInfo::Type::PLASMA)
					std::cout << "Plasma gun is fired!" << std::endl;
			#endif
		}
		gun++;
	}
	// End of fire guns
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pd3dDevice);
	UNREFERENCED_PARAMETER(fTime);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	pd3dImmediateContext->ClearRenderTargetView(pRTV, clearColor);


	if (g_gameEffect.effect == NULL) {
		g_txtHelper->Begin();
		g_txtHelper->SetInsertionPos(5, 5);
		g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
		g_txtHelper->DrawTextLine(L"SHADER ERROR");
		g_txtHelper->End();
		return;
	}

	// Clear the depth stencil
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	// Update variables that change once per frame
	XMMATRIX const view = g_camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	XMMATRIX const proj = g_camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
	XMVECTOR cameraPosition = g_camera.GetEyePt();
	V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&cameraPosition));
	V(g_gameEffect.lightDirEV->SetFloatVector((float*)&g_lightDir));
	// Render Terrain
	XMMATRIX worldViewProj = g_terrainWorld * view * proj;
	XMMATRIX g_nomalTerrainWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, g_terrainWorld));
	//Assignment 5 settig  World Normals Projection Matrix
	V(g_gameEffect.worldEV->SetMatrix((float*)&g_terrainWorld));
	V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&worldViewProj));
	V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&g_nomalTerrainWorld));

	// Set input layout
	pd3dImmediateContext->IASetInputLayout(nullptr);
	g_terrain.render(pd3dImmediateContext, g_gameEffect.pass0);

	const vector<ObjectInfo> objects = g_configParser.getObjectInfo();
	for (auto it : objects) {
		if (it.type == ObjectInfo::Type::COCKPIT) {
			Mesh* elementToRender = g_meshes.at(it.myName);
			
			XMMATRIX mTrans, mScale, mRot;
			mRot = XMMatrixRotationX(DEG2RAD(it.rotX))*XMMatrixRotationY(DEG2RAD(it.rotY))*XMMatrixRotationZ(DEG2RAD(it.rotZ));
			mTrans = XMMatrixTranslation(float(it.trnX), float(it.trnY), float(it.trnZ));
			mScale = XMMatrixScaling(float(it.scaling), float(it.scaling), float(it.scaling));
			
			for ( auto g_animation : g_animations )
			{
				AbsAnimation* animation = g_animation.second;
				if (animation->getMeshName() == it.myName)
				{
					if (animation->getAnimationType() == Type::ROTATION)
					{
						mRot = animation->getAnimationMat(fElapsedTime) * mRot;
					}
					else if (animation->getAnimationType() == Type::TRANSLATION)
					{
						mTrans *= animation->getAnimationMat(fElapsedTime);
					}
				}
			}

			g_meshWorld = mScale * mRot * mTrans * g_camera.GetWorldMatrix();

			XMMATRIX meshViewProj = g_meshWorld * view * proj;
			XMMATRIX g_normalMeshWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, g_meshWorld));

			V(g_gameEffect.meshEV->SetMatrix((float*)&g_meshWorld));
			V(g_gameEffect.meshViewProjectionEV->SetMatrix((float*)&meshViewProj));
			V(g_gameEffect.meshNormalsEV->SetMatrix((float*)&g_normalMeshWorld));


			elementToRender->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV,
				g_gameEffect.specularEV, g_gameEffect.glowEV);
		}
		if (it.type == ObjectInfo::Type::GRAOUND) {
			Mesh* elementToRender = g_meshes.at(it.myName);

			XMMATRIX mTrans, mScale, mRot;
			mRot = XMMatrixRotationX(DEG2RAD(it.rotX))*XMMatrixRotationY(DEG2RAD(it.rotY))*XMMatrixRotationZ(DEG2RAD(it.rotZ));
			mTrans = XMMatrixTranslation(float(it.trnX), float(it.trnY), float(it.trnZ));
			mScale = XMMatrixScaling(float(it.scaling), float(it.scaling), float(it.scaling));

			for ( auto g_animation : g_animations )
			{
				AbsAnimation* animation = g_animation.second;
				if (animation->getMeshName() == it.myName)
				{ 
					if (animation->getAnimationType() == Type::ROTATION)
					{
						mRot = animation->getAnimationMat(fElapsedTime) * mRot;
					}
					else if (animation->getAnimationType() == Type::TRANSLATION)
					{
						mTrans *= animation->getAnimationMat(fElapsedTime);
					}
				}
			}

			g_meshWorld = mScale * mRot * mTrans;

			if (g_terrainSpinning)
			{
				// If spinng enabled, rotate the world matrix around the y-axis
				g_meshWorld *= XMMatrixRotationY(10.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
			}

			XMMATRIX meshViewProj = g_meshWorld * view * proj;
			XMMATRIX g_normalMeshWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, g_meshWorld));

			V(g_gameEffect.meshEV->SetMatrix((float*)&g_meshWorld));
			V(g_gameEffect.meshViewProjectionEV->SetMatrix((float*)&meshViewProj));
			V(g_gameEffect.meshNormalsEV->SetMatrix((float*)&g_normalMeshWorld));


			elementToRender->render(pd3dImmediateContext, 
									g_gameEffect.meshPass1, g_gameEffect.diffuseEV,
									g_gameEffect.specularEV, g_gameEffect.glowEV);
		}
	}

	for (auto it : g_enemyInstances)
	{
		Mesh* elementToRender = g_meshes.at(it.enemyInfo.meshName);

		XMMATRIX mTrans, mScale, mRot;

		mRot = XMMatrixRotationX(DEG2RAD(it.enemyInfo.rotX)) * \
			   XMMatrixRotationY(DEG2RAD(it.enemyInfo.rotY)) * \
			   XMMatrixRotationZ(DEG2RAD(it.enemyInfo.rotZ));
		mTrans = XMMatrixTranslation(float(it.enemyInfo.trnX), float(it.enemyInfo.trnY), float(it.enemyInfo.trnZ));
		mScale = XMMatrixScaling(float(it.enemyInfo.scaling), float(it.enemyInfo.scaling), float(it.enemyInfo.scaling));

		XMMATRIX mObj = mScale * mRot * mTrans;
		float alpha = atan2(it.vel.x, it.vel.z);
		XMMATRIX mAnim = XMMatrixRotationY(alpha) * XMMatrixTranslation(it.pos.x, it.pos.y, it.pos.z);
		
		g_meshWorld = mObj * mAnim;
		
		XMMATRIX meshViewProj = g_meshWorld * view * proj;
		XMMATRIX g_normalMeshWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, g_meshWorld));

		V(g_gameEffect.meshEV->SetMatrix((float*)&g_meshWorld));
		V(g_gameEffect.meshViewProjectionEV->SetMatrix((float*)&meshViewProj));
		V(g_gameEffect.meshNormalsEV->SetMatrix((float*)&g_normalMeshWorld));


		elementToRender->render(pd3dImmediateContext, 
								g_gameEffect.meshPass1, g_gameEffect.diffuseEV,
								g_gameEffect.specularEV, g_gameEffect.glowEV);

	}
	
	
	std::vector<SpriteVertex> spritesToRender(g_projectiles.size() + g_explosions.size() + g_explosionParticles.size());
	std::copy(g_projectiles.begin(), g_projectiles.end(), spritesToRender.begin());
	std::copy(g_explosions.begin(), g_explosions.end(), std::back_inserter(spritesToRender));
	std::copy(g_explosionParticles.begin(), g_explosionParticles.end(), std::back_inserter(spritesToRender));

	std::sort(spritesToRender.begin(), spritesToRender.end(),
		[](const SpriteVertex& a, const SpriteVertex& b) -> bool
	{
		return XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(&a.position), g_camera.GetWorldAhead())) >
			XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(&b.position), g_camera.GetWorldAhead()));
	});

	g_skybox->renderSkybox(pd3dImmediateContext, g_camera);
	g_spriteRenderer->renderSprites(pd3dImmediateContext, spritesToRender, g_camera);
	
	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	V(g_hud.OnRender(fElapsedTime));
	V(g_sampleUI.OnRender(fElapsedTime));
	RenderText();
	DXUT_EndPerfEvent();

	static DWORD dwTimefirst = GetTickCount();
	if (GetTickCount() - dwTimefirst > 5000)
	{
		OutputDebugString(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		OutputDebugString(L"\n");
		dwTimefirst = GetTickCount();
	}
}
