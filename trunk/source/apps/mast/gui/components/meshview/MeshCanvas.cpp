/***
 * millipede: MeshCanvas.cpp
 * Copyright Stuart Golodetz, 2010. All rights reserved.
 ***/

#include "MeshCanvas.h"

#include <GL/glu.h>

#include <wx/slider.h>

#include <common/visualization/MeshRenderer.h>
#include "MeshView.h"
#include "SphereMeshCamera.h"

namespace mp {

//#################### CONSTRUCTORS ####################
MeshCanvas::MeshCanvas(MeshView *meshView, wxGLContext *context, int *attribList, wxWindowID id, const wxPoint& pos,
					   const wxSize& size, long style)
:	Canvas(meshView, context, attribList, id, pos, size, style), m_meshView(meshView)
{}

//#################### PUBLIC METHODS ####################
void MeshCanvas::render(wxPaintDC&) const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	m_meshView->m_camera->use_as_view();

	// Render a set of axes at the origin.
	glBegin(GL_LINES);
		glColor3d(1,0,0);
		glVertex3d(0,0,0);
		glVertex3d(1,0,0);

		glColor3d(0,1,0);
		glVertex3d(0,0,0);
		glVertex3d(0,1,0);

		glColor3d(0,0,1);
		glVertex3d(0,0,0);
		glVertex3d(0,0,1);
	glEnd();

	// Translate the mesh so that the sphere camera's centre is at the origin, and then scale it in each of the {x,y,z} directions as necessary.
	// (Note that the order of the transformations appears back to front - this is normal with OpenGL.)
	const itk::Vector<double,3>& meshScale = m_meshView->m_meshScale;
	glScaled(meshScale[0], meshScale[1], meshScale[2]);
	const Vector3i& cameraCentre = m_meshView->m_sphereCamera->centre();
	glTranslated(-cameraCentre.x, -cameraCentre.y, -cameraCentre.z);

	glPushAttrib(GL_TRANSFORM_BIT);

	// Set up user-defined clip planes to clip away bits of the mesh.
	for(int i=0; i<6; ++i)
	{
		GLdouble plane[4] = {0.0,};
		double sign = i%2 ? -1.0 : 1.0;
		plane[i/2] = sign;
		plane[3] = m_meshView->m_clipSliders[i]->GetValue() * -sign;
		glClipPlane(GL_CLIP_PLANE0 + i, plane);
		glEnable(GL_CLIP_PLANE0 + i);
	}

	// Render the mesh itself.
	m_meshView->m_meshRenderer->render();

	glPopAttrib();
}

void MeshCanvas::setup()
{
	SetCurrent();

	// Set up the viewport.
	int width, height;
	GetSize(&width, &height);
	glViewport(0, 0, width, height);

	// Enable back-face culling.
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	// Set up the z-buffer.
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	// Set up the clear colour.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Set up the projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, static_cast<double>(width) / height, 1.0, 4096.0);
}

}
