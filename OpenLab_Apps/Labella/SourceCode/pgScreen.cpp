////////////////////////////////////////////////////////////////////////////////
// pgScreen.cpp

// Includes
#include <stdio.h>
#include "pgScreen.h"
#include "UiKeyboard.h"
#include "RsUtil.h"
#include "HlScreenSize.h"
#include "BtString.h"
#include "RsMaterial.h"
#include "ApConfig.h"

////////////////////////////////////////////////////////////////////////////////
// Setup

void pgScreen::Setup( BtU32 index )
{
	BtChar archiveName[32];
	sprintf( archiveName, "screen%d", index );
	pgView::Setup( archiveName, "screen", BtTrue );
}

////////////////////////////////////////////////////////////////////////////////
// Update

void pgScreen::Update()
{
    BtU32 numItems = m_items.GetNumItems();
    
    MtVector2 v2CurrentDimension = RsUtil::GetDimension();
    
    BtFloat x, y;
    
    // Refactor these for the current viewport
    for(BtU32 i = 0; i < numItems; i++)
    {
        // Cache each item
        HlMenuItems &item = m_items[i];
        
        x = (BtFloat)item.m_v2OriginalPosition.x / item.m_v2OriginalScreenSize.x;
        y = (BtFloat)item.m_v2OriginalPosition.y / item.m_v2OriginalScreenSize.y;
        
        MtVector2 v2Position = MtVector2( x * v2CurrentDimension.x, y * v2CurrentDimension.y );
        
        if( ( v2CurrentDimension.x == 640.0f ) && ( v2CurrentDimension.y == 960.0f ) )
        {
            //v2Position.y += 8.0f;
            
            int a=0;
            a++;
        }
        
        // Refactor the position
        item.m_v2Position = v2Position;
        
        x = (BtFloat)item.m_v2OriginalDimension.x / item.m_v2OriginalScreenSize.x;
        y = (BtFloat)item.m_v2OriginalDimension.y / item.m_v2OriginalScreenSize.y;
        MtVector2 v2Dimension = MtVector2( x * v2CurrentDimension.x, y * v2CurrentDimension.y );
        
        // Refactor the size
        item.m_v2Dimension = v2Dimension;
    }
    
	pgView::Update();

    if( ApConfig::IsWin() && UiKeyboard::pInstance()->IsReleased( UiKeyCode_ESCAPE ) == BtTrue)
	{
		int a=0;
		a++;
	}
    
    for(BtU32 i = 0; i < m_items.GetNumItems(); i++)
    {
        // Cache each item
        HlMenuItems &item = m_items[i];
        
        if( item.m_isPressed )
        {
            int a=0;
            a++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Render

void pgScreen::Render()
{
    if( BtStrCompare( m_lastArchiveName, m_archiveName ) == BtFalse )
    {
        printf( "Archive name %s", m_archiveName );
        Unload();
        Load();
    }
    
    if( ApConfig::IsWin() && UiKeyboard::pInstance()->IsPressed( UiKeyCode_F2 ) )
    {
        Unload();
        Load();
    }
    
    if( m_isBackground )
    {
        MtVector2 v2Screen     = RsUtil::GetDimension();
        MtVector2 v2Background = m_pBackgroundMaterial->GetDimension();
        
        if( ApConfig::IsWin() )
        {
            //height /= 2;
        }
      
        RsColour colour( RsColour::WhiteColour() );
        m_pBackgroundMaterial->Render( MtVector2( 0, 0 ), v2Screen, colour, 6 );
    }
    
    // Render the menu
    if( 1 )//ApConfig::IsDebug() )
    {
        RenderMenu();
    }
}
