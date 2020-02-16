/*                                                                   iWave1D.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                        IVANOV'S WAVE VISUALIZATOR 1D                         #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<MenuBar.h>
# include	<ToolBar.h>
# include	<DrawingArea.h>
# include	<TreeView.h>
# include	<StatusBar.h>
# include	<Oscillators.h>

//****************************************************************************//
//      Global objects                                                        //
//****************************************************************************//
GtkWidget		*window;			// Main window
GdkPixbuf		*logo;				// Logo image
GtkAccelGroup	*accelgroup;		// Accelerator group
GtkWidget		*filemenu;			// File menu
GtkWidget		*editmenu;			// Edit menu
GtkWidget		*viewmenu;			// View menu
GtkWidget		*simulationmenu;	// Simulation menu
GtkWidget		*helpmenu;			// Help menu
GtkWidget		*toolbar;			// Tool bar
GtkWidget 		*scale;				// Scale
GtkWidget		*drawing;			// Drawing area
GtkWidget		*treeview;			// Tree view
GtkWidget		*statusbar;			// Status bar
GtkToolItem 	*Run;				// Run button
Oscillators		oscillators;		// Oscillator list

//****************************************************************************//
//      Global variables                                                      //
//****************************************************************************//
guint			menuid;				// Status bar context identifier for menu messages
guint			fileid;				// Status bar context identifier for file messages
gchar			*file_name = NULL;	// Opened file name
gboolean		saved = TRUE;		// Saved document state
gboolean		run = FALSE;		// Run simulation flag
gboolean		vOscillators = TRUE;// View oscillators flag
gboolean		vRwave = FALSE;		// View running waves flag
gboolean		vIwave = TRUE;		// View interference wave flag
gboolean		vEnvelope = TRUE;	// View envelope flag

//============================================================================//
//      Animation settings                                                    //
//============================================================================//
gfloat 			timestamp = 0;		// Current timestamp

//============================================================================//
//      Current viewpoint                                                     //
//============================================================================//
gfloat			startX;				// Begining X coordinate
gfloat			endX;				// Ending X coordinate
gfloat			startY;				// Begining Y coordinate
gfloat			endY;				// Ending Y coordinate

//============================================================================//
//      Graph colors                                                          //
//============================================================================//
GdkRGBA			background;			// Background color
GdkRGBA			text;				// Text color
GdkRGBA			border;				// Border color
GdkRGBA			line;				// Lines color
GdkRGBA			rwave;				// Running waves color
GdkRGBA			iwave;				// Interference wave color
GdkRGBA			envelope;			// Envelope color

//****************************************************************************//
//      Local variables                                                       //
//****************************************************************************//

// Button labels which change sensitivity when opening and closing model
const gchar* FileMenuOpenClose[] = {
	MENU_FILE_SAVE_AS,
	MENU_FILE_CLOSE
};
const gchar* EditMenuOpenClose[] = {
	MENU_EDIT_INSERT,
	MENU_EDIT_ENABLE_ALL,
	MENU_EDIT_INVERT,
	MENU_EDIT_DISABLE_ALL
};
const gchar* SimulationMenuOpenClose[] = {
	MENU_SIMULATION_RESET,
	MENU_SIMULATION_BACKWARD,
	MENU_SIMULATION_FORWARD
};
const gchar* ToolBarOpenClose[] = {
	TOOL_CLOSE,
	TOOL_INSERT,
	TOOL_RESET,
	TOOL_RUN,
	TOOL_PAUSE,
	TOOL_BACKWARD,
	TOOL_FORWARD
};

// Button labels which change sensitivity when saving and modifying oscillator list
const gchar* FileMenuSaveUnsave[] = {
	MENU_FILE_SAVE,
};
const gchar* ToolBarSaveUnsave[] = {
	TOOL_SAVE,
};

// Button labels which change sensitivity when changing selected ossillator
const gchar* EditMenuSelectUnselect[] = {
	MENU_EDIT_REMOVE
};
const gchar* ToolBarSelectUnselect[] = {
	TOOL_REMOVE
};

// Button labels which change sensitivity when run or pause simulation
const gchar* SimulationMenuRunPause[] = {
	MENU_SIMULATION_BACKWARD,
	MENU_SIMULATION_FORWARD
};
const gchar* ToolBarRunPause[] = {
	TOOL_BACKWARD,
	TOOL_FORWARD
};

//****************************************************************************//
//      Program state functions                                               //
//****************************************************************************//

//============================================================================//
//      Change document state                                                 //
//============================================================================//
void ChangeDocumentState (gchar *fname, gboolean state)
{
	// Change oscillator list file name
	g_free (file_name);
	file_name = g_strdup (fname);

	// Change drawing area opacity
	if (state)
		gtk_widget_set_opacity (GTK_WIDGET (drawing), 1);
	else
		gtk_widget_set_opacity (GTK_WIDGET (drawing), 0);

	// Change tree view sensitivity state
	gtk_widget_set_sensitive (GTK_WIDGET (treeview), state);

	// Create arrays of elements to change their sensitivity
	GArray *file_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *edit_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *simulation_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (file_array, FileMenuOpenClose, sizeof (FileMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (edit_array, EditMenuOpenClose, sizeof (EditMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (simulation_array, SimulationMenuOpenClose, sizeof (SimulationMenuOpenClose) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarOpenClose, sizeof (ToolBarOpenClose) / sizeof (const gchar*));

	// Set label arrays
	LabelArray FileArray = {file_array, state};
	LabelArray EditArray = {edit_array, state};
	LabelArray SimulationArray = {simulation_array, state};
	LabelArray ToolArray = {tool_array, state};

	// Change elements sensitivity
	gtk_container_foreach (GTK_CONTAINER (filemenu), ChangeMenuButtonsSensitivity, &FileArray);
	gtk_container_foreach (GTK_CONTAINER (editmenu), ChangeMenuButtonsSensitivity, &EditArray);
	gtk_container_foreach (GTK_CONTAINER (simulationmenu), ChangeMenuButtonsSensitivity, &SimulationArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (file_array, TRUE);
	g_array_free (edit_array, TRUE);
	g_array_free (simulation_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//============================================================================//
//      Change save state                                                     //
//============================================================================//
void ChangeSavedState (gboolean state)
{
	// Change document saved flag
	saved = state;

	// Check if we have opened file
	if (state || !state && file_name)
	{
		// Create arrays of elements to change their sensitivity
		GArray *file_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
		GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

		// Set array elements
		g_array_append_vals (file_array, FileMenuSaveUnsave, sizeof (FileMenuSaveUnsave) / sizeof (const gchar*));
		g_array_append_vals (tool_array, ToolBarSaveUnsave, sizeof (ToolBarSaveUnsave) / sizeof (const gchar*));

		// Set label arrays
		LabelArray FileArray = {file_array, !state};
		LabelArray ToolArray = {tool_array, !state};

		// Make elements insesitive
		gtk_container_foreach (GTK_CONTAINER (filemenu), ChangeMenuButtonsSensitivity, &FileArray);
		gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

		// Release array objects
		g_array_free (file_array, TRUE);
		g_array_free (tool_array, TRUE);
	}

	// Init envelope arrays
	if (!state)
		InitEnvelopeArrays ();

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Change selection state                                                //
//============================================================================//
void ChangeSelectionState (gboolean state)
{
	// Create arrays of elements to change their sensitivity
	GArray *edit_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (edit_array, EditMenuSelectUnselect, sizeof (EditMenuSelectUnselect) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarSelectUnselect, sizeof (ToolBarSelectUnselect) / sizeof (const gchar*));

	// Set label arrays
	LabelArray EditArray = {edit_array, state};
	LabelArray ToolArray = {tool_array, state};

	// Make elements insesitive
	gtk_container_foreach (GTK_CONTAINER (editmenu), ChangeMenuButtonsSensitivity, &EditArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (edit_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//============================================================================//
//      Change run state                                                      //
//============================================================================//
void ChangeRunState (gboolean state)
{
	// Change run flag
	run = state;

	// Create arrays of elements to change their sensitivity
	GArray *simulation_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));
	GArray *tool_array = g_array_new (FALSE, FALSE, sizeof (const gchar*));

	// Set array elements
	g_array_append_vals (simulation_array, SimulationMenuRunPause, sizeof (SimulationMenuRunPause) / sizeof (const gchar*));
	g_array_append_vals (tool_array, ToolBarRunPause, sizeof (ToolBarRunPause) / sizeof (const gchar*));

	// Set label arrays
	LabelArray SimulationArray = {simulation_array, !state};
	LabelArray ToolArray = {tool_array, !state};

	// Make elements insesitive
	gtk_container_foreach (GTK_CONTAINER (simulationmenu), ChangeMenuButtonsSensitivity, &SimulationArray);
	gtk_container_foreach (GTK_CONTAINER (toolbar), ChangeToolButtonsSensitivity, &ToolArray);

	// Release array objects
	g_array_free (simulation_array, TRUE);
	g_array_free (tool_array, TRUE);
}

//****************************************************************************//
//      Create panes                                                          //
//****************************************************************************//
static GtkWidget* CreatePanes (void)
{
	// Create panes
	GtkWidget *paned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

	// Add items to panes
	gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET (CreateDrawingArea ()), TRUE, FALSE);
	gtk_paned_pack2 (GTK_PANED (paned), GTK_WIDGET (CreateTreeView ()), TRUE, TRUE);

	// Return paned object
	return paned;
}

//****************************************************************************//
//      Layout                                                                //
//****************************************************************************//
static GtkWidget* CreateLayout (void)
{
	// Create box
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	// Add items to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateMenuBar ()), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolLayout ()), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreatePanes ()), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateStatusBar ()), FALSE, FALSE, 0);

	// Return box object
	return box;
}

//****************************************************************************//
//      Main function                                                         //
//****************************************************************************//
int main (int argc, char *argv[])
{
	// Create error object
	GError *error = NULL;

	// Init GTK library
	gtk_init (&argc, &argv);

	// Create main window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// Create accelerator group
	accelgroup = gtk_accel_group_new ();

	// Add accelerator group to window
	gtk_window_add_accel_group (GTK_WINDOW (window), GTK_ACCEL_GROUP (accelgroup));

	// Add items to main widow
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (CreateLayout ()));

	// Load program logo
	logo = gdk_pixbuf_new_from_file (LOGO_FILE, &error);
	if (!logo)
		ShowStatusMessage (error -> message, fileid);

	// Set window properties
	gtk_window_set_icon (GTK_WINDOW (window), GDK_PIXBUF (logo));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_decorated (GTK_WINDOW (window), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
	gtk_window_set_focus_visible (GTK_WINDOW (window), FALSE);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (QuitProgram), NULL);

	// Show all window elements
	gtk_widget_show_all (GTK_WIDGET (window));

	// Set initial program state
	CloseDocument ();

	// Start main GTK loop
	gtk_main ();

	// Normal exit
	return 0;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
