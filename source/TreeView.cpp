/*                                                                  TreeView.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  TREE VIEW                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<cstdlib>
# include	<Common.h>
# include	<MenuBar.h>
# include	<TreeView.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	BUFFER_SIZE		512				// Static buffer size
# define	COLOR_INACTIVE	"#EEEEEE"		// Background color of inactive columns

//****************************************************************************//
//      External objects                                                      //
//****************************************************************************//
extern GtkWidget		*window;			// Main window
extern GtkWidget		*treeview;			// Tree view

//****************************************************************************//
//      Check field limits                                                    //
//****************************************************************************//
bool CheckValue (const gchar *field, gfloat min, gfloat max, gfloat value, GError **error)
{
	// Check if value is in range
	if (value < min || value > max)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Oscilator %s value '%.2f' is out of range [%.2f,%.2f]", value, min, max, field);
		return FALSE;
	}

	// Normal exit
	return TRUE;
}

//****************************************************************************//
//      Set sort column of oscillator list                                    //
//****************************************************************************//
void SetSortColumn (GtkTreeView *treeview, gint column, GtkSortType order)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Set search column for immediate search
		gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), column);

		// Sort oscillators by chosen field
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), column, order);
	}
}

//****************************************************************************//
//      Number render function                                                //
//****************************************************************************//
void NumberRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Get numeric value
	gfloat value;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_INT (data), &value, -1);

	// Format numeric value into string buffer
	switch (GPOINTER_TO_INT (data))
	{
		case OSCILLATOR_POSITION_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", POSITION_DIGITS, value);
			break;
		case OSCILLATOR_CURPOSITION_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", POSITION_DIGITS, value);
			break;
		case OSCILLATOR_MAGNITUDE_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", MAGNITUDE_DIGITS, value);
			break;
		case OSCILLATOR_CURVALUE_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", MAGNITUDE_DIGITS, value);
			break;
		case OSCILLATOR_WAVELENGTH_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", WAVELENGTH_DIGITS, value);
			break;
		case OSCILLATOR_PHASE_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", PHASE_DIGITS, value);
			break;
		case OSCILLATOR_BETA_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", BETA_DIGITS, value);
			break;
		default:
			g_snprintf (buffer, BUFFER_SIZE, "%.f", value);
	}

	// Set cell text
	g_object_set (G_OBJECT (cell), "text", buffer, NULL);
}

//****************************************************************************//
//      Progress render function                                              //
//****************************************************************************//
void ProgressRenderFunc (GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Progress value
	gint progress;

	// Get numeric value
	gfloat value, magnitude;
	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, GPOINTER_TO_INT (data), &value, OSCILLATOR_MAGNITUDE_ID, &magnitude, -1);

	// Format numeric value into string buffer
	switch (GPOINTER_TO_INT (data))
	{
		case OSCILLATOR_CURVALUE_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", MAGNITUDE_DIGITS, value);
			progress = 50 * (1.0 + value / magnitude);
			break;
		case OSCILLATOR_CURPHASE_ID:
			g_snprintf (buffer, BUFFER_SIZE, "%.*f", PHASE_DIGITS, value);
			progress = 100 * value / 360;
			break;
		default:
			g_snprintf (buffer, BUFFER_SIZE, "%.f", value);
	}

	// Set cell text
	g_object_set (G_OBJECT (cell), "text", buffer, NULL);

	// Set cell progress
	g_object_set (G_OBJECT (cell), "value", progress, NULL);
}

//****************************************************************************//
//      Signal handlers                                                       //
//****************************************************************************//

//============================================================================//
//      Signal handler for selection changed                                  //
//============================================================================//
static void SelectionChanged (GtkTreeSelection *selection, gpointer user_data)
{
	// Change selection state
	ChangeSelectionState (gtk_tree_selection_count_selected_rows (GTK_TREE_SELECTION (selection)));
}

//============================================================================//
//      Signal handler for column header click                                //
//============================================================================//
static void ColumnHandler (GtkTreeViewColumn *column, gpointer data)
{
	// Set search column for immediate search
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), GPOINTER_TO_UINT (data));
}

//============================================================================//
//      Signal handler for "Enable" cell                                      //
//============================================================================//
static void EnableCellHandler (GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get mark state
			gboolean state;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_INT (data), &state, -1);

			// Switch mark state
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_INT (data), !state, -1);

			// Change save state
			ChangeSavedState (FALSE);
		}
	}
}

//============================================================================//
//      Signal handler for other cells                                        //
//============================================================================//
static void CellHandler (GtkCellRendererText *cell, gchar *path, gchar *newval, gpointer data)
{
	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		{
			// Get old value
			gfloat oldval;
			gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GPOINTER_TO_INT (data), &oldval, -1);

			// Create error object
			GError *error = NULL;

			// Extract numeric value from string
			gfloat value = strtod (newval, NULL);

			// Check if oscillator value is correct
			bool status;
			switch (GPOINTER_TO_INT (data))
			{
				case OSCILLATOR_POSITION_ID:
					status = CheckValue ("position", POSITION_MIN, POSITION_MAX, value, &error);
					break;
				case OSCILLATOR_MAGNITUDE_ID:
					status = CheckValue ("magnitude", MAGNITUDE_MIN, MAGNITUDE_MAX, value, &error);
					break;
				case OSCILLATOR_PHASE_ID:
					status = CheckValue ("phase", PHASE_MIN, PHASE_MAX, value, &error);
					break;
				case OSCILLATOR_WAVELENGTH_ID:
					status = CheckValue ("wavelength", WAVELENGTH_MIN, WAVELENGTH_MAX, value, &error);
					break;
				case OSCILLATOR_BETA_ID:
					status = CheckValue ("beta", BETA_MIN, BETA_MAX, value, &error);
					break;
				default:
					status = TRUE;
			}

			// In case of an error show error message
			if (!status)
				ShowErrorMessage (GTK_WINDOW (window), "Wrong oscillator propertie", error);
			else
			{
				// Check if value is changed
				if (oldval != value)
				{
					// Set value
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, GPOINTER_TO_INT (data), value, -1);

					// Change save state
					ChangeSavedState (FALSE);
				}
			}
		}
	}
}

//****************************************************************************//
//      Create tree view                                                      //
//****************************************************************************//
GtkWidget* CreateTreeView (void)
{
	// Create tree view
	treeview = gtk_tree_view_new ();

	// Get tree view adjustments
	GtkAdjustment *PositionAdjustment = gtk_adjustment_new (0, POSITION_MIN, POSITION_MAX, POSITION_STEP, POSITION_PAGE, 0);
	GtkAdjustment *CurPositionAdjustment = gtk_adjustment_new (0, POSITION_MIN, POSITION_MAX, POSITION_STEP, POSITION_PAGE, 0);
	GtkAdjustment *MagnitudeAdjustment = gtk_adjustment_new (1.0, MAGNITUDE_MIN, MAGNITUDE_MAX, MAGNITUDE_STEP, MAGNITUDE_PAGE, 0);
	GtkAdjustment *PhaseAdjustment = gtk_adjustment_new (0, PHASE_MIN, PHASE_MAX, PHASE_STEP, PHASE_PAGE, 0);
	GtkAdjustment *WavelengthAdjustment = gtk_adjustment_new (2.0, WAVELENGTH_MIN, WAVELENGTH_MAX, WAVELENGTH_STEP, WAVELENGTH_PAGE, 0);
	GtkAdjustment *BetaAdjustment = gtk_adjustment_new (0, BETA_MIN, BETA_MAX, BETA_STEP, BETA_PAGE, 0);
	GtkAdjustment *vadjustment = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (treeview));
	GtkAdjustment *hadjustment = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (treeview));

	// Get tree view selection
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	// Create scrolled window
	GtkWidget *scrolled = gtk_scrolled_window_new (hadjustment, vadjustment);

	// Create cell renderers for columns
	GtkCellRenderer *EnableCell = gtk_cell_renderer_toggle_new ();
	GtkCellRenderer *PositionCell = gtk_cell_renderer_spin_new ();
	GtkCellRenderer *CurPositionCell = gtk_cell_renderer_spin_new ();
	GtkCellRenderer *MagnitudeCell = gtk_cell_renderer_spin_new ();
	GtkCellRenderer *CurValueCell = gtk_cell_renderer_progress_new ();
	GtkCellRenderer *PhaseCell = gtk_cell_renderer_spin_new ();
	GtkCellRenderer *CurPhaseCell = gtk_cell_renderer_progress_new ();
	GtkCellRenderer *WavelengthCell = gtk_cell_renderer_spin_new ();
	GtkCellRenderer *BetaCell = gtk_cell_renderer_spin_new ();

	// Create columns for tree view
	GtkTreeViewColumn *EnableColumn = gtk_tree_view_column_new_with_attributes ("Enable", GTK_CELL_RENDERER (EnableCell), "active", OSCILLATOR_ENABLE_ID, NULL);
	GtkTreeViewColumn *PositionColumn = gtk_tree_view_column_new_with_attributes ("Position", GTK_CELL_RENDERER (PositionCell), "text", OSCILLATOR_POSITION_ID, NULL);
	GtkTreeViewColumn *CurPositionColumn = gtk_tree_view_column_new_with_attributes ("Current position", GTK_CELL_RENDERER (CurPositionCell), "text", OSCILLATOR_CURPOSITION_ID, NULL);
	GtkTreeViewColumn *MagnitudeColumn = gtk_tree_view_column_new_with_attributes ("Magnitude", GTK_CELL_RENDERER (MagnitudeCell), "text", OSCILLATOR_MAGNITUDE_ID, NULL);
	GtkTreeViewColumn *CurValueColumn = gtk_tree_view_column_new_with_attributes ("Current value", GTK_CELL_RENDERER (CurValueCell), "text", OSCILLATOR_CURVALUE_ID, NULL);
	GtkTreeViewColumn *PhaseColumn = gtk_tree_view_column_new_with_attributes ("Initial phase", GTK_CELL_RENDERER (PhaseCell), "text", OSCILLATOR_PHASE_ID, NULL);
	GtkTreeViewColumn *CurPhaseColumn = gtk_tree_view_column_new_with_attributes ("Current phase", GTK_CELL_RENDERER (CurPhaseCell), "text", OSCILLATOR_CURPHASE_ID, NULL);
	GtkTreeViewColumn *WavelengthColumn = gtk_tree_view_column_new_with_attributes ("Wavelength", GTK_CELL_RENDERER (WavelengthCell), "text", OSCILLATOR_WAVELENGTH_ID, NULL);
	GtkTreeViewColumn *BetaColumn = gtk_tree_view_column_new_with_attributes ("Beta", GTK_CELL_RENDERER (BetaCell), "text", OSCILLATOR_BETA_ID, NULL);

	// Add columns to tree view
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (EnableColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (PositionColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CurPositionColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (MagnitudeColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CurValueColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (PhaseColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (CurPhaseColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (WavelengthColumn));
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (BetaColumn));

	// Add tree view to scrolled window
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	// Set cell properties
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (PositionCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (CurPositionCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (MagnitudeCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (CurValueCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (PhaseCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (CurPhaseCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (WavelengthCell), 1.0, 0.0);
	gtk_cell_renderer_set_alignment (GTK_CELL_RENDERER (BetaCell), 1.0, 0.0);
	g_object_set (G_OBJECT (PositionCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (CurPositionCell), "editable", FALSE, NULL);
	g_object_set (G_OBJECT (MagnitudeCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (PhaseCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (WavelengthCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (BetaCell), "editable", TRUE, NULL);
	g_object_set (G_OBJECT (PositionCell), "adjustment", PositionAdjustment, NULL);
	g_object_set (G_OBJECT (CurPositionCell), "adjustment", CurPositionAdjustment, NULL);
	g_object_set (G_OBJECT (MagnitudeCell), "adjustment", MagnitudeAdjustment, NULL);
	g_object_set (G_OBJECT (PhaseCell), "adjustment", PhaseAdjustment, NULL);
	g_object_set (G_OBJECT (WavelengthCell), "adjustment", WavelengthAdjustment, NULL);
	g_object_set (G_OBJECT (BetaCell), "adjustment", BetaAdjustment, NULL);
	g_object_set (G_OBJECT (PositionCell), "digits", POSITION_DIGITS, NULL);
	g_object_set (G_OBJECT (CurPositionCell), "digits", POSITION_DIGITS, NULL);
	g_object_set (G_OBJECT (MagnitudeCell), "digits", MAGNITUDE_DIGITS, NULL);
	g_object_set (G_OBJECT (PhaseCell), "digits", PHASE_DIGITS, NULL);
	g_object_set (G_OBJECT (WavelengthCell), "digits", WAVELENGTH_DIGITS, NULL);
	g_object_set (G_OBJECT (BetaCell), "digits", BETA_DIGITS, NULL);
	g_object_set (G_OBJECT (CurPositionCell), "cell-background", COLOR_INACTIVE, NULL);
	g_object_set (G_OBJECT (CurValueCell), "text-xalign", 0.95, NULL);
	g_object_set (G_OBJECT (CurPhaseCell), "text-xalign", 0.95, NULL);

	// Set column properties
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (EnableColumn), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (PositionColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CurPositionColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (MagnitudeColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CurValueColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (PhaseColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (CurPhaseColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (WavelengthColumn), TRUE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (BetaColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (EnableColumn), FALSE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (PositionColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CurPositionColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (MagnitudeColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CurValueColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (PhaseColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (CurPhaseColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (WavelengthColumn), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (BetaColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (EnableColumn), FALSE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (PositionColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CurPositionColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (MagnitudeColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CurValueColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (PhaseColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (CurPhaseColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (WavelengthColumn), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN (BetaColumn), TRUE);

	// Set tree view sort properties
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (EnableColumn), OSCILLATOR_ENABLE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (PositionColumn), OSCILLATOR_POSITION_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (CurPositionColumn), OSCILLATOR_CURPOSITION_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (MagnitudeColumn), OSCILLATOR_MAGNITUDE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (CurValueColumn), OSCILLATOR_CURVALUE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (PhaseColumn), OSCILLATOR_PHASE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (CurPhaseColumn), OSCILLATOR_CURPHASE_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (WavelengthColumn), OSCILLATOR_WAVELENGTH_ID);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN (BetaColumn), OSCILLATOR_BETA_ID);

	// Set tree view properties
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

	// Set scrolled window preperties
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement (GTK_SCROLLED_WINDOW (scrolled), GTK_CORNER_TOP_LEFT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_NONE);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled), 150);

	// Set selection properties
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);

	// Assign cell data functions
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (PositionColumn), GTK_CELL_RENDERER (PositionCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_POSITION_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (CurPositionColumn), GTK_CELL_RENDERER (CurPositionCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_CURPOSITION_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (MagnitudeColumn), GTK_CELL_RENDERER (MagnitudeCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_MAGNITUDE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (CurValueColumn), GTK_CELL_RENDERER (CurValueCell), ProgressRenderFunc, GINT_TO_POINTER (OSCILLATOR_CURVALUE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (PhaseColumn), GTK_CELL_RENDERER (PhaseCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_PHASE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (CurPhaseColumn), GTK_CELL_RENDERER (CurPhaseCell), ProgressRenderFunc, GINT_TO_POINTER (OSCILLATOR_CURPHASE_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (WavelengthColumn), GTK_CELL_RENDERER (WavelengthCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_WAVELENGTH_ID), NULL);
	gtk_tree_view_column_set_cell_data_func (GTK_TREE_VIEW_COLUMN (BetaColumn), GTK_CELL_RENDERER (BetaCell), NumberRenderFunc, GINT_TO_POINTER (OSCILLATOR_BETA_ID), NULL);

	// Assign signal handlers for selection
	g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (SelectionChanged), NULL);

	// Assign signal handlers for cells
	g_signal_connect (G_OBJECT (EnableCell), "toggled", G_CALLBACK (EnableCellHandler), GUINT_TO_POINTER (OSCILLATOR_ENABLE_ID));
	g_signal_connect (G_OBJECT (PositionCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (OSCILLATOR_POSITION_ID));
	g_signal_connect (G_OBJECT (MagnitudeCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (OSCILLATOR_MAGNITUDE_ID));
	g_signal_connect (G_OBJECT (PhaseCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (OSCILLATOR_PHASE_ID));
	g_signal_connect (G_OBJECT (WavelengthCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (OSCILLATOR_WAVELENGTH_ID));
	g_signal_connect (G_OBJECT (BetaCell), "edited", G_CALLBACK (CellHandler), GUINT_TO_POINTER (OSCILLATOR_BETA_ID));

	// Assign signal handlers for columns
	g_signal_connect (G_OBJECT (PositionColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_POSITION_ID));
	g_signal_connect (G_OBJECT (CurPositionColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_CURPOSITION_ID));
	g_signal_connect (G_OBJECT (MagnitudeColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_MAGNITUDE_ID));
	g_signal_connect (G_OBJECT (CurValueColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_CURVALUE_ID));
	g_signal_connect (G_OBJECT (PhaseColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_PHASE_ID));
	g_signal_connect (G_OBJECT (CurPhaseColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_CURPHASE_ID));
	g_signal_connect (G_OBJECT (WavelengthColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_WAVELENGTH_ID));
	g_signal_connect (G_OBJECT (BetaColumn), "clicked", G_CALLBACK (ColumnHandler), GUINT_TO_POINTER (OSCILLATOR_BETA_ID));

	// Return scrolled window object
	return scrolled;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
