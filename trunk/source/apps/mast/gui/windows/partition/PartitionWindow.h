/***
 * millipede: PartitionWindow.h
 * Copyright Stuart Golodetz, 2009. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_PARTITIONWINDOW
#define H_MILLIPEDE_PARTITIONWINDOW

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

#include <wx/frame.h>
#include <wx/glcanvas.h>
#include <wx/listctrl.h>
#include <wx/slider.h>

#include <common/dicom/volumes/DICOMVolumeChoice.h>
#include <common/slices/SliceOrientation.h>
#include <mast/models/PartitionModel.h>

//#################### FORWARD DECLARATIONS ####################
class wxButton;

namespace mp {

class DICOMCanvas;
typedef shared_ptr<class DICOMVolume> DICOMVolume_Ptr;
class PartitionCanvas;
typedef shared_ptr<class SliceTextureSet> SliceTextureSet_Ptr;

class PartitionWindow : public wxFrame, public PartitionModel::Listener
{
	//#################### TYPEDEFS ####################
private:
	typedef PartitionModel::ViewLocation ViewLocation;
	typedef shared_ptr<ViewLocation> ViewLocation_Ptr;

	//#################### PRIVATE VARIABLES ####################
private:
	int m_canvasWidth, m_canvasHeight;
	wxGLContext *m_context;
	PartitionModel_Ptr m_model;
	boost::optional<ViewLocation> m_oldViewLocation;	// the location which was being viewed before the user started scrolling a slider (empty when not scrolling)
	DICOMVolumeChoice m_volumeChoice;

	// Top right
	wxButton *m_segmentVolumeButton;

	// Middle left
	DICOMCanvas *m_dicomCanvas;
	wxSlider *m_xSlider, *m_ySlider, *m_zSlider;

	// Middle right
	PartitionCanvas *m_partitionCanvas;
	wxSlider *m_layerSlider;

	// Bottom
	wxListCtrl *m_regionInfo;

	//#################### CONSTRUCTORS ####################
public:
	PartitionWindow(wxWindow *parent, const std::string& title, const DICOMVolume_Ptr& volume, const DICOMVolumeChoice& volumeChoice, wxGLContext *context = NULL);

	//#################### PUBLIC METHODS ####################
public:
	wxGLContext *get_context() const;
	void model_changed();

	//#################### PRIVATE METHODS ####################
private:
	void calculate_canvas_size();
	bool create_dicom_textures(SliceOrientation ori);
	void create_partition_textures(SliceOrientation ori);
	bool create_textures(SliceOrientation ori);
	void refresh_canvases();
	void setup_gui(wxGLContext *context);

	//#################### EVENT HANDLERS ####################
public:
	//~~~~~~~~~~~~~~~~~~~~ BUTTONS ~~~~~~~~~~~~~~~~~~~~
	void OnButtonSegmentCTVolume(wxCommandEvent&);
	void OnButtonViewXY(wxCommandEvent&);
	void OnButtonViewXZ(wxCommandEvent&);
	void OnButtonViewYZ(wxCommandEvent&);

	//~~~~~~~~~~~~~~~~~~~~ SLIDERS ~~~~~~~~~~~~~~~~~~~~
	void OnSliderX(wxScrollEvent&);
	void OnSliderY(wxScrollEvent&);
	void OnSliderZ(wxScrollEvent&);
	void OnSliderLayer(wxScrollEvent&);

	//#################### EVENT TABLE ####################
	DECLARE_EVENT_TABLE()
};

}

#endif
