////////////////////////////////////////////////////////////////////////////////
// pgModel.cpp

// Includes
#include "BaArchive.h"
#include "pgModel.h"
#include "MtVector2.h"
#include "MtVector3.h"
#include "RsColour.h"
#include "SgNode.h"
#include "RsShader.h"
#include "BtBase.h"
#include "BtTime.h"
#include "ShTouch.h"
#include "RsRenderTarget.h"
#include "pgRenderOrders.h"
#include "HlModel.h"
#include "UiKeyboard.h"
#include <stdio.h>
#include "HlFont.h"
#include "ApConfig.h"

static BtU32 ShowShape = 1;

////////////////////////////////////////////////////////////////////////////////
// Setup

void pgModel::Setup( BaArchive *pArchive )
{
    m_pShader = pArchive->GetShader( "shader" );
	m_pModel = pArchive->GetNode( "Outer_All" );
	
	HlModel::SetSortOrders( m_pModel, 5);//MaxSortOrders-1 );
	m_m3Rotation.SetIdentity();
    m_isContracting = BtFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Reset

void pgModel::Reset()
{
    m_isContracting = BtFalse;
    scaleTime = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Update

BtBool pgModel::Update( BtU32 screenIndex, BtBool isPressed, BtBool isReleased )
{
    if( ApConfig::IsWin() )
    {
        if( UiKeyboard::pInstance()->IsPressed( UiKeyCode_1 ) )
        {
            ShowShape = 1;
        }
        if( UiKeyboard::pInstance()->IsPressed( UiKeyCode_2 ) )
        {
            ShowShape = 2;
        }

        // Rotate the model
        BtFloat speed = 0.1f;
        if( UiKeyboard::pInstance()->IsHeld( UiKeyCode_UP ) )
        {
            MtMatrix3 m3Rotate;
            m3Rotate.SetRotationX( -speed );
            m_m3Rotation = m_m3Rotation * m3Rotate;
        }
        if( UiKeyboard::pInstance()->IsHeld( UiKeyCode_DOWN ) )
        {
            MtMatrix3 m3Rotate;
            m3Rotate.SetRotationX( speed );
            m_m3Rotation = m_m3Rotation * m3Rotate;
        }
        if( UiKeyboard::pInstance()->IsHeld( UiKeyCode_RIGHT ) )
        {
            MtMatrix3 m3Rotate;
            m3Rotate.SetRotationY( -speed );
            m_m3Rotation = m3Rotate * m_m3Rotation;
        }
        if( UiKeyboard::pInstance()->IsHeld( UiKeyCode_LEFT ) )
        {
            MtMatrix3 m3Rotate;
            m3Rotate.SetRotationY( speed );
            m_m3Rotation = m3Rotate * m_m3Rotation;
        }
    }
    
	/*
	MtMatrix4 m4Translation;
	m4Translation.SetTranslation( MtVector3( 0, -4.5f, 13.0f ) );

	// Set the rotation
	MtMatrix4 m4Transform;
    MtMatrix4 Y;
    Y.SetRotationX(MtDegreesToRadians(-20.0f));
    m4Transform = Y * m_m3Rotation * m4Translation;
	*/
	MtMatrix4 m4Rotate;
	m4Rotate.SetRotationX(MtDegreesToRadians(90.0f));
	MtMatrix4 m4Transform;
	m4Transform.SetTranslation(MtVector3(-5, 0, 10.0f));
	m_pModel->SetLocalTransform(m_m3Rotation * m4Rotate * m4Transform);
	m_pModel->Update();
    
	if( ( screenIndex == 20 ) || ( screenIndex == 21 ) )
    {
        m4Transform =  m4Transform;
		scaleTime = 0;
        m_elapsedTime = BtTime::GetElapsedTimeInSeconds();
    }
         
    const BtFloat MaxTime = 1.5f;
    
    if( ( screenIndex == 22 ) || ( screenIndex == 23 ) )
    {
		if( isPressed )
		{
            if( m_isContracting )
            {
                scaleTime -= BtTime::GetTick();
            }
            else
            {
                scaleTime += BtTime::GetTick();
            }
            
            if( scaleTime < 0 )
            {
                scaleTime = 0;
                m_isContracting = BtFalse;
                return BtTrue;
            }
            if( scaleTime > MaxTime )
            {
                scaleTime = MaxTime;
                m_isContracting = BtTrue;
            }
		}
        if( isReleased )
        {
            scaleTime = 0;
            m_isContracting = BtFalse;
		}
		m_scale = scaleTime / MaxTime;
    }
	return BtFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Render

void pgModel::Render( BtU32 screenIndex )
{
    // Set the light direction
	static BtFloat x, y, z;
	z = -0.05f;
	y =  0.05f;
	x =  0.02f;
	MtVector3 v3LightDirection( x, y, z );
	v3LightDirection.GetNormalise();

	MtVector3 ambient(0.5f, 0.5f, 0.5f);
    //MtVector3 ambient(0.3f, 0.3f, 0.3f);
	//MtVector3 ambient(0, 0, 0);
    m_pShader->SetFloats( RsHandles_Light0Direction, &v3LightDirection.x, 3 );
    
    // Set the lights ambient level
    m_pShader->SetFloats( RsHandles_LightAmbient, &ambient.x, 3 );
    
    // Apply the shader
    m_pShader->Apply();
    
    if( ( screenIndex > 1 ) && ( screenIndex <= 18 ) )
    {
        m_pModel->Render();
    }
	if( screenIndex == 20 )
	{
		m_pModel->Render();
	}
	/*
	BtChar text[32];
	sprintf( text, "Sin-angle %.2f", sinAngle );
	HlFont::RenderHeavy( MtVector2( 0, 0 ), text, MaxSortOrders-1 );
	*/
}
