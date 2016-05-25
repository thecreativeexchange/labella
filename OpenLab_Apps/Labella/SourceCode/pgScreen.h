////////////////////////////////////////////////////////////////////////////////
// pgScreen1.h

// Include guard
#pragma once

#include "pgView.h"

// Class definition
class pgScreen : public pgView
{
public:

	// Public functions
	void							Setup( BtU32 screenIndex );
	void							Update();
	void							Render();

	// Accessors

private:

	// Private functions

	// Private members
};
