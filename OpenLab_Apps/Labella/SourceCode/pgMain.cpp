////////////////////////////////////////////////////////////////////////////////
// pgMain.cpp

// http://dasl.mem.drexel.edu/~noahKuntz/openCVTut7.html
// http://stackoverflow.com/questions/8239651/finger-tracking-counting-using-opencv-convex-hull-and-convexity-defect-functio

// Includes
#include <stdio.h>
#include "pgMain.h"
#include "pgRenderOrders.h"
#include "BtTime.h"
#include "ApConfig.h"
#include "RsFont.h"
#include "RsUtil.h"
#include "RsSprite.h"
#include "RsTexture.h"
#include "RsShader.h"
#include "RsCamera.h"
#include "RsMaterial.h"
#include "SgMesh.h"
#include "SgNode.h"
#include "UiKeyboard.h"
#include "RdRandom.h"
#include "MtVector2.h"
#include "HlMouse.h"
#include "HlFont.h"
#include "HlDraw.h"
#include "RsUtil.h"
#include "pgView.h"
#include "ShTouch.h"
#include "ShMail.h"
#include "ShRecorder.h"
#include "HlMaterial.h"
#include "ShCamera.h"

pgCamera pgMain::m_camera;
//static BtU32 gScreenInd = 2;
static BtU32 gScreenInd = 1;

//static BtU32 gScreenInd = 60;

static BtU32 g_lastScreenIndex = 100000;
static BtU32 pgScreen_MAX = 73;
static BtBool isLastCapture = BtFalse;
static BtBool gIsPerineum, gIsFlaps, gIsPeehole, gIsVagina, gIsClitoris, gIsAnus;
static BtBool sentEmail = BtFalse;
static BtBool sentEmail2 = BtFalse;
BtBool isRecording = BtFalse;

////////////////////////////////////////////////////////////////////////////////
// Init

void pgMain::Init()
{
	ApConfig::SetTitle("Labella");

	m_isClosing = BtFalse;
	m_isClosed  = BtFalse;

    MtVector2 v2Dimension;

	if( ApConfig::IsWin() )
	{
		v2Dimension = MtVector2( 640.0f, 1136.0f );
		v2Dimension /= 2;
		RsUtil::SetDimension( v2Dimension );
	}
    else
    {
        v2Dimension = RsUtil::GetDimension();
    }

	m_camera.Init();
	//m_camera.SetPosition(MtVector3(0, 0, -0.8f));
	m_camera.SetPosition(MtVector3(0, 0, 0));
	m_camera.SetSpeed( 50.0f );

	RdRandom::SetSeed( 123 );

	if( ApConfig::IsWin() )
	{
		ApConfig::SetDebug(BtTrue);
	}
	else
	{
		ApConfig::SetDebug(BtFalse);
	}
    
    //ApConfig::SetDebug(BtTrue);
}

////////////////////////////////////////////////////////////////////////////////
// GetCamera

RsCamera pgMain::GetCamera()
{
	return m_camera.GetCamera();
}

#include "ImageLibrary.h"

////////////////////////////////////////////////////////////////////////////////
// Setup

void pgMain::Create()
{
    
    BtChar text[256];
    sprintf(text, "%s", ApConfig::GetResourcePath() );
    strcat( text, "screen.png" );
    
    printf( "%s", text );
    
    //void *openCVImage = ImageLibrary::OpenImage( text );
    //BtU8 *pBytes = ImageLibrary::GetPixels(openCVImage);
    
    
	// Load the game archive
	if( ApConfig::GetPlatform() == ApPlatform_WinGL )
	{
		ApConfig::SetResourcePath( "..\\labella\\Release\\" );
		ApConfig::SetExtension( ".winglz" );
	}

	m_gameArchive.Load( "game" );
    
    if( ApConfig::GetDevice() == ApDevice_WIN )
    {
         m_utilityArchive.Load("win");
    }
    
    m_pCam4 = m_gameArchive.GetMaterial("cam4");
    BtFloat width  = (BtFloat)m_pCam4->Width();
    BtFloat height = (BtFloat)m_pCam4->Height();
    (void)(width);
    (void)(height);
    
	RsMaterial *pMaterial2 = m_gameArchive.GetMaterial("white2");
	m_pMaterial3 = m_gameArchive.GetMaterial("white3");
	m_pPelvicFloor  = m_gameArchive.GetMaterial("pelvicfloor");
	m_pPelvicFloor2 = m_gameArchive.GetMaterial("pelvicfloor2");
	m_pPelvicFloor3 = m_gameArchive.GetMaterial("pelvicfloor3");

	HlDraw::Setup(pMaterial2, m_pMaterial3);

	// Setup the model
	m_model.Setup( &m_gameArchive );

	// Cache the shader
	m_pShader = m_gameArchive.GetShader( "shader" );

	// Cache the sound
	m_pSound = m_gameArchive.GetSound( "howl" ); 

//	PlaySound();
	m_cameraInput.Setup( &m_gameArchive );

    if( ApConfig::GetDevice() == ApDevice_WIN )
    {
        HlMouse::Setup( &m_utilityArchive );
        HlFont::Setup( &m_utilityArchive, "vera20" );
    }
    else
    {
        HlFont::Setup( &m_gameArchive, "verabold20" );
    }
 
	for( BtU32 i=1; i<=pgScreen_MAX; i++ )
	{
		m_screen[i].Setup(i);
	}

	int a=0;
    a++;
}

////////////////////////////////////////////////////////////////////////////////
// SetData

//static
void pgMain::SetData( void *data, int width, int height )
{
}

////////////////////////////////////////////////////////////////////////////////
// Reset

void pgMain::Reset()
{
	gIsPerineum = BtFalse;
	gIsFlaps = BtFalse;
	gIsPeehole = BtFalse;
	gIsVagina = BtFalse;
	gIsClitoris = BtFalse;
	gIsAnus = BtFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Update

void pgMain::Update()
{
	// Are we closing
	if( m_isClosing == BtTrue )
	{
		// Unload the archive
		m_gameArchive.Unload();

		// Unload the archive
		m_utilityArchive.Unload();

		// Read to close
		m_isClosed = BtTrue;
	}
    else
    {
		m_camera.Update();
        
		// Update the current screen
		UpdateCurrentScreen();
    }
}

////////////////////////////////////////////////////////////////////////////////
// UpdateCurrentScreen

BtBool pgMain::IsCameraMode()
{
    if( ( gScreenInd == 68 ) || ( gScreenInd == 70 ) || ( gScreenInd == 72 ) )
    {
        return BtFalse;
    }
    
    if( (gScreenInd == 1) ||
        (gScreenInd == 19) ||
        (gScreenInd >= 30) )
    {
        return BtTrue;
    }
    return BtFalse;
}

////////////////////////////////////////////////////////////////////////////////
// UpdateCurrentScreen

void pgMain::UpdateCurrentScreen()
{
	BtU32 nextIndex = gScreenInd;

	// Count since we changed screen
	if(g_lastScreenIndex != gScreenInd)
	{
		m_elapsedTime = BtTime::GetElapsedTimeInSeconds();
		g_lastScreenIndex = gScreenInd;
	}
	m_elapsedScreenTime = BtTime::GetElapsedTimeInSeconds() - m_elapsedTime;

	// Should we capture the camera?
	BtBool isCaptureOn = BtFalse;
	if( IsCameraMode() )
	{
		isCaptureOn = BtTrue;
	}
	else
	{
		isCaptureOn = BtFalse;
	}
	if(isCaptureOn != isLastCapture)
	{
		if(isCaptureOn)
		{
			ShCamera::PushAction(ShCameraActionType_CaptureFront);
			m_cameraInput.Start();
		}
		else
		{
			ShCamera::PushAction(ShCameraActionType_CaptureStop);
			m_cameraInput.Stop();
		}
		isLastCapture = isCaptureOn;
	}

	if( isCaptureOn )
	{
		m_cameraInput.Update();
	}

    if( ApConfig::IsWin() )
    {
        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_F1) )
        {
            ApConfig::SetDebug(!ApConfig::IsDebug());
        }
        
        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_3) )
            nextIndex = 30;
        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_4) )
            nextIndex = 40;
        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_5) )
            nextIndex = 50;
    
        if(UiKeyboard::pInstance()->IsPressed(UiKeyCode_T))
        {
            if(gScreenInd > 1)
            {
                nextIndex = gScreenInd - 1;
            }
        }
        if(UiKeyboard::pInstance()->IsPressed(UiKeyCode_Y))
        {
            if(gScreenInd < pgScreen_MAX)
            {
                nextIndex = gScreenInd + 1;
            }
        }

        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_MINUS))
        {
            if(gScreenInd > 1)
            {
                nextIndex = gScreenInd - 1;

				if (nextIndex == 18)
				{
					nextIndex = 17;
				}
			}
        }
        if( UiKeyboard::pInstance()->IsPressed(UiKeyCode_PLUS))
        {
            if(gScreenInd < pgScreen_MAX)
            {
                nextIndex = gScreenInd + 1;

				if (nextIndex == 18)
				{
					nextIndex = 19;
				}
            }
        }
    }
    
	BtBool isNext = BtFalse;
	BtBool isPerineum = BtFalse;
	BtBool isFlaps = BtFalse;
	BtBool isPeehole = BtFalse;
	BtBool isVagina = BtFalse;
	BtBool isClitoris = BtFalse;
	BtBool isAnus = BtFalse;
	BtBool isPerineumHeld = BtFalse;
    BtBool isPerineumReleased = BtFalse;
    BtBool isDelete = BtFalse;

	// Cache the view
	pgView &view = m_screen[gScreenInd];

	HlMenuItems *pCurrentItem = view.GetCurrentItemSelected();
	if(pCurrentItem)
	{
		if( pCurrentItem && pCurrentItem->m_isPressed )
		{
			isNext = view.IsEqual("next") || view.IsEqual("end");
            isDelete = view.IsEqual("delete");

			isVagina = view.IsEqual("vagina");
			gIsVagina |= isVagina;

			isClitoris = view.IsEqual("clitoris");
			gIsClitoris |= isClitoris;

			isPerineum = view.IsEqual("perineum");
			gIsPerineum |= isPerineum;

			isFlaps    = view.IsEqual( "flaps1" ) | view.IsEqual( "flaps2" );
			gIsFlaps |= isFlaps;

			isAnus     = view.IsEqual( "anus" );
			gIsAnus |= isAnus;

			isPeehole  = view.IsEqual( "peehole" );
			gIsPeehole |= isPeehole;
		}
		if(pCurrentItem && pCurrentItem->m_isHeld)
		{
			isPerineumHeld = BtTrue;
		}
        if(pCurrentItem && pCurrentItem->m_isReleased)
        {
            isPerineumReleased = BtTrue;
        }
	}

	// Flag if we have seen everything
	BtBool isVisitedEverthing = gIsAnus & gIsFlaps & gIsVagina &
								gIsPeehole & gIsClitoris;

	// Update the menu screen
	m_screen[gScreenInd].Update();

	// Update the 3D model
	BtBool isContractedFully = m_model.Update(gScreenInd, isPerineumHeld, isPerineumReleased );

	// Do the per-screen logic
	if(gScreenInd == 1 )
	{
        // Start the sound recording
        if( isRecording == BtFalse )
        {
            isRecording = BtTrue;
            ShRecorderAction action;
            action.m_action = ShRecorderActionType_Record;
            strcpy( action.m_filename, "recording.aac");
            ShRecorder::PushAction(action);
        }
        
		Reset();

        if( m_cameraInput.IsFound() )
        {
            gScreenInd = 2;
            m_cameraInput.Reset();
        }
	}
    
    // Stop the sound recording
    if(gScreenInd > 2)
    {
        if( isRecording == BtTrue )
        {
            isRecording = BtFalse;
            
            ShRecorderAction action;
            action.m_action = ShRecorderActionType_Stop;
            ShRecorder::PushAction(action);
        }
    }

	if(gScreenInd == 2)
	{
		if(m_elapsedScreenTime > 5.0f)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 3)
	{
        // Send an email
        if(sentEmail == BtFalse)
        {
            ShMailAction action;
            action.m_action = ShMailActionType_Send;
            strcpy( action.m_email, "t.a.almeida@ncl.ac.uk" );
            strcpy( action.m_subject, "Really inner parts..." );
            strcpy( action.m_mimeType, "audio/caf" );
            strcpy( action.m_body, "Labella recorded this moment. Audio only. This consists of a few seconds of sound, starting from the moment you have taken the phone down there to the moment you've seen the illustration of your inner parts on the screen of your phone. This is for documentation only, and if you agree to share this audio file with the researcher please send email." );
            strcpy( action.m_filename, "record.caf" );
            ShMail::PushAction(action);
            sentEmail = BtTrue;
        }
		if(isFlaps == BtTrue)
		{
			nextIndex = 4;
		}
		if(isVagina == BtTrue)
		{
			nextIndex = 8;
		}
		if(isPeehole == BtTrue)
		{
			nextIndex = 10;
		}
		if(isClitoris == BtTrue)
		{
			nextIndex = 13;
		}
		if(isAnus == BtTrue)
		{
			nextIndex = 16;
		}
	}
	if( gScreenInd == 4)
	{
		if( isNext == BtTrue )
			nextIndex++;
	}
	if(gScreenInd == 5)
	{
		if(isNext == BtTrue)
			nextIndex++;
	}
	if(gScreenInd == 6)
	{
		if(isNext == BtTrue)
			nextIndex++;
	}
	if(gScreenInd == 7)
	{
		if(isNext == BtTrue)
		{
			if(isVisitedEverthing)
				nextIndex = 19;
			else
				nextIndex = 3;
		}
	}
	if(gScreenInd == 8)
	{
		if(isNext == BtTrue)
			nextIndex++;
	}
	if(gScreenInd == 9)
	{
		if(isNext == BtTrue)
		{
			if( isVisitedEverthing )
				nextIndex = 19;
			else
				nextIndex = 3;
		}
	}
	if(gScreenInd == 10)		// peehole explanation 1
	{
		if(isNext == BtTrue)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 11)		// peehole explanation 2
	{
		if(isNext == BtTrue)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 12)		// peehole explanation 3
	{
		if(isNext == BtTrue)
		{
			if( isVisitedEverthing )
				nextIndex = 19;
			else
				nextIndex = 3;
		}
	}
	if(gScreenInd == 13)		// clitoris explanation 1
	{
		if(isNext == BtTrue)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 14)		// clitoris explanation 2
	{
		if(isNext == BtTrue)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 15)		// clitoris explanation 3
	{
		if(isNext == BtTrue)
		{
			if( isVisitedEverthing )
				nextIndex = 19;
			else
				nextIndex = 3;
		}
	}
	if(gScreenInd == 16)
	{
		if(isNext == BtTrue)
			nextIndex++;
	}
	if(gScreenInd == 17)
	{
		if(isNext == BtTrue)
		{
			if( isVisitedEverthing )
            {
				nextIndex = 19;
            }
			else
				nextIndex = 3;
		}
	}
    // Screen 18 is deleted
	if(gScreenInd == 19)
	{
        if( m_cameraInput.IsFound() )
        {
            nextIndex = 20;
            m_cameraInput.Reset();
        }
	}
	if(gScreenInd == 20)
	{
		if(isPerineum == BtTrue)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 21)                                            // Show image and then show text
	{
		if(m_elapsedScreenTime > 5.0f)
		{
			nextIndex++;
		}
	}
	if( gScreenInd == 22 )
	{
		if( isContractedFully )
		{
            // One extra second here
			nextIndex++;
		}
	}
	if(gScreenInd == 23)
	{
        // Don't show the Perineum message straight away             Show image and then show text
        if(m_elapsedScreenTime > 5.0f)
        {
            nextIndex++;
        }
	}
	if(gScreenInd == 24)
	{
		if(m_elapsedScreenTime > 4.0f)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 25)
	{
		if(m_elapsedScreenTime > 2.0f)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 26)
	{
		if(m_elapsedScreenTime > 2.0f)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 27)
	{
        if(m_elapsedScreenTime > 2.0f)
        {
            nextIndex++;
        }
	}
	if(gScreenInd == 28)
	{
		if(isNext)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 29)
	{
		if(isNext)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 30)                    // Switch to camera mode caption
	{
		if(m_elapsedScreenTime > 2.0f)
		{
			nextIndex++;
		}
	}
	if(gScreenInd == 31)
	{
		if(m_elapsedScreenTime > 2.0f)
		{
			nextIndex++;
		}
	}
	if( gScreenInd == 32 )
	{
		if(m_elapsedScreenTime > 2.0f)
			nextIndex++;
	}
	if(gScreenInd == 33 )
	{
		if(isNext)
		{
			nextIndex++;
		}
	}
	if( gScreenInd == 34 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 35 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 36 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 37 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 38 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 39 )					// Now relax for 4 seconds
	{										// 1 second apart
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 40 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 41 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 42 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 43 )					// how did you go?
	{
		if(isNext)
		{
			nextIndex++;
		}
	}
	if( gScreenInd == 44 )					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 45)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 46)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 47)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 48)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 49)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 50)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 51)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 52)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 53)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 53)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 54)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 55)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 56)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 57)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 57)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}	
	if(gScreenInd == 58)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if(gScreenInd == 59)					// how did you go?
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 60 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 61 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 62 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 63 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 64 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 65 )					// 1 second apart
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
	if( gScreenInd == 66 )                  // Show camera video source
	{
		if(m_elapsedScreenTime > 1.0f)
			nextIndex++;
	}
    if( gScreenInd == 67 )					// 52  Takes photo here
    {
        if(isNext)
        {
            nextIndex++;
        }
    }
    if( gScreenInd == 68 )
    {
        if(isNext)
        {
            nextIndex = 70;					// 54
        }
        if(isDelete)
        {
            nextIndex = 69;					// 53
        }
    }
    if( gScreenInd == 69 )                  // 69   Takes photo here
    {
        if(isNext)
        {
            // Save this to the camera roll
            ShCameraAction action;
            action.m_action = ShCameraActionType_SaveToAlbums;
            action.m_pMemory = m_pCam4->GetTexture(0)->ReadMemory();
            ShCamera::PushAction(action);
            
            nextIndex++;
        }
    }
    if( gScreenInd == 70 )					// 1 second apart
    {
        if(isNext)
        {
            nextIndex++;
        }
    }
    if( gScreenInd == 71 )					// 1 second apart
    {
        if(isNext)
        {
            nextIndex++;
        }
    }
    if( gScreenInd == 72 )					// 1 second apart
    {
        if(isNext)
        {
            // Send an email
            if(sentEmail2 == BtFalse)
            {
                ShMailAction action;
                action.m_action = ShMailActionType_Send;
                strcpy( action.m_mimeType, "image/png" );
                strcpy( action.m_email, "t.a.almeida@ncl.ac.uk" );
                strcpy( action.m_subject, "Squeeze, release" );
                strcpy( action.m_body, "This email will send the photo you have taken previously to the researcher. Whereas the photo might be of you exercising your pelvic floor muscles, exercising them should not show at all 'on the outside'. It is important that you should not pull in your tummy, tighten your buttocks excessively, nor hold your breath. This is for documentation only, and if you agree to share this image file with the researcher please send email." );
                strcpy( action.m_filename, "portrait.png" );
                ShMail::PushAction(action);
                sentEmail2 = BtTrue;
            }
            
            nextIndex++;
        }
    }
    if( gScreenInd == 73 )					// 1 second apart
    {
        // nothing happens here
    }

    if( ApConfig::IsDebug() )
    {
        for(BtU32 i = 0; i < MaxTouches; i++)
        {
            if(ShTouch::IsPressed(i))
            {
                MtVector2 v2Position = ShTouch::GetPosition(i);
                
                if(v2Position.y < RsUtil::GetHalfDimension().y / 5.0f)
                {
                    ApConfig::SetDebug(BtTrue);
                    gIsPerineum = gIsFlaps = gIsPeehole = gIsVagina = gIsClitoris = gIsAnus = BtFalse;
                    sentEmail = BtFalse;
                    sentEmail2 = BtFalse;
                    m_cameraInput.Reset();
                    m_model.Reset();
                    
                    if(v2Position.x > RsUtil::GetHalfDimension().x)
                    {
                        if(gScreenInd < pgScreen_MAX)
                        {
                            nextIndex++;
                            
                            if( nextIndex == 18 )
                            {
                                nextIndex = 19;
                            }
                        }
                    }
                    else
                    {
                        if(gScreenInd > 1)
                        {
                            nextIndex--;
                            
                            if( nextIndex == 18 )
                            {
                                nextIndex = 17;
                            }
                        }
                    }
                }
            }
        }
    }
	gScreenInd = nextIndex;
}

////////////////////////////////////////////////////////////////////////////////
// SetupScreen

void pgMain::SetupScreen()
{
	RsUtil::EmptyRenderTargets();

	// Apply the shader
	m_pShader->Apply();

	// Make a new render target
	RsRenderTarget *pRenderTarget;

	// Make a new render target
	pRenderTarget = RsUtil::GetNewRenderTarget();

	// Set a default camera for all the render targets
	pRenderTarget->SetCamera( m_camera.GetCamera() );

	// Clear the render target
	pRenderTarget->SetCleared( BtTrue );

	// Clear the z buffer
	pRenderTarget->SetZCleared( BtTrue );

	// Set the colour
	//pRenderTarget->SetClearColour( RsColour( 0.1f, 0.4f, 0.6f, 1.0f ) );
    pRenderTarget->SetClearColour( RsColour::WhiteColour() );

	// Apply this render target
	pRenderTarget->Apply();
}

BtFloat GetDepth( RsMaterial *pMaterial, BtFloat ratioX, BtFloat ratioY )
{
	if( ratioX >= 1.0f )
	{
		return 0;
	}
	if( ratioY >= 1.0f)
	{
		return 0;
	}

	BtU8 r, g, b, a;
	BtFloat x = ratioX * pMaterial->GetTexture(0)->GetWidth();
	BtFloat y = ratioY * pMaterial->GetTexture(0)->GetHeight();

	pMaterial->GetTexture(0)->GetPixel(x, y, r, g, b, a);

	BtFloat depth = r / 255.0f;
	return depth * 4.0f;
}

////////////////////////////////////////////////////////////////////////////////
// RenderPelvicFloor

void pgMain::RenderPelvicFloor( RsMaterial *pMaterial )
{
	RsCamera camera = RsRenderTarget::GetCurrent()->GetCamera();
	//camera.SetRotation(MtMatrix3::GetIdentity());

	BtFloat aspect = camera.GetAspect();

	// This code perfectly frames a billboard in front of the camera
	BtFloat completeFieldOfView = camera.GetFieldOfView();
	MtMatrix3 m3Orientation = camera.GetRotation().GetInverse();
	MtVector3 v3AxisZ = m3Orientation.ZAxis();
	MtVector3 v3AxisY = m3Orientation.YAxis();
	MtVector3 v3AxisX = m3Orientation.XAxis() * 1.1f;

	BtFloat fieldOfView = completeFieldOfView * 0.5f;
	BtFloat tanAngle = MtTan(fieldOfView);
	BtFloat distance = 0.5f / tanAngle;

    if( ApConfig::IsPhone() )
    {
        distance = distance * 3.9f;
    }
    else
    {
        distance = distance * 2.9f;
    }
    
	camera.SetPosition(MtVector3(0, 0, 0));
	camera.Update();

	MtVector3 v3Position = camera.GetPosition() + (v3AxisZ * distance);
	//

	const BtU32 GridSize = 48;
	const BtU32 GridSizePlus1 = GridSize + 1;
	MtVector2 positionLookup[GridSizePlus1][GridSizePlus1];
	MtVector2 uvLookup[GridSizePlus1][GridSizePlus1];

	BtFloat normaliseGrid = 1.0f / (BtFloat)GridSize;

	// 0      1     2     3     4
	// 0   0.25   0.5  0.75   1.0
	for (BtU32 i = 0; i <= GridSize; i++)
	{
		for (BtU32 j = 0; j <= GridSize; j++)
		{
			uvLookup[i][j] = MtVector2((BtFloat)i * normaliseGrid, (BtFloat)j * normaliseGrid);

			// 0 .. 1
			// - 0.5
			// -0.5 to 0.5
			positionLookup[i][j] = uvLookup[i][j] - MtVector2(0.5f, 0.5f);
			positionLookup[i][j] *= 2.0f;

			// Flip U(V)
			uvLookup[i][j].y = 1.0f - uvLookup[i][j].y;
			positionLookup[i][j].y -= 0.10f;
		}
	}
	
	BtFloat u, v, w;
	MtVector3 v3PosX, v3PosY;
	BtU32 colour = RsColour::WhiteColour().asWord();
//	BtU32 colour = RsColour::RedColour().asWord();

	for (BtU32 i = 0; i < GridSize; i++)
	{
		RsVertex3 v3Vertex[GridSize * 2];

		BtU32 numVertex = 0;

		for (BtU32 j = 0; j < GridSize; j++ )
		{
			v3PosX = positionLookup[i][j].x;
			v3PosY = positionLookup[i][j].y / aspect;
			u = uvLookup[i][j].x;
			v = uvLookup[i][j].y;
			w = GetDepth(m_pPelvicFloor2, u, v) * m_model.GetScale();

			v3Vertex[numVertex].m_v3Position = v3Position + (v3AxisX * v3PosX.x) + (v3AxisY * v3PosY.y) + (v3AxisZ * w);
			v3Vertex[numVertex].m_v2UV.x = u;
			v3Vertex[numVertex].m_v2UV.y = v;
			v3Vertex[numVertex].m_colour = colour;
			v3Vertex[numVertex].m_v3Normal = MtVector3(0, 1, 0);
			++numVertex;

			v3PosX = positionLookup[i+1][j].x;
			v3PosY = positionLookup[i+1][j].y / aspect;
			u = uvLookup[i+1][j].x;
			v = uvLookup[i+1][j].y;
			w = GetDepth(m_pPelvicFloor2, u, v) * m_model.GetScale();

			v3Vertex[numVertex].m_v3Position = v3Position + (v3AxisX * v3PosX.x) + (v3AxisY * v3PosY.y) + (v3AxisZ * w);
			v3Vertex[numVertex].m_v2UV.x = u;
			v3Vertex[numVertex].m_v2UV.y = v;
			v3Vertex[numVertex].m_colour = colour;
			v3Vertex[numVertex].m_v3Normal = MtVector3(0, 1, 0);
			++numVertex;
		}
		pMaterial->Render(RsPT_TriangleStrip, v3Vertex, numVertex, 5, BtTrue);
//		m_pMaterial3->Render(RsPT_TriangleStrip, v3Vertex, numVertex, 5, BtTrue);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render

void pgMain::Render()
{
	if( m_isClosing == BtTrue )
	{
		return;
	}

	SetupScreen();

    static int temp = 0;
    
    // Render the camera input
	if( IsCameraMode() )
	{
        ++temp;
        if( temp > 5 )
            m_cameraInput.Render();
	}
    else
    {
        temp = 0;
    }
    
    // Set all the items to visible if it's the debug build
    {
        BtU32 numItems = m_screen[gScreenInd].GetNumItems();
        
        for( BtU32 i=0; i<numItems; i++ )
        {
            m_screen[gScreenInd].SetVisible( i, ApConfig::IsDebug() );
        }
    }
    
	// Render the screen
	m_screen[gScreenInd].Render();

    if( gScreenInd == 68 )
    {
        MtVector2 v2ScreenDimension = RsRenderTarget::GetCurrent()->GetCameraDimension();
        MtVector2 v2Centre = v2ScreenDimension * 0.5f;
        MtVector2 v2Quart  = v2Centre * 0.5f;
        MtVector2 v2Position = v2Centre - v2Quart;
        HlMaterial::RenderSideWays( m_pCam4, v2Position, v2Centre, v2ScreenDimension );
    }
    if( gScreenInd == 70 )
    {
        MtVector2 v2ScreenDimension = RsRenderTarget::GetCurrent()->GetCameraDimension();
        MtVector2 v2Centre = v2ScreenDimension * 0.5f;
        MtVector2 v2Quart  = v2Centre * 0.5f;
        MtVector2 v2Position = v2Centre - ( v2Quart * 0.5f );
        v2Position.y += v2Quart.y;
        HlMaterial::RenderSideWays( m_pCam4, v2Position, v2Quart, v2ScreenDimension );
    }
    
	if (1)
	{
		// Render the model
		//m_model.Render(gScreenInd);

		if ((gScreenInd >= 2) && (gScreenInd < 19))
		{
			RenderPelvicFloor( m_pPelvicFloor3 );
		}

		if( (gScreenInd >= 20 ) && (gScreenInd < 25 ) )
		{
			RenderPelvicFloor( m_pPelvicFloor );
		}
	}

	// Render the game
	RenderFont();

    if( ApConfig::GetDevice() == ApDevice_WIN )
    {
        // Render the mouse
        HlMouse::Render();
    }
}

////////////////////////////////////////////////////////////////////////////////
// RenderFont

void pgMain::RenderFont()
{
	MtVector2 v2TouchPosition = ShTouch::GetPosition(0);
    
	MtVector3 v3Position = m_camera.GetCamera().GetPosition();
    
    if( ApConfig::IsDebug() )
    {
		BtFloat scale = m_model.GetScale();
	    BtChar text[256];
        sprintf( text, "S%d %.2f %.2f", gScreenInd, m_model.GetScaleTime(), scale );
        HlFont::Render( MtVector2( 10, 10 ), text, RsColour::BlackColour(), MaxSortOrders-1 );
    }
}

////////////////////////////////////////////////////////////////////////////////
// Destroy

void pgMain::Destroy()
{
	for( BtU32 i=1; i<=pgScreen_MAX; i++ )
	{
		m_screen[i].Unload();
	}
	m_cameraInput.Destroy();
}
