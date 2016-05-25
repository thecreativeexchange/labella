////////////////////////////////////////////////////////////////////////////////
// pgCameraInput.cpp

// Includes

#include "RsVertex.h"
#include "RsTexture.h"
#include "BaArchive.h"
#include "pgCameraInput.h"
#include "RsMaterial.h"
#include "SgNode.h"
#include "ShTouch.h"
#include "pgRenderOrders.h"
#include "RsUtil.h"
#include "BtMemory.h"
#include "ApConfig.h"
#include "VideoCapture.h"
#include "ImageLibrary.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "HlFont.h"
#include "HlDraw.h"
#include "MtMatrix4.h"
#include "pgMain.h"
#include "UiKeyboard.h"
#include "HlModel.h"
#include <vector>
#include "BtArray.h"
#include "BtTime.h"

//static
RsMaterial *pgCameraInput::m_pMaterial;
RsMaterial *pgCameraInput::m_pWhite2;
BtU8 *pgCameraInput::m_pMemory;
BtU32 numContours = 0;
const BtU32 sides = 4;

struct pgShape
{
	MtVector2 m_v2[4];
	MtVector2 m_v2Centre;
	BtFloat   m_match;
};

const BtU32 MaxShapes = 128;
BtArray<pgShape, MaxShapes> g_shapes;
BtArray< pgShape, MaxShapes> validShapes;

const BtFloat FoundTime = 2.0f;

////////////////////////////////////////////////////////////////////////////////
// Setup

void pgCameraInput::Setup( BaArchive *pArchive )
{
    //m_pMaterial = pArchive->GetMaterial( "cam3" );
	m_pMaterial = pArchive->GetMaterial( "cam4" );
    m_pWhite2 = pArchive->GetMaterial( "white2" );
	m_pMemory = BtMemory::Allocate( 640 * 480 * 4 );
    
    Reset();
}

////////////////////////////////////////////////////////////////////////////////
// Reset

void pgCameraInput::Reset()
{
    m_foundTime = 0;
    m_isFound = BtFalse;
    m_lastTime = BtTime::GetElapsedTimeInSeconds();
    m_isReset = BtTrue;
    BtU32 size = 640 * 480 * 4;
    BtMemory::Set( m_pMemory, 0, size );
    g_shapes.Empty();
    validShapes.Empty();
    
    // For image rendering
    RsTexture *pTexture = m_pMaterial->GetTexture(0);
    pTexture->WriteMemory( m_pMemory, size );
}

////////////////////////////////////////////////////////////////////////////////
// GetMemory

BtU8 *pgCameraInput::GetMemory()
{
	return m_pMemory;
}

////////////////////////////////////////////////////////////////////////////////
// IsFound

BtBool pgCameraInput::IsFound()
{
    return m_isFound;
}

////////////////////////////////////////////////////////////////////////////////
// Destroy

void pgCameraInput::Destroy()
{
    CVVideoCapture::Stop();
    
    BtMemory::Free( m_pMemory );
}

////////////////////////////////////////////////////////////////////////////////
// SetData

//static
void pgCameraInput::SetData( BtU8* data, BtU32 width, BtU32 height )
{
    BtU32 size = width * height * 4;
    BtMemory::Copy( m_pMemory, data, size );
    
    int a=0;
    a++;
}

////////////////////////////////////////////////////////////////////////////////
// Start

void pgCameraInput::Start()
{
	CVVideoCapture::Start();
}

////////////////////////////////////////////////////////////////////////////////
// Stop

void pgCameraInput::Stop()
{
	CVVideoCapture::Stop();
}

////////////////////////////////////////////////////////////////////////////////
// Update

void pgCameraInput::Update()
{
    if( m_isReset )
    {
        m_isReset = BtFalse;
        m_lastTime = BtTime::GetElapsedTimeInSeconds();
    }
    {
		if( ApConfig::GetDevice() == ApDevice_WIN )
		{
			// Update the video capture
			CVVideoCapture::Update();
        
			// Set's the same data that has been set from the mobile phone source video capture
			SetData( CVVideoCapture::GetMemory(), 640, 480 );
		}

		cv::Mat originalCameraImage = cv::Mat( 480, 640, CV_8UC4, m_pMemory);
        CvSeq* contours = 0;  //hold the pointer to a contour in the memory block
        CvSeq* result;   //hold sequence of points of a contour
        CvMemStorage *storage = cvCreateMemStorage(0); //storage area for all contours
        
		CvSize imageSize = cvSize( 640, 480 );
		IplImage* imgGrayScale = cvCreateImage( imageSize, 8, 1);
		IplImage* imgBin = cvCreateImage( imageSize, 8, 1);

		IplImage dst_img = originalCameraImage;
		cvCvtColor( &dst_img, imgGrayScale, CV_BGRA2GRAY);

        // Sample the grey scale image to a threshold
		cvThreshold( imgGrayScale, imgBin, 80, 255, CV_THRESH_BINARY);
        
        // Erode the image
        IplImage* erodeImg=cvCloneImage( imgBin );
        cvErode(imgBin,erodeImg,NULL,4);
        
        // For image rendering
		RsTexture *pTexture = m_pMaterial->GetTexture(0);
        pTexture->WriteMemory( (BtU8*)dst_img.imageData, 640 * 480 * 4 );
        
        // Find the contours
        cvFindContours( erodeImg, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

		g_shapes.Empty();
		numContours = 0;

		//iterating through each contour
		while(contours)
		{
			//obtain a sequence of points of contour, pointed by the variable 'contour'
			result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);
   
			if( result->total == sides )
			{
				//iterating through each point
				CvPoint *pt[sides];
				for(int i=0;i<sides;i++)
				{
					 pt[i] = (CvPoint*)cvGetSeqElem( result, i );
				}

				pgShape shape;

				for( BtU32 i=0; i<sides; i++ )
				{
					shape.m_v2[i] = MtVector2( pt[i]->x, pt[i]->y );
					shape.m_v2[i].x = 640.0f - shape.m_v2[i].x;
					shape.m_v2[i].x = shape.m_v2[i].x / 640.0f;
                    shape.m_v2[i].y = 480.0f - shape.m_v2[i].y;
					shape.m_v2[i].y = shape.m_v2[i].y / 480.0f;

					if( ApConfig::GetDevice() == ApDevice_iPhone )
					{
						BtSwap( shape.m_v2[i].x, shape.m_v2[i].y );
						shape.m_v2[i].x = 1.0f - shape.m_v2[i].x;
						shape.m_v2[i].y = 1.0f - shape.m_v2[i].y;
					}
            
					shape.m_v2[i].x *= (BtFloat)RsUtil::GetWidth();
					shape.m_v2[i].y *= (BtFloat)RsUtil::GetHeight();
				}
            
				g_shapes.Add( shape );
			}

			++numContours;
		
			// obtain the next contour
			contours = contours->h_next; 
		}

		int a=0;
		a++;

        cvReleaseImage( &erodeImg );
		cvReleaseMemStorage( &storage );
		cvReleaseImage(&imgGrayScale);
		cvReleaseImage(&imgBin);
	}
}

////////////////////////////////////////////////////////////////////////////////
// RenderCameraSourceVideo

void pgCameraInput::RenderCameraSourceVideo()
{
	// Allocate vertex
	RsVertex3 vertex[4];
    
	RsVertex3 *pVertex = &vertex[0];
    
    // Calculate the positions
	BtFloat minU = 0;
	BtFloat minV = 0;
    
	BtFloat maxU = 1.0f;
	BtFloat maxV = 1.0f;
    
    RsVertex3 *pQuad = pVertex;
    
    BtFloat xmax = (BtFloat)RsUtil::GetWidth();
    BtFloat ymax = (BtFloat)RsUtil::GetHeight();
    
	// Copy these into vertex
	pVertex->m_v3Position = MtVector3( 0, 0, 0.1f );
	pVertex->m_v2UV = MtVector2( minV, minU  );
	pVertex->m_colour = 0xFFFFFFFF;
	++pVertex;
    
	pVertex->m_v3Position = MtVector3( 0, ymax, 0.1f );
	pVertex->m_v2UV = MtVector2( minV, maxU );
	pVertex->m_colour = 0xFFFFFFFF;
	++pVertex;
    
	pVertex->m_v3Position = MtVector3( xmax, 0, 0.1f );
	pVertex->m_v2UV = MtVector2( maxV, minU  );
	pVertex->m_colour = 0xFFFFFFFF;
	++pVertex;
    
	pVertex->m_v3Position = MtVector3( xmax, ymax, 0.1f );
	pVertex->m_v2UV = MtVector2( maxV, maxU );
	pVertex->m_colour = 0xFFFFFFFF;
	++pVertex;

    // Scale to the viewport
    xmax = (BtFloat)RsUtil::GetWidth();
    ymax = (BtFloat)RsUtil::GetHeight();
    
    // Cache the display width and height
	BtFloat fScaleWidth  = 1.0f / xmax;
	BtFloat fScaleHeight = 1.0f / ymax;
    
    // Scale the position to local screen space -1 to 1
	for( BtU32 i=0; i<4; i++ )
	{
        if( ApConfig::GetDevice() == ApDevice_iPhone )
        {
            BtSwap( pQuad[i].m_v2UV.x, pQuad[i].m_v2UV.y );
        }
        if( ApConfig::GetDevice() == ApDevice_iPad )
        {
            BtSwap( pQuad[i].m_v2UV.x, pQuad[i].m_v2UV.y );
        }
        
        // 0..width to 0..1
		pQuad[ i ].m_v3Position.x *= fScaleWidth;
		pQuad[ i ].m_v3Position.y *= fScaleHeight;
        
        pQuad[ i ].m_v3Position.y = 1.0f - pQuad[ i ].m_v3Position.y;
        
		// Scale from 0..1 to 0..2
		pQuad[ i ].m_v3Position.x *= 2.0f;
		pQuad[ i ].m_v3Position.y *= 2.0f;
        
		// Translate from 0..2 to -1..1
		pQuad[ i ].m_v3Position.x -= 1.0f;
		pQuad[ i ].m_v3Position.y -= 1.0f;
	}
    m_pMaterial->Render( RsPT_TriangleStrip, pQuad, 4, 0 );
    //m_pWhite2->Render( RsPT_TriangleStrip, pQuad, 4, 0 );
}

////////////////////////////////////////////////////////////////////////////////
// RenderTriangle

void pgCameraInput::RenderTriangle( MtVector2 v2A, MtVector2 v2B, MtVector2 v2C )
{
    static int count = 0;
    --count;
    
    if( count < -2 )
    {
        count = 0;
        return;
    }
    RsVertex2 vertex[3];
    
    vertex[ 0 ].m_v2Position = v2A;
    vertex[ 1 ].m_v2Position = v2B;
    vertex[ 2 ].m_v2Position = v2C;
 
    RsColour lightGreen( 0, 1, 0, 1.0f );
    
    BtU32 col = lightGreen.asWord();
    
    // Scale the position to local screen space -1 to 1
    for( BtU32 i=0; i<3; i++ )
    {
        // Set the colour
        vertex[ i ].m_colour = col;
    }
    
    m_pWhite2->Render( RsPT_TriangleList, vertex, 3, MaxSortOrders-1 );
    BtSwap( vertex[0], vertex[2] );
    m_pWhite2->Render( RsPT_TriangleList, vertex, 3, MaxSortOrders-1 );
}

////////////////////////////////////////////////////////////////////////////////
// Render

void pgCameraInput::Render()
{
	// Now process the captured shapes
    RenderCameraSourceVideo();

	BtU32 numShapes = g_shapes.GetNumItems();

	for( BtU32 i=0; i<numShapes; i++ )
	{
		pgShape &shape = g_shapes[i];

        shape.m_match = 0;
        
		MtVector2 v2Centre = shape.m_v2[0] + shape.m_v2[1] + shape.m_v2[2] + shape.m_v2[3];
		v2Centre = v2Centre * 0.25f;
      
        BtFloat from = RsUtil::GetHalfDimension().x / 10.0f;
        
        if( v2Centre.GetLength() < 100 )
        {
            int a=0;
            a++;
            continue;
        }
        
        MtVector2 v2ScreenCentre = RsUtil::GetHalfDimension();
        
        BtFloat length1 = ( shape.m_v2[0] - v2Centre ).GetLength();
        BtFloat length2 = ( shape.m_v2[1] - v2Centre ).GetLength();
        BtFloat length3 = ( shape.m_v2[2] - v2Centre ).GetLength();
        BtFloat length4 = ( shape.m_v2[3] - v2Centre ).GetLength();
        
        // If it's too long and thin remove it
        if( ( length1 > v2ScreenCentre.x ) || ( length2 > v2ScreenCentre.x ) ||
            ( length3 > v2ScreenCentre.x ) || ( length4 > v2ScreenCentre.x ) )
        {
            continue;
        }
        
        BtFloat minDistance = (shape.m_v2[0] - shape.m_v2[1]).GetLength();
        
        for( BtU32 i=0; i<4; i++)
        {
            for( BtU32 j=0; j<4; j++)
            {
                if( i != j )
                {
                    MtVector2 v2Delta = shape.m_v2[i] - shape.m_v2[j];
                    BtFloat diff = v2Delta.GetLength();
                    minDistance = MtMin( minDistance, diff );
                }
            }
        }
        if( minDistance < from )
        {
            continue;
        }
        
        MtVector2 v2Delta = v2ScreenCentre - v2Centre;
        BtFloat dist = v2Delta.GetLength();
        
        if( dist < from * 5)
        {
            shape.m_match += 1.0f;
        }
        if( dist < from * 4)
        {
            shape.m_match += 1.0f;
        }
        if( dist < from * 3)
        {
            shape.m_match += 1.0f;
        }
        if( dist < from * 2)
        {
            shape.m_match += 1.0f;
        }
        if( dist < from)
        {
            shape.m_match += 1.0f;
        }
	}

    BtBool isOk = BtFalse;
    
    for( BtU32 i=0; i<numShapes; i++ )
    {
        if( g_shapes[i].m_match > 0 )
        {
            isOk = BtTrue;
        }
    }
    
    //if( isOk == BtTrue )
    {
        validShapes.Empty();
        
        for( BtU32 i=0; i<numShapes; i++ )
        {
            if( g_shapes[i].m_match > 0 )
            {
                validShapes.Add( g_shapes[i] );
            }
        }
    }
    
    BtU32 numValidShapes = validShapes.GetNumItems();

	for( BtU32 i=0; i<numValidShapes; i++ )
	{
		for( BtU32 j=0; j<numValidShapes; j++ )
		{
			if( validShapes[i].m_match > validShapes[j].m_match )
			{
				BtSwap( validShapes[i], validShapes[j] );
			}
		}
	}
    
    BtFloat tick  = BtTime::GetElapsedTimeInSeconds() - m_lastTime;
    m_lastTime = BtTime::GetElapsedTimeInSeconds();

	if( numValidShapes )
	{
        for( BtU32 s = 0; s<numValidShapes; s++ )
        {
            pgShape &shape = validShapes[s];

            for( BtU32 i=0; i<sides; i++ )
            {
                for( BtU32 j=0; j<sides; j++ )
                {
                    if( i != j )
                    {
                        HlDraw::RenderLine( shape.m_v2[i], shape.m_v2[j], RsColour::GreenColour() );
                    }
                }
            }
            break;
        }
        
        // Capture mark
        m_foundTime += tick;
        
        if( m_foundTime > FoundTime )
        {
            m_isFound = BtTrue;
        }
	}
    else
    {
        m_isFound = BtFalse;
        m_foundTime -= tick;
        
        if( m_foundTime < 0 )
        {
            m_foundTime = 0;
        }
    }
}


