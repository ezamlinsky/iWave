/*                                                                   MenuBar.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                   MENU BAR                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
// TODO: Добавить функции отмены действия для случая закрытия файла
# include	<Common.h>
# include	<Oscillators.h>
# include	<DrawingArea.h>
# include	<TreeView.h>
# include	<StatusBar.h>
# include	<MenuBar.h>

//****************************************************************************//
//      External objects                                                      //
//****************************************************************************//
extern GtkWidget		*window;			// Main window
extern GdkPixbuf		*logo;				// Logo image
extern GtkAccelGroup	*accelgroup;		// Accelerator group
extern GtkWidget		*filemenu;			// File menu
extern GtkWidget		*editmenu;			// Edit menu
extern GtkWidget		*viewmenu;			// View menu
extern GtkWidget		*simulationmenu;	// Simulation menu
extern GtkWidget		*helpmenu;			// Help menu
extern GtkWidget 		*scale;				// Scale
extern GtkWidget		*drawing;			// Drawing area
extern GtkWidget		*treeview;			// Tree view
extern GtkWidget		*statusbar;			// Status bar
extern GtkToolItem 		*Run;				// Run button
extern Oscillators		oscillators;		// Oscillators list

//****************************************************************************//
//      External variables                                                    //
//****************************************************************************//
extern guint			menuid;				// Status bar context identifier for menu messages
extern guint			fileid;				// Status bar context identifier for file messages
extern gchar			*file_name;			// Opened file name
extern gboolean			saved;				// Save document state
extern gboolean			run;				// Run simulation flag
extern gboolean			vOscillators;		// View oscillators flag
extern gboolean			vRwave;				// View running waves flag
extern gboolean			vIwave;				// View interference wave flag
extern gboolean			vEnvelope;			// View envelope flag

//============================================================================//
//      Animation settings                                                    //
//============================================================================//
extern gfloat 			timestamp;			// Current timestamp

//============================================================================//
//      Graph colors                                                          //
//============================================================================//
extern GdkRGBA			background;			// Background color
extern GdkRGBA			text;				// Text color
extern GdkRGBA			border;				// Border color
extern GdkRGBA			line;				// Lines color
extern GdkRGBA			rwave;				// Running waves color
extern GdkRGBA			iwave;				// Interference wave color
extern GdkRGBA			envelope;			// Envelope color

//****************************************************************************//
//      Local functions                                                       //
//****************************************************************************//
static gboolean SaveModelAs (void);

//****************************************************************************//
//      Show error message                                                    //
//****************************************************************************//
void ShowErrorMessage (GtkWindow *parent, const gchar *message, GError *error)
{
	// Create message dialog window
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (parent), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, message);

	// Set dialog message
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), error -> message);

	// Run dialog window
	gtk_dialog_run (GTK_DIALOG (dialog));

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));

	// Release error object
	g_error_free (error);
}

//****************************************************************************//
//      Show file operation error message                                     //
//****************************************************************************//
static void ShowFileErrorMessage (GtkWindow *parent, const gchar *message, GError *error)
{
	// Show status message
	ShowStatusMessage (error -> message, fileid);

	// Show error message
	ShowErrorMessage (parent, message, error);
}

//****************************************************************************//
//      Change menu buttons sensitivity                                       //
//****************************************************************************//
void ChangeMenuButtonsSensitivity (GtkWidget *widget, gpointer data)
{
	// Check if widget is menu item
	if (GTK_IS_MENU_ITEM (widget) && !GTK_IS_SEPARATOR_MENU_ITEM (widget))
	{
		// Get menu item label
		const gchar *label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

		// Convert data pointer
		LabelArray *larray = reinterpret_cast <LabelArray*> (data);

		// Get label array properties
		GArray *array = larray -> array;
		guint size = array -> len;
		gboolean state = larray -> state;

		// Iterate through all elements
		for (guint i = 0; i < size; i++)
		{
			// Check if menu label match to array element
			if (g_utf8_collate (label, g_array_index (array, gchar*, i)) == 0)
			{
				// Change sensitivity state of the menu item
				gtk_widget_set_sensitive (GTK_WIDGET (widget), state);
			}
		}
	}
}

//****************************************************************************//
//      Save oscillator list changes                                          //
//****************************************************************************//
static gboolean SaveChanges (void)
{
	// Operation status
	gboolean status = TRUE;

	// Check if oscillators is not saved
	if (!saved)
	{
		// Create message dialog window
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "The model was changed");

		// Set dialog message
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "Would you like to save it?");

		// Add cancel button
		gtk_dialog_add_button (GTK_DIALOG (dialog), "_Cancel", GTK_RESPONSE_CANCEL);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

		// Run dialog window
		gint response = gtk_dialog_run (GTK_DIALOG (dialog));

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));

		// Check user's choice
		switch (response)
		{
			// Try to save oscillator list
			case GTK_RESPONSE_YES:
				if (!SaveModel ())
				{
					// If can not save changes, then return fail status
					return FALSE;
				}
				break;

			// Discard changes
			case GTK_RESPONSE_NO:
				return TRUE;

			// Do not close the model
			default:
				return FALSE;
		}
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Signal handlers                                                       //
//****************************************************************************//

//============================================================================//
//      Signal handler for "select" menu signal                               //
//============================================================================//
static void MenuSelect (GtkMenuItem *menuitem, gpointer data)
{
	// Push new message into status bar stack
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), menuid, reinterpret_cast <const gchar*> (data));
}

//============================================================================//
//      Signal handler for "deselect" menu signal                             //
//============================================================================//
static void MenuDeselect (GtkMenuItem *menuitem, gpointer data)
{
	// Pop message from the top of status bar stack
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), menuid);
}

//============================================================================//
//      Signal handler for "New" menu button                                  //
//============================================================================//
void NewModel (void)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ()) return;
	}

	// Create new oscillator list
	oscillators.NewList ();

	// Set main window title
	gtk_window_set_title (GTK_WINDOW (window), PROGRAM_TITLE " - Unsaved model");

	// Show status message
	ShowStatusMessage ("New model is created", fileid);

	// Set tree view model
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (oscillators.GetOscillatorList ()));

	// Set sort column
	SetSortColumn (GTK_TREE_VIEW (treeview), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);

	// Change document state
	ChangeDocumentState (NULL, TRUE);

	// Change save state
	ChangeSavedState (TRUE);

	// Reset simulation
	ResetSimulation ();

	// Pause simulation
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (Run), FALSE);
}

//============================================================================//
//      Signal handler for "Open" menu button                                 //
//============================================================================//
void OpenModel (void)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ()) return;
	}

	// Create dialog window
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open oscillators from...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

	// Create file filter
	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.xml");

	// Add file filter to file chooser
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

	// Set file chooser properties
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	// Run dialog window
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Get chosen file name
		gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// Create error object
		GError *error = NULL;

		// Open oscillator list from file
		if (!oscillators.OpenList (path, &error))
			ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not open oscillator list", error);
		else
		{
			// Extract file name from path
			gchar *fname = g_filename_display_basename (path);

			// Create string buffers
			GString *tstring = g_string_new (NULL);
			GString *sstring = g_string_new (NULL);

			// Create title bar string
			g_string_printf (tstring, PROGRAM_TITLE " - %s", fname);

			// Create status bar string
			g_string_printf (sstring, "Opened model file '%s'", fname);

			// Set main window title
			gtk_window_set_title (GTK_WINDOW (window), tstring -> str);

			// Show status message
			ShowStatusMessage (sstring -> str, fileid);

			// Free temporary string buffer
			g_free (fname);

			// Relase string buffers
			g_string_free (tstring, TRUE);
			g_string_free (sstring, TRUE);

			// Get oscillator list
			model = GTK_TREE_MODEL (oscillators.GetOscillatorList ());

			// Set tree view model
			gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));

			// Set sort column
			SetSortColumn (GTK_TREE_VIEW (treeview), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);

			// Change document state
			ChangeDocumentState (path, TRUE);

			// Change save state
			ChangeSavedState (TRUE);

			// Reset simulation
			ResetSimulation ();

			// Pause simulation
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (Run), FALSE);
		}

		// Free temporary string buffer
		g_free (path);
	}

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

//============================================================================//
//      Signal handler for "Save" menu button                                 //
//============================================================================//
gboolean SaveModel (void)
{
	// Check if file name is set
	if (file_name)
	{
		// Operation status
		gboolean status = FALSE;

		// Get tree model object from tree view
		GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
		if (model)
		{
			// Create error object
			GError *error = NULL;

			// Save oscillator list into file
			if (!oscillators.SaveList (file_name, &error))
				ShowFileErrorMessage (GTK_WINDOW (window), "Can not save model", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (file_name);

				// Create string buffer
				GString *sstring = g_string_new (NULL);

				// Create status bar string
				g_string_printf (sstring, "Model saved into file '%s'", fname);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffer
				g_string_free (sstring, TRUE);

				// Change save state
				ChangeSavedState (TRUE);

				// Set success state
				status = TRUE;
			}
		}

		// Return operation status
		return status;
	}
	else
		return SaveModelAs ();
}

//============================================================================//
//      Signal handler for "SaveAs" menu button                               //
//============================================================================//
static gboolean SaveModelAs (void)
{
	// Operation status
	gboolean status = FALSE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save model to...", GTK_WINDOW (window), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

		// Create file filter
		GtkFileFilter *filter = gtk_file_filter_new ();
		gtk_file_filter_add_pattern (GTK_FILE_FILTER (filter), "*.xml");

		// Add file filter to file chooser
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), GTK_FILE_FILTER (filter));

		// Set file chooser properties
		gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);
		gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Oscillators.xml");

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			// Get chosen file name
			gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// Create error object
			GError *error = NULL;

			// Save oscillator list into file
			if (!oscillators.SaveList (path, &error))
				ShowFileErrorMessage (GTK_WINDOW (dialog), "Can not save model", error);
			else
			{
				// Extract file name from path
				gchar *fname = g_filename_display_basename (path);

				// Create string buffers
				GString *tstring = g_string_new (NULL);
				GString *sstring = g_string_new (NULL);

				// Create title bar string
				g_string_printf (tstring, PROGRAM_TITLE " - %s", fname);

				// Create status bar string
				g_string_printf (sstring, "Model saved into file '%s'", fname);

				// Set main window title
				gtk_window_set_title (GTK_WINDOW (window), tstring -> str);

				// Show status message
				ShowStatusMessage (sstring -> str, fileid);

				// Free temporary string buffer
				g_free (fname);

				// Relase string buffers
				g_string_free (tstring, TRUE);
				g_string_free (sstring, TRUE);

				// Change document state
				ChangeDocumentState (path, TRUE);

				// Change save state
				ChangeSavedState (TRUE);

				// Set success state
				status = TRUE;
			}

			// Free temporary string buffer
			g_free (path);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	// Return operation status
	return status;
}

//============================================================================//
//      Signal handler for "Close" menu button                                //
//============================================================================//
gboolean CloseDocument (void)
{
	// Operation status
	gboolean status = TRUE;

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// If file is changed, then ask to save changes before continue
		if (!SaveChanges ()) return FALSE;

		// Clear oscillator list
		oscillators.ClearList ();

		// Check oscillators file name
		if (file_name)
		{
			// Extract file name from path
			gchar *fname = g_filename_display_basename (file_name);

			// Create string buffer
			GString *sstring = g_string_new (NULL);

			// Create status bar string
			g_string_printf (sstring, "File '%s' closed", fname);

			// Show status message
			ShowStatusMessage (sstring -> str, fileid);

			// Free temporary string buffer
			g_free (fname);

			// Relase string buffer
			g_string_free (sstring, TRUE);
		}
		else
		{
			// Show status message
			ShowStatusMessage ("The model is closed", fileid);
		}

		// Set tree view model
		gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (oscillators.GetOscillatorList ()));
	}

	// Set main window title
	gtk_window_set_title (GTK_WINDOW (window), PROGRAM_TITLE);

	// Change program state
	ChangeDocumentState (NULL, FALSE);

	// Change save state
	ChangeSavedState (TRUE);

	// Change selection state
	ChangeSelectionState (FALSE);

	// Return operation status
	return status;
}

//============================================================================//
//      Signal handler for "Quit" menu button                                 //
//============================================================================//
void QuitProgram (void)
{
	// Try to close opened document before exit
	if (CloseDocument ())
	{
		// Quit program
		gtk_main_quit ();
	}
}

//============================================================================//
//      Signal handler for "Insert" menu button                               //
//============================================================================//
void InsertOscillator (void)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Create dialog window
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("Oscillator details", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL, "_Insert", GTK_RESPONSE_OK, NULL);

		// Get content area of dialog
		GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

		// Create grid
		GtkWidget *grid = gtk_grid_new ();

		// Create label fields
		GtkWidget *PositionLabel = gtk_label_new ("Position");
		GtkWidget *MagnitudeLabel = gtk_label_new ("Magnitude");
		GtkWidget *PhaseLabel = gtk_label_new ("Initial phase");
		GtkWidget *WavelengthLabel = gtk_label_new ("Wavelength");
		GtkWidget *BetaLabel = gtk_label_new ("Beta");

		// Create spin buttons
		GtkWidget *PositionSpin = gtk_spin_button_new_with_range (POSITION_MIN, POSITION_MAX, POSITION_STEP);
		GtkWidget *MagnitudeSpin = gtk_spin_button_new_with_range (MAGNITUDE_MIN, MAGNITUDE_MAX, MAGNITUDE_STEP);
		GtkWidget *PhaseSpin = gtk_spin_button_new_with_range (PHASE_MIN, PHASE_MAX, PHASE_STEP);
		GtkWidget *WavelengthSpin = gtk_spin_button_new_with_range (WAVELENGTH_MIN, WAVELENGTH_MAX, WAVELENGTH_STEP);
		GtkWidget *BetaSpin = gtk_spin_button_new_with_range (BETA_MIN, BETA_MAX, BETA_STEP);

		// Create check button
		GtkWidget *EnableBox = gtk_check_button_new_with_label ("Enabled");

		// Add labels to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PositionLabel), 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (MagnitudeLabel), 0, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PhaseLabel), 0, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (WavelengthLabel), 0, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BetaLabel), 0, 4, 1, 1);

		// Add spin buttons to grid
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PositionSpin), 1, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (MagnitudeSpin), 1, 1, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (PhaseSpin), 1, 2, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (WavelengthSpin), 1, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BetaSpin), 1, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (EnableBox), 1, 5, 1, 1);

		// Add tooltip text to spin buttons
		gtk_widget_set_tooltip_text (GTK_WIDGET (PositionSpin), "Oscillator position");
		gtk_widget_set_tooltip_text (GTK_WIDGET (MagnitudeSpin), "Oscillation magnitude");
		gtk_widget_set_tooltip_text (GTK_WIDGET (PhaseSpin), "Initial phase of oscillation");
		gtk_widget_set_tooltip_text (GTK_WIDGET (WavelengthSpin), "Oscillator wavelength");
		gtk_widget_set_tooltip_text (GTK_WIDGET (BetaSpin), "Beta coefficient");
		gtk_widget_set_tooltip_text (GTK_WIDGET (EnableBox), "Allow oscillator to pulse");

		// Add grid to dialog content area
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (grid), TRUE, TRUE, 0);

		// Set label properties
		gtk_label_set_selectable (GTK_LABEL (PositionLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (MagnitudeLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (PhaseLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (WavelengthLabel), FALSE);
		gtk_label_set_selectable (GTK_LABEL (BetaLabel), FALSE);
		gtk_label_set_single_line_mode (GTK_LABEL (PositionLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (MagnitudeLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (PhaseLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (WavelengthLabel), TRUE);
		gtk_label_set_single_line_mode (GTK_LABEL (BetaLabel), TRUE);
		gtk_widget_set_halign (GTK_WIDGET (PositionLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (MagnitudeLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (PhaseLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (WavelengthLabel), GTK_ALIGN_END);
		gtk_widget_set_halign (GTK_WIDGET (BetaLabel), GTK_ALIGN_END);

		// Set spin button properties
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (PositionSpin), POSITION_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (MagnitudeSpin), MAGNITUDE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (PhaseSpin), PHASE_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (WavelengthSpin), WAVELENGTH_DIGITS);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (BetaSpin), BETA_DIGITS);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (PositionSpin), POSITION_STEP, POSITION_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (MagnitudeSpin), MAGNITUDE_STEP, MAGNITUDE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (PhaseSpin), PHASE_STEP, PHASE_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (WavelengthSpin), WAVELENGTH_STEP, WAVELENGTH_PAGE);
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (BetaSpin), BETA_STEP, BETA_PAGE);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (PositionSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (MagnitudeSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (PhaseSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (WavelengthSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (BetaSpin), GTK_UPDATE_IF_VALID);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (PositionSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (MagnitudeSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (PhaseSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (WavelengthSpin), TRUE);
		gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (BetaSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (PositionSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (MagnitudeSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (PhaseSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (WavelengthSpin), TRUE);
		gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (BetaSpin), TRUE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (PositionSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (MagnitudeSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (PhaseSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (WavelengthSpin), FALSE);
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (BetaSpin), FALSE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (PositionSpin), 0.0);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (MagnitudeSpin), 1.0);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (PhaseSpin), 0.0);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (WavelengthSpin), 2.0);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (BetaSpin), 0.0);
		gtk_widget_set_hexpand (GTK_WIDGET (PositionSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (MagnitudeSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (PhaseSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (WavelengthSpin), TRUE);
		gtk_widget_set_hexpand (GTK_WIDGET (BetaSpin), TRUE);

		// Set grid properties
		gtk_container_set_border_width (GTK_CONTAINER (grid), 6);
		gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
		gtk_grid_set_column_spacing (GTK_GRID (grid), 6);

		// Set window properties
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

		// Set default dialog button
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

		// Show all box elements
		gtk_widget_show_all (GTK_WIDGET (box));

		// Run dialog window
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
		{
			// Extract oscillator information from dialog inputs
			gfloat position = gtk_spin_button_get_value (GTK_SPIN_BUTTON (PositionSpin));
			gfloat magnitude = gtk_spin_button_get_value (GTK_SPIN_BUTTON (MagnitudeSpin));
			gfloat phase = gtk_spin_button_get_value (GTK_SPIN_BUTTON (PhaseSpin));
			gfloat wavelength = gtk_spin_button_get_value (GTK_SPIN_BUTTON (WavelengthSpin));
			gfloat beta = gtk_spin_button_get_value (GTK_SPIN_BUTTON (BetaSpin));
			gboolean enable = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (EnableBox));

			// Create error object
			GError *error = NULL;

			// Add new element to list store object
			GtkTreeIter iter;
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, OSCILLATOR_POSITION_ID, position, OSCILLATOR_MAGNITUDE_ID, magnitude, OSCILLATOR_PHASE_ID, phase, OSCILLATOR_WAVELENGTH_ID, wavelength, OSCILLATOR_BETA_ID, beta, OSCILLATOR_ENABLE_ID, enable, -1);

			// Change save state
			ChangeSavedState (FALSE);
		}

		// Destroy dialog widget
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

//============================================================================//
//      Signal handler for "Remove" menu button                               //
//============================================================================//
void RemoveOscillators (void)
{
	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Get selected rows
	GtkTreeModel *model;
	GList *plist = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), &model);
	GList *rlist = NULL;
	GList *element = NULL;

	// Process all path list elements
	element = g_list_first (plist);
	while (element)
	{
		// Append new element to reference list
		rlist = g_list_append (rlist, gtk_tree_row_reference_new (GTK_TREE_MODEL (model), reinterpret_cast <GtkTreePath*> (element -> data)));

		// Go to next path list element
		element = element -> next;
	}

	// Process all reference list elements
	element = g_list_first (rlist);
	if (element)
	{
		do {
			// Get tree path
			GtkTreePath *path = gtk_tree_row_reference_get_path (reinterpret_cast <GtkTreeRowReference*> (element -> data));
			if (path)
			{
				// Get iterator position
				GtkTreeIter iter;
				if (gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path))
				{
					// Remove oscillator from the list
					gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
				}
			}

			// Go to next reference list element
			element = element -> next;
		} while (element);

		// Change save state
		ChangeSavedState (FALSE);
	}

	// Release list of selected rows
	g_list_free_full (plist, reinterpret_cast <GDestroyNotify> (gtk_tree_path_free));

	// Release list of references
	g_list_free_full (rlist, reinterpret_cast <GDestroyNotify> (gtk_tree_row_reference_free));
}

//============================================================================//
//      Signal handler for "EnableAll" menu button                            //
//============================================================================//
static void EnableAllOscillators (void)
{
	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Set mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, OSCILLATOR_ENABLE_ID, TRUE, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
		}

		// If oscillators were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);

		// Change save state
		ChangeSavedState (FALSE);
	}
}

//============================================================================//
//      Signal handler for "Invert" menu button                               //
//============================================================================//
static void InvertSelection (void)
{
	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Get old state
				gboolean state;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OSCILLATOR_ENABLE_ID, &state, -1);

				// Switch mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, OSCILLATOR_ENABLE_ID, !state, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
		}

		// If oscillators were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);

		// Change save state
		ChangeSavedState (FALSE);
	}
}

//============================================================================//
//      Signal handler for "DisableAll" menu button                           //
//============================================================================//
static void DisableAllOscillators (void)
{
	// Get tree model object from tree view
	GtkTreeSortable *model = GTK_TREE_SORTABLE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	if (model)
	{
		// Get current sort order
		gint id;
		GtkSortType order;
		gboolean issorted = gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);

		// Change sort column
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Clear mark state
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, OSCILLATOR_ENABLE_ID, FALSE, -1);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
		}

		// If oscillators were sorted, then restore their sort order
		if (issorted)
			gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), id, order);

		// Change save state
		ChangeSavedState (FALSE);
	}
}

//============================================================================//
//      Signal handler for "Preferences" menu button                          //
//============================================================================//
void SetPreferences (void)
{
	// Create dialog window
	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Preferences", GTK_WINDOW (window), GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, "_Default", GTK_RESPONSE_NO, "_Set", GTK_RESPONSE_YES, NULL);

	// Get content area of dialog
	GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	// Create grid
	GtkWidget *grid = gtk_grid_new ();

	// Create label fields
	GtkWidget *BackgroundLabel = gtk_label_new ("Background color");
	GtkWidget *TextLabel = gtk_label_new ("Text color");
	GtkWidget *BorderLabel = gtk_label_new ("Border color");
	GtkWidget *LineLabel = gtk_label_new ("Lines color");
	GtkWidget *RWaveLabel = gtk_label_new ("Running waves color");
	GtkWidget *IWaveLabel = gtk_label_new ("Interference wave color");
	GtkWidget *EnvelopeLabel = gtk_label_new ("Envelope color");

	// Create color buttons
	GtkWidget *BackgroundColor = gtk_color_button_new_with_rgba (&background);
	GtkWidget *TextColor = gtk_color_button_new_with_rgba (&text);
	GtkWidget *BorderColor = gtk_color_button_new_with_rgba (&border);
	GtkWidget *LineColor = gtk_color_button_new_with_rgba (&line);
	GtkWidget *RWaveColor = gtk_color_button_new_with_rgba (&rwave);
	GtkWidget *IWaveColor = gtk_color_button_new_with_rgba (&iwave);
	GtkWidget *EnvelopeColor = gtk_color_button_new_with_rgba (&envelope);

	// Add labels to grid
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BackgroundLabel), 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TextLabel), 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BorderLabel), 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LineLabel), 0, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (RWaveLabel), 0, 4, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IWaveLabel), 0, 5, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (EnvelopeLabel), 0, 6, 1, 1);

	// Add spin buttons to grid
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BackgroundColor), 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (TextColor), 1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (BorderColor), 1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (LineColor), 1, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (RWaveColor), 1, 4, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (IWaveColor), 1, 5, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (EnvelopeColor), 1, 6, 1, 1);

	// Add tooltip text to spin buttons
	gtk_widget_set_tooltip_text (GTK_WIDGET (BackgroundColor), "Background color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (TextColor), "Text color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (BorderColor), "Border color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (LineColor), "Lines color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (RWaveColor), "Running waves color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (IWaveColor), "Interference wave color");
	gtk_widget_set_tooltip_text (GTK_WIDGET (EnvelopeColor), "Envelope color");

	// Add grid to dialog content area
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (grid), TRUE, TRUE, 0);

	// Set label properties
	gtk_label_set_selectable (GTK_LABEL (BackgroundLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (TextLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (BorderLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (LineLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (RWaveLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (IWaveLabel), FALSE);
	gtk_label_set_selectable (GTK_LABEL (EnvelopeLabel), FALSE);
	gtk_label_set_single_line_mode (GTK_LABEL (BackgroundLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (TextLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (BorderLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (LineLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (RWaveLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (IWaveLabel), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (EnvelopeLabel), TRUE);
	gtk_widget_set_halign (GTK_WIDGET (BackgroundLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (TextLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (BorderLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (LineLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (RWaveLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (IWaveLabel), GTK_ALIGN_END);
	gtk_widget_set_halign (GTK_WIDGET (EnvelopeLabel), GTK_ALIGN_END);
	gtk_widget_set_hexpand (GTK_WIDGET (BackgroundLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (TextLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (BorderLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (LineLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (RWaveLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (IWaveLabel), TRUE);
	gtk_widget_set_hexpand (GTK_WIDGET (EnvelopeLabel), TRUE);

	// Set grid properties
	gtk_container_set_border_width (GTK_CONTAINER (grid), 6);
	gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 6);

	// Set window properties
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	// Set default dialog button
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	// Show all box elements
	gtk_widget_show_all (GTK_WIDGET (box));

	// Run dialog window
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result)
	{
		// Extract color values from dialog inputs
		case GTK_RESPONSE_YES:
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (BackgroundColor), &background);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (TextColor), &text);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (BorderColor), &border);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (LineColor), &line);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (RWaveColor), &rwave);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (IWaveColor), &iwave);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (EnvelopeColor), &envelope);
			break;

		// Set dafault colors
		case GTK_RESPONSE_NO:
			gdk_rgba_parse  (&background, COLOR_BACKGROUND);
			gdk_rgba_parse  (&text, COLOR_TEXT);
			gdk_rgba_parse  (&border, COLOR_BORDER);
			gdk_rgba_parse  (&line, COLOR_LINE);
			gdk_rgba_parse  (&rwave, COLOR_RWAVE);
			gdk_rgba_parse  (&iwave, COLOR_IWAVE);
			gdk_rgba_parse  (&envelope, COLOR_ENVELOPE);
			break;

		default:
			break;
	}

	// Destroy dialog widget
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

//============================================================================//
//      Signal handler for "Oscillators" menu button                          //
//============================================================================//
static void ViewOscillators (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	// Set new flag value
	vOscillators = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (checkmenuitem));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "RunningWaves" menu button                         //
//============================================================================//
static void ViewRunningWaves (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	// Set new flag value
	vRwave = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (checkmenuitem));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "InterferenceWave" menu button                     //
//============================================================================//
static void ViewInterferenceWave (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	// Set new flag value
	vIwave = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (checkmenuitem));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "Envelope" menu button                             //
//============================================================================//
static void ViewEnvelopeWave (GtkCheckMenuItem *checkmenuitem, gpointer user_data)
{
	// Set new flag value
	vEnvelope = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (checkmenuitem));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "Reset" menu button                                //
//============================================================================//
void ResetSimulation (void)
{
	// Reset timestamp to initial value
	timestamp = 0;

	// Init envelope arrays
	InitEnvelopeArrays ();

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "Backward" menu button                             //
//============================================================================//
void BackwardStep (void)
{
	// Do one step backward in time
	timestamp -= gtk_range_get_value (GTK_RANGE (scale));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "Forward" menu button                              //
//============================================================================//
void ForwardStep (void)
{
	// Do one step forward in time
	timestamp += gtk_range_get_value (GTK_RANGE (scale));

	// Redraw drawing area
	gtk_widget_queue_draw (GTK_WIDGET (drawing));
}

//============================================================================//
//      Signal handler for "About" menu button                                //
//============================================================================//
static void AboutDialog (void)
{
	// Set authors array
	const gchar* authors[] = {"Jack Black <ezamlinsky@gmail.com>", NULL};

	// Show about dialog window
	gtk_show_about_dialog (GTK_WINDOW (window), "program-name", PROGRAM_NAME, "version", "1.0", "comments", "1D visualizator for Ivanov's waves", "website-label", "LinAsm website", "website", "http://linasm.sourceforge.net/", "copyright", "Copyright (C) 2017, Jack Black", "license-type", GTK_LICENSE_GPL_3_0, "authors", authors, "logo", logo, NULL);
}

//****************************************************************************//
//      "File" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateFileSubMenu (void)
{
	// Create submenu
	filemenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *New = gtk_menu_item_new_with_mnemonic (MENU_FILE_NEW);
	GtkWidget *Open = gtk_menu_item_new_with_mnemonic (MENU_FILE_OPEN);
	GtkWidget *Separator1 = gtk_separator_menu_item_new ();
	GtkWidget *Save = gtk_menu_item_new_with_mnemonic (MENU_FILE_SAVE);
	GtkWidget *SaveAs = gtk_menu_item_new_with_mnemonic (MENU_FILE_SAVE_AS);
	GtkWidget *Separator2 = gtk_separator_menu_item_new ();
	GtkWidget *Close = gtk_menu_item_new_with_mnemonic (MENU_FILE_CLOSE);
	GtkWidget *Quit = gtk_menu_item_new_with_mnemonic (MENU_FILE_QUIT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (New));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Open));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator1));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Save));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (SaveAs));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Separator2));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Close));
	gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), GTK_WIDGET (Quit));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (New), "activate", GTK_ACCEL_GROUP (accelgroup), 'N', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Open), "activate", GTK_ACCEL_GROUP (accelgroup), 'O', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Save), "activate", GTK_ACCEL_GROUP (accelgroup), 'S', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Close), "activate", GTK_ACCEL_GROUP (accelgroup), 'W', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Quit), "activate", GTK_ACCEL_GROUP (accelgroup), 'Q', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (New), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Open), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Save), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (SaveAs), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Close), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Quit), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (New), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Create new model"));
	g_signal_connect (G_OBJECT (Open), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Open existing model"));
	g_signal_connect (G_OBJECT (Save), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Save model into the file"));
	g_signal_connect (G_OBJECT (SaveAs), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Save model into a new file"));
	g_signal_connect (G_OBJECT (Close), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Close opened model"));
	g_signal_connect (G_OBJECT (Quit), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Quit the application"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (New), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Open), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Save), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (SaveAs), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Close), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Quit), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (New), "activate", G_CALLBACK (NewModel), NULL);
	g_signal_connect (G_OBJECT (Open), "activate", G_CALLBACK (OpenModel), NULL);
	g_signal_connect (G_OBJECT (Save), "activate", G_CALLBACK (SaveModel), NULL);
	g_signal_connect (G_OBJECT (SaveAs), "activate", G_CALLBACK (SaveModelAs), NULL);
	g_signal_connect (G_OBJECT (Close), "activate", G_CALLBACK (CloseDocument), NULL);
	g_signal_connect (G_OBJECT (Quit), "activate", G_CALLBACK (QuitProgram), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateFileMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic (MENU_FILE);

	// Create submenu
	CreateFileSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (filemenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Edit" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateEditSubMenu (void)
{
	// Create submenu
	editmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *Insert = gtk_menu_item_new_with_mnemonic (MENU_EDIT_INSERT);
	GtkWidget *Remove = gtk_menu_item_new_with_mnemonic (MENU_EDIT_REMOVE);
	GtkWidget *Separator1 = gtk_separator_menu_item_new ();
	GtkWidget *EnableAll = gtk_menu_item_new_with_mnemonic (MENU_EDIT_ENABLE_ALL);
	GtkWidget *Invert = gtk_menu_item_new_with_mnemonic (MENU_EDIT_INVERT);
	GtkWidget *DisableAll = gtk_menu_item_new_with_mnemonic (MENU_EDIT_DISABLE_ALL);
	GtkWidget *Separator2 = gtk_separator_menu_item_new ();
	GtkWidget *Preferences = gtk_menu_item_new_with_mnemonic (MENU_EDIT_PREFERENCES);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Insert));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Remove));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Separator1));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (EnableAll));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Invert));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (DisableAll));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Separator2));
	gtk_menu_shell_append (GTK_MENU_SHELL (editmenu), GTK_WIDGET (Preferences));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (Insert), "activate", GTK_ACCEL_GROUP (accelgroup), 'I', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Remove), "activate", GTK_ACCEL_GROUP (accelgroup), 'R', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (EnableAll), "activate", GTK_ACCEL_GROUP (accelgroup), 'E', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Invert), "activate", GTK_ACCEL_GROUP (accelgroup), 'X', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (DisableAll), "activate", GTK_ACCEL_GROUP (accelgroup), 'D', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Preferences), "activate", GTK_ACCEL_GROUP (accelgroup), 'P', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Insert), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Remove), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (EnableAll), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Invert), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (DisableAll), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Preferences), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (Insert), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Insert a new oscillator into the model"));
	g_signal_connect (G_OBJECT (Remove), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Remove selected oscillators from the model"));
	g_signal_connect (G_OBJECT (EnableAll), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Enable all oscillators"));
	g_signal_connect (G_OBJECT (Invert), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Invert selection mark for all oscillators"));
	g_signal_connect (G_OBJECT (DisableAll), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Disable all oscillators"));
	g_signal_connect (G_OBJECT (Preferences), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Change program preferences"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (Insert), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Remove), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (EnableAll), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Invert), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (DisableAll), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Preferences), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (Insert), "activate", G_CALLBACK (InsertOscillator), NULL);
	g_signal_connect (G_OBJECT (Remove), "activate", G_CALLBACK (RemoveOscillators), NULL);
	g_signal_connect (G_OBJECT (EnableAll), "activate", G_CALLBACK (EnableAllOscillators), NULL);
	g_signal_connect (G_OBJECT (Invert), "activate", G_CALLBACK (InvertSelection), NULL);
	g_signal_connect (G_OBJECT (DisableAll), "activate", G_CALLBACK (DisableAllOscillators), NULL);
	g_signal_connect (G_OBJECT (Preferences), "activate", G_CALLBACK (SetPreferences), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateEditMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic (MENU_EDIT);

	// Create submenu
	CreateEditSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (editmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "View" menu                                                         //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateViewSubMenu (void)
{
	// Create submenu
	viewmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *Oscill = gtk_check_menu_item_new_with_label (MENU_VIEW_OSCILLATORS);
	GtkWidget *RWave = gtk_check_menu_item_new_with_label (MENU_VIEW_RWAVE);
	GtkWidget *IWave = gtk_check_menu_item_new_with_label (MENU_VIEW_IWAVE);
	GtkWidget *Envelope = gtk_check_menu_item_new_with_label (MENU_VIEW_ENVELOPE);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (viewmenu), GTK_WIDGET (Oscill));
	gtk_menu_shell_append (GTK_MENU_SHELL (viewmenu), GTK_WIDGET (RWave));
	gtk_menu_shell_append (GTK_MENU_SHELL (viewmenu), GTK_WIDGET (IWave));
	gtk_menu_shell_append (GTK_MENU_SHELL (viewmenu), GTK_WIDGET (Envelope));

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Oscill), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (RWave), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (IWave), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Envelope), TRUE);

	// Apply default selection settings
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (Oscill), vOscillators);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (RWave), vRwave);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (IWave), vIwave);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (Envelope), vEnvelope);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (Oscill), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Show or hide oscillators"));
	g_signal_connect (G_OBJECT (RWave), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Show or hide running waves"));
	g_signal_connect (G_OBJECT (IWave), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Show or hide interference wave"));
	g_signal_connect (G_OBJECT (Envelope), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Show or hide envelope"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (Oscill), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (RWave), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (IWave), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Envelope), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (Oscill), "toggled", G_CALLBACK (ViewOscillators), NULL);
	g_signal_connect (G_OBJECT (RWave), "toggled", G_CALLBACK (ViewRunningWaves), NULL);
	g_signal_connect (G_OBJECT (IWave), "toggled", G_CALLBACK (ViewInterferenceWave), NULL);
	g_signal_connect (G_OBJECT (Envelope), "toggled", G_CALLBACK (ViewEnvelopeWave), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateViewMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic (MENU_VIEW);

	// Create submenu
	CreateViewSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (viewmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Simulation" menu                                                     //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateSimulationSubMenu (void)
{
	// Create submenu
	simulationmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *Reset = gtk_menu_item_new_with_mnemonic (MENU_SIMULATION_RESET);
	GtkWidget *Separator = gtk_separator_menu_item_new ();
	GtkWidget *Backward = gtk_menu_item_new_with_mnemonic (MENU_SIMULATION_BACKWARD);
	GtkWidget *Forward = gtk_menu_item_new_with_mnemonic (MENU_SIMULATION_FORWARD);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (simulationmenu), GTK_WIDGET (Reset));
	gtk_menu_shell_append (GTK_MENU_SHELL (simulationmenu), GTK_WIDGET (Separator));
	gtk_menu_shell_append (GTK_MENU_SHELL (simulationmenu), GTK_WIDGET (Backward));
	gtk_menu_shell_append (GTK_MENU_SHELL (simulationmenu), GTK_WIDGET (Forward));

	// Add accelerators to menu buttons
	gtk_widget_add_accelerator (GTK_WIDGET (Reset), "activate", GTK_ACCEL_GROUP (accelgroup), '0', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Backward), "activate", GTK_ACCEL_GROUP (accelgroup), '[', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (GTK_WIDGET (Forward), "activate", GTK_ACCEL_GROUP (accelgroup), ']', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Reset), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Backward), TRUE);
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (Forward), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (Reset), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Restart the simulation process"));
	g_signal_connect (G_OBJECT (Backward), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Do one step backward in time"));
	g_signal_connect (G_OBJECT (Forward), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Do one step forward in time"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (Reset), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Backward), "deselect", G_CALLBACK (MenuDeselect), NULL);
	g_signal_connect (G_OBJECT (Forward), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (Reset), "activate", G_CALLBACK (ResetSimulation), NULL);
	g_signal_connect (G_OBJECT (Backward), "activate", G_CALLBACK (BackwardStep), NULL);
	g_signal_connect (G_OBJECT (Forward), "activate", G_CALLBACK (ForwardStep), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateSimulationMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic (MENU_SIMULATION);

	// Create submenu
	CreateSimulationSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (simulationmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      "Help" menu                                                           //
//****************************************************************************//

//============================================================================//
//      Submenu buttons                                                       //
//============================================================================//
static void CreateHelpSubMenu (void)
{
	// Create submenu
	helpmenu = gtk_menu_new ();

	// Create submenu elements
	GtkWidget *About = gtk_menu_item_new_with_mnemonic (MENU_HELP_ABOUT);

	// Add elements to submenu
	gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), GTK_WIDGET (About));

	// Set submenu element properties
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (About), TRUE);

	// Assign signal handlers for "select" signal
	g_signal_connect (G_OBJECT (About), "select", G_CALLBACK (MenuSelect), const_cast <char*> ("Information about this program"));

	// Assign signal handlers for "deselect" signal
	g_signal_connect (G_OBJECT (About), "deselect", G_CALLBACK (MenuDeselect), NULL);

	// Assign signal handlers for "activate" signal
	g_signal_connect (G_OBJECT (About), "activate", G_CALLBACK (AboutDialog), NULL);
}

//============================================================================//
//      Menu button                                                           //
//============================================================================//
static GtkWidget* CreateHelpMenu (void)
{
	// Create menu
	GtkWidget *menu = gtk_menu_item_new_with_mnemonic (MENU_HELP);

	// Create submenu
	CreateHelpSubMenu ();

	// Add submenu
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), GTK_WIDGET (helpmenu));

	// Return menu object
	return menu;
}

//****************************************************************************//
//      Create menu bar                                                       //
//****************************************************************************//
GtkWidget* CreateMenuBar (void)
{
	// Create menu bar
	GtkWidget *menubar = gtk_menu_bar_new ();

	// Add menu buttons
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateFileMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateEditMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateViewMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateSimulationMenu ()));
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (CreateHelpMenu ()));

	// Return menu bar object
	return menubar;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
