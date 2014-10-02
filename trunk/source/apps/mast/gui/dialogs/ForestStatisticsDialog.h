/***
 * millipede: ForestStatisticsDialog.h
 * Copyright Stuart Golodetz, 2013. All rights reserved.
 ***/

#ifndef H_MILLIPEDE_FORESTSTATISTICSDIALOG
#define H_MILLIPEDE_FORESTSTATISTICSDIALOG

#include <sstream>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include <common/partitionforests/base/PartitionForest.h>
#include <mast/util/StringConversion.h>

namespace mp {

class ForestStatisticsDialog : public wxDialog
{
	//#################### PRIVATE VARIABLES ####################
private:
	int m_index;
	wxListCtrl *m_list;

	//#################### CONSTRUCTORS ####################
public:
	template <typename Forest>
	ForestStatisticsDialog(wxWindow *parent, const boost::shared_ptr<Forest>& forest)
	:	wxDialog(parent, wxID_ANY, wxT("Forest Statistics"), wxDefaultPosition, wxDefaultSize), m_index(0)
	{
		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		// Add a list control to show the statistics.
		m_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(300,200), wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES);
		m_list->InsertColumn(0, wxT("Statistic"));
		m_list->InsertColumn(1, wxT("Value"));

		// Add the statistics themselves.
		add_nonsibling_percentages(forest);
		add_layer_change_percentages(forest);
		add_parent_switch_statistics(forest);

		m_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
		m_list->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
		sizer->Add(m_list, 0, wxALIGN_CENTRE_HORIZONTAL);

		// Add an OK button.
		wxButton *okButton = new wxButton(this, wxID_OK, wxT("OK"));
		sizer->Add(okButton, 0, wxALIGN_CENTRE_HORIZONTAL);
		okButton->SetFocus();

		sizer->Fit(this);
		CentreOnParent();
	}

	//#################### PRIVATE METHODS ####################
private:
	template <typename Forest>
	void add_layer_change_percentages(const boost::shared_ptr<Forest>& forest)
	{
		std::map<int,int> histogram;
		int count = 0;

		for(int i=1, highestLayer=forest->highest_layer(); i<highestLayer; ++i)
		{
			for(typename Forest::EdgeConstIterator jt=forest->edges_cbegin(i), jend=forest->edges_cend(i); jt!=jend; ++jt)
			{
				std::set<PFNodeID> nodes;
				nodes.insert(PFNodeID(i, jt->u));
				nodes.insert(PFNodeID(i, jt->v));

				int layerChange = forest->nonsibling_merge_layer(nodes) - i;
				++histogram[layerChange];
				++count;
			}
		}

		for(std::map<int,int>::const_iterator it=histogram.begin(), iend=histogram.end(); it!=iend; ++it)
		{
			add_percentage("Layer Change % (" + boost::lexical_cast<std::string>(it->first) + ")", (100.0 * it->second) / count);
		}
	}

	void add_nonsibling_percentage(const std::string& which, int nonSiblings, int siblings)
	{
		add_percentage("Non-Sibling % (" + which + ")", (100.0 * nonSiblings) / (nonSiblings + siblings));
	}

	void add_number(const std::string& statistic, size_t number)
	{
		m_list->InsertItem(m_index, string_to_wxString(statistic));
		m_list->SetItem(m_index, 1, string_to_wxString(boost::lexical_cast<std::string>(number)));
		++m_index;
	}

	template <typename Forest>
	void add_parent_switch_statistics(const boost::shared_ptr<Forest>& forest)
	{
		std::set<std::pair<PFNodeID,PFNodeID> > possibleParentSwitches;

		// First, calculate all of the potential parent switches that can be performed.
		for(int i=1, highestLayer=forest->highest_layer(); i<highestLayer; ++i)
		{
			for(typename Forest::NodeConstIterator jt=forest->nodes_cbegin(i), jend=forest->nodes_cend(i); jt!=jend; ++jt)
			{
				PFNodeID node(i, jt.index());
				PFNodeID parent = forest->parent_of(node);
				std::vector<int> adjNodes = forest->adjacent_nodes(node);
				for(typename std::vector<int>::const_iterator kt=adjNodes.begin(), kend=adjNodes.end(); kt!=kend; ++kt)
				{
					PFNodeID adjNode(i, *kt);
					PFNodeID adjParent = forest->parent_of(adjNode);
					if(adjParent != parent) possibleParentSwitches.insert(std::make_pair(node, adjParent));
				}
			}
		}

		add_number("Possible Parent Switches", possibleParentSwitches.size());

		// TODO
	}

	void add_percentage(const std::string& statistic, double percentage)
	{
		std::ostringstream oss;
		oss.setf(std::ios::fixed, std::ios::floatfield);
		oss.precision(3);
		oss << percentage;

		m_list->InsertItem(m_index, string_to_wxString(statistic));
		m_list->SetItem(m_index, 1, string_to_wxString(oss.str()));

		++m_index;
	}

	/**
	@brief	Adds the percentage of the edges that do not share the same parent in each branch layer of the forest.

	Combining the regions they join would require a non-sibling merge rather than a sibling merge.

	@param[in]	forest	The forest for which to calculate non-sibling edge percentages.
	*/
	template <typename Forest>
	void add_nonsibling_percentages(const boost::shared_ptr<Forest>& forest)
	{
		int totalNonSiblings = 0, totalSiblings = 0;

		for(int i=1, highestLayer=forest->highest_layer(); i<highestLayer; ++i)
		{
			int layerNonSiblings = 0, layerSiblings = 0;

			for(typename Forest::EdgeConstIterator jt=forest->edges_cbegin(i), jend=forest->edges_cend(i); jt!=jend; ++jt)
			{
				PFNodeID u(i, jt->u), v(i, jt->v);

				if(forest->parent_of(u) == forest->parent_of(v)) ++layerSiblings;
				else ++layerNonSiblings;
			}

			add_nonsibling_percentage("Layer " + boost::lexical_cast<std::string>(i), layerNonSiblings, layerSiblings);

			totalNonSiblings += layerNonSiblings;
			totalSiblings += layerSiblings;
		}

		add_nonsibling_percentage("Total", totalNonSiblings, totalSiblings);
	}
};

}

#endif
