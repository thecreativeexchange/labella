////////////////////////////////////////////////////////////////////////////////
// ShMail

// Includes
#include "ShMail.h"

////////////////////////////////////////////////////////////////////////////////
// statics

//static

// Public functions
BtQueue<ShMailAction, 128> ShMail::m_actions;

////////////////////////////////////////////////////////////////////////////////
// PushAction

//static
void ShMail::PushAction( ShMailAction cameraAction )
{
    m_actions.Push( cameraAction );
}

////////////////////////////////////////////////////////////////////////////////
// GetNumItems

//static
BtU32 ShMail::GetNumItems()
{
    return m_actions.GetItemCount();
}

////////////////////////////////////////////////////////////////////////////////
// PopAction

//static
ShMailAction ShMail::PopAction()
{
    ShMailAction action;
    action = m_actions.Pop();
    return action;
}

