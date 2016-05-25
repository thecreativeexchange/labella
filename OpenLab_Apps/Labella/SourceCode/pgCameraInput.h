////////////////////////////////////////////////////////////////////////////////
// pgCameraInput.h

// Include guard
#pragma once

#include "BtTypes.h"
#include "pgModel.h"
#include "MtVector2.h"
#include "MtMatrix4.h"

class BaArchive;
class RsMaterial;
class SgNode;
class RsShader;

// Class definition
class pgCameraInput
{
public:

	// Public functions
	void							Init();
	void							Setup( BaArchive *pArchive );
    
    void                            Reset();
    
    void                            Destroy();

	void							Start();
	void							Update();
	void							Stop();

	void							Render();
    
    // Accessors
    BtBool                          IsFound();
    
    static BtU8					   *GetMemory();
    static void                     SetData( BtU8* data, BtU32 width, BtU32 height );

private:

	// Private functions
	void                            RenderCameraSourceVideo();
    void                            RenderTriangle( MtVector2 v2A, MtVector2 v2B, MtVector2 v2C );
    
	// Private members
    static RsMaterial              *m_pMaterial;
    static RsMaterial              *m_pWhite2;
    static BtU8                    *m_pMemory;
    
    // Recognising the mark
    BtFloat                         m_foundTime;
    BtFloat                         m_lastTime;
    BtBool                          m_isFound;
    BtBool                          m_isReset;
};
