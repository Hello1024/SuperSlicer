#ifndef PLATER_HPP
#define PLATER_HPP
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/notebook.h>
#include <wx/toolbar.h>

#include <stack>

#include "libslic3r.h"
#include "Model.hpp"
#include "Print.hpp"
#include "Config.hpp"

#include "Plater/PlaterObject.hpp"
#include "Plater/Plate2D.hpp"
#include "Plater/Plate3D.hpp"
#include "Plater/Preview2D.hpp"
#include "Plater/Preview3D.hpp"
#include "Plater/PreviewDLP.hpp"

#include "Settings.hpp"

#include "MainFrame.hpp"

namespace Slic3r { namespace GUI {

using UndoOperation = int;

enum class UndoCmd {
    Remove, Add, Reset
};

using ObjIdx = unsigned int;
using ObjRef = std::vector<PlaterObject>::iterator;

class PlaterObject;
class Plate2D;
class MainFrame;

/// Extension of wxPanel class to handle the main plater.
/// 2D, 3D, preview, etc tabs.
class Plater : public wxPanel 
{
public:
    Plater(wxWindow* parent, const wxString& title, std::shared_ptr<Settings> _settings);

    /// User-level function called through external interface.
    /// Pops file dialog.
    void add();

    /// Remove a selected model from the plater.
    void remove(int obj_idx, bool dont_push = false);
    void remove();
    
    /// Arrange models via a simple packing mechanism based on bounding boxes.
    void arrange();

    /// Ask if there are any unsaved changes.
    bool prompt_unsaved_changes() { return true; }

    void add_undo_operation(UndoCmd cmd, int obj_id, Slic3r::Model& model);

    /// Push an undo op onto the stack.
    void add_undo_operation(UndoCmd cmd, std::vector<int>& obj_ids, Slic3r::Model& model);

private:
    std::shared_ptr<Slic3r::Print> print {std::make_shared<Print>(Slic3r::Print())};
    std::shared_ptr<Slic3r::Model> model {std::make_shared<Model>(Slic3r::Model())};
    std::shared_ptr<Settings> settings {};

    std::shared_ptr<Slic3r::Config> config { Slic3r::Config::new_from_defaults(
        {"bed_shape", "complete_objects", "extruder_clearance_radius", "skirts", "skirt_distance", 
        "brim_width", "serial_port", "serial_speed", "host_type", "print_host", "octoprint_apikey",
        "shortcuts", "filament_colour", "duplicate_distance"})};

    bool processed {false};

    std::vector<PlaterObject> objects {}; //< Main object vector.

    size_t object_identifier {0U}; //< Counter for adding objects to Slic3r. Increment after adding each object.

    std::stack<UndoOperation> undo {}; 
    std::stack<UndoOperation> redo {}; 

    wxNotebook* preview_notebook {new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(335,335), wxNB_BOTTOM)};
    wxBoxSizer* right_sizer {new wxBoxSizer(wxVERTICAL)};

    wxToolBar* htoolbar {nullptr}; //< toolbar for non-MSW platforms.
    wxBoxSizer* btoolbar {nullptr}; //< button-based toolbar for Windows

    Plate2D* canvas2D {nullptr}; //< 2D plater canvas
    Plate3D* canvas3D {nullptr}; //< 3D plater canvas

    Preview2D* preview2D {nullptr}; //< 2D Preview 
    Preview3D* preview3D {nullptr}; //< 3D Preview 

    PreviewDLP* previewDLP {nullptr}; //< DLP/SLA Preview canvas

    /// Handles the actual load of the file from the dialog handoff.
    std::vector<int> load_file(const std::string file, const int obj_idx_to_load = -1);

    const std::string LogChannel {"GUI_Plater"}; //< Which log these messages should go to.

    /// Populate the PlaterObject vector.
    std::vector<int> load_model_objects(ModelObject* model_object);
    std::vector<int> load_model_objects(ModelObjectPtrs model_objects);

    bool scaled_down {false};
    bool outside_bounds {false};

    /// Method to get the top-level window and cast it as a MainFrame.
    MainFrame* GetFrame();

    void select_object(ObjRef obj_idx);
    void select_object(ObjIdx obj_idx);

    /// Overload to unselect objects
    void select_object();
    
    int get_object_index(ObjIdx object_id);

    /// Get the center of the configured bed's bounding box.
    Slic3r::Pointf bed_centerf() {
        const auto& bed_shape { Slic3r::Polygon::new_scale(this->config->get<ConfigOptionPoints>("bed_shape").values) };
        const auto& bed_center {BoundingBox(bed_shape).center()};
        return Slic3r::Pointf::new_unscale(bed_center);
    }

    /// Build thumbnails for the models
    void make_thumbnail(size_t idx);

    /// Complete thumbnail transformation and refresh canvases  
    void on_thumbnail_made(size_t idx); 


    /// Issue a repaint event to all of the canvasses.
    void refresh_canvases();

    /// Action to take when selection changes. Update platers, etc.
    void selection_changed();

    /// Run everything that needs to happen when models change.
    /// Includes updating canvases, reloading menus, etc.
    void on_model_change(bool force_autocenter = false);

    /// Searches the object vector for the first selected object and returns an iterator to it.
    ObjRef selected_object();

    /// Create and launch dialog for object settings.
    void object_settings_dialog();
    void object_settings_dialog(ObjIdx obj_idx);
    void object_settings_dialog(ObjRef obj);

    /// Create and launch menu for object.
    wxMenu* object_menu();

    /// Instantiate the toolbar 
    void build_toolbar();

    /// Clear plate.
    void reset(bool dont_push = false);

    /// Make instances of the currently selected model.
    void increase(); 

    /// Remove instances of the currently selected model.
    void decrease(); 

    /// Rotate the currently selected model.
    void rotate(double angle); 

    /// Separate a multipart model to its component interfaces.
    void split_object(); 

    /// Prompt a change of scaling.
    void changescale();

    /// Open the dialog to perform a cut on the current model.
    void object_cut_dialog();

    /// Open a menu to configure the layer heights.
    void object_layers_dialog();

    /// Process a change in the object list.
    void object_list_changed();

};



} } // Namespace Slic3r::GUI

#endif  // PLATER_HPP
