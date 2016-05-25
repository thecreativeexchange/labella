////////////////////////////////////////////////////////////////////////////////
// pgModel.h

// Include guard
#pragma once

#include "BtTypes.h"
#include "RsShader.h"

class RsModel;
class BaArchive;
class RsMaterial;


// Class definition
class pgModel
{
public:

	// Public functions
	void							Init();
    void                            Reset();
	void							Setup( BaArchive *pArchive );
	BtBool							Update( BtU32 screenIndex, BtBool isPressed, BtBool isReleased );
	void							Render( BtU32 screenIndex );

	// Accessors
	BtFloat							GetScale() { return m_scale; }
	BtFloat							GetScaleTime() { return scaleTime; }

private:

	// Private functions

	// Private members
	SgNode						   *m_pModel;
	RsShader					   *m_pShader;
	MtMatrix3						m_m3Rotation;
	BtFloat							scaleTime;
    BtFloat                         m_elapsedTime;
    BtBool                          m_isContracting;
	BtFloat							m_scale;
};
