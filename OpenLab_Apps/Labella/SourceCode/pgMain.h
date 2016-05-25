////////////////////////////////////////////////////////////////////////////////
// pgMain.h

// Include guard
#pragma once

#include "BtTypes.h"
#include "BaArchive.h"
#include "MtVector3.h"
#include "pgCamera.h"
#include "SdSound.h"
#include "pgCameraInput.h"
#include "GaProject.h"
#include <vector>

// Test classes
#include "pgScreen.h"

class pgMain;
class RsTexture;
class RsShader;
class RsMaterial;
class SgNode;

// Class definition
class pgMain : public GaProject
{
public:

	// Public functions
	void							Init();
	void							Reset();
	void							Create();
	void							Update();
	void							Render();
	void							Destroy();
    void                            ProcessPhoto();

	void							UpdateDebug();
    
	// Accessors
	void							SetClosing();
	BtBool							IsClosed();
	BtBool							IsClosing();
    
    static void                     SetData( void *data, int width, int height );

	static RsCamera					GetCamera();
    BtBool                          IsCameraMode();

private:
    
    // Private functions
	void							UpdateCurrentScreen();
	void                            RenderVideo();
    void                            RenderScreen();
	void							RenderPelvicFloor(RsMaterial *pModel);
    
	void							SetupScreen();
	void							RenderFont();

	// Private members
	BaArchive						m_gameArchive;
	BaArchive						m_utilityArchive;
	
	// Rendering

	// Resources
	RsShader					   *m_pShader;
	SdSound						   *m_pSound;
    
    RsMaterial                      *m_pCam4;
	RsMaterial						*m_pMaterial3;
	RsMaterial						*m_pPelvicFloor;
	RsMaterial						*m_pPelvicFloor2;
	RsMaterial						*m_pPelvicFloor3;

	pgModel							m_model;

	static pgCamera					m_camera;

	pgScreen						m_screen[256];
	
	pgCameraInput					m_cameraInput;
	MtMatrix3						m_m3Rotation;
	BtFloat							m_elapsedTime;
	BtFloat							m_elapsedScreenTime;
};

