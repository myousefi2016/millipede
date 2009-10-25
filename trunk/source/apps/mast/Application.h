/***
 * mast: Application.h
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#ifndef H_MAST_APPLICATION
#define H_MAST_APPLICATION

#include <wx/wx.h>

namespace mp {

class Application : public wxApp
{
	//#################### EVENT HANDLERS ####################
public:
	bool OnInit();
};

DECLARE_APP(Application)
IMPLEMENT_APP(Application)

}

#endif