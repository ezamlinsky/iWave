/*                                                                   ToolBar.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                   TOOL BAR                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<Common.h>
# include	<MenuBar.h>
# include	<ToolBar.h>

//****************************************************************************//
//      External objects                                                      //
//****************************************************************************//
extern GtkWidget		*toolbar;			// Tool bar
extern GtkWidget 		*scale;				// Scale
extern GtkToolItem 		*Run;				// Run button

//****************************************************************************//
//      Change tool buttons sensitivity                                       //
//****************************************************************************//
void ChangeToolButtonsSensitivity (GtkWidget *widget, gpointer data)
{
	// Check if widget is tool button
	if (GTK_IS_TOOL_BUTTON (widget))
	{
		// Get tool button label
		const gchar *label = gtk_tool_button_get_label (GTK_TOOL_BUTTON (widget));

		// Convert data pointer
		LabelArray *larray = reinterpret_cast <LabelArray*> (data);

		// Get label array properties
		GArray *array = larray -> array;
		guint size = array -> len;
		gboolean state = larray -> state;

		// Iterate through all elements
		for (guint i = 0; i < size; i++)
		{
			// Check if button label match to array element
			if (g_utf8_collate (label, g_array_index (array, gchar*, i)) == 0)
			{
				// Change sensitivity state of the button
				gtk_widget_set_sensitive (GTK_WIDGET (widget), state);
			}
		}
	}
}

//****************************************************************************//
//      Signal handlers                                                       //
//****************************************************************************//

//============================================================================//
//      Signal handler for "Run" tool button                                  //
//============================================================================//
void RunSimulation (GtkToggleToolButton *button, gpointer user_data)
{
	// Get run status
	gboolean run = gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (button));

	// Change button look
	if (run)
	{
		gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "media-playback-pause");
		gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), TOOL_PAUSE);
	}
	else
	{
		gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "media-playback-start");
		gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), TOOL_RUN);
	}

	// Change run state
	ChangeRunState (run);
}

//****************************************************************************//
//      Create tool bar                                                       //
//****************************************************************************//
static GtkWidget* CreateToolBar (void)
{
	// Create tool bar
	toolbar = gtk_toolbar_new ();

	// Create buttons
	GtkToolItem *New = gtk_tool_button_new (NULL, TOOL_NEW);
	GtkToolItem *Open = gtk_tool_button_new (NULL, TOOL_OPEN);
	GtkToolItem *Save = gtk_tool_button_new (NULL, TOOL_SAVE);
	GtkToolItem *Separator1 = gtk_separator_tool_item_new();
	GtkToolItem *Close = gtk_tool_button_new (NULL, TOOL_CLOSE);
	GtkToolItem *Separator2 = gtk_separator_tool_item_new();
	GtkToolItem *Insert = gtk_tool_button_new (NULL, TOOL_INSERT);
	GtkToolItem *Remove = gtk_tool_button_new (NULL, TOOL_REMOVE);
	GtkToolItem *Separator3 = gtk_separator_tool_item_new();
	GtkToolItem *Reset = gtk_tool_button_new (NULL, TOOL_RESET);
	GtkToolItem *Backward = gtk_tool_button_new (NULL, TOOL_BACKWARD);
	GtkToolItem *Forward = gtk_tool_button_new (NULL, TOOL_FORWARD);
	GtkToolItem *Separator4 = gtk_separator_tool_item_new();
	GtkToolItem *Preferences = gtk_tool_button_new (NULL, TOOL_PREFERENCES);
	Run = gtk_toggle_tool_button_new ();

	// Add buttons to tool bar
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (New), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Open), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Save), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator1), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Close), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator2), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Insert), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Remove), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator3), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Reset), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Run), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Backward), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Forward), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Separator4), -1);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (Preferences), -1);

	// Add tooltip text to buttons
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (New), "Create new model");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Open), "Open existing model");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Save), "Save model into the file");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Close), "Close current model");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Insert), "Insert a new oscillator into the model");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Remove), "Remove selected oscillators from the model");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Reset), "Restart the simulation process");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Run), "Run or pause the simmulation process");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Backward), "Do one step backward in time");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Forward), "Do one step forward in time");
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (Preferences), "Change program preferences");

	// Set button properties
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (New), "document-new");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Open), "document-open");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Save), "document-save");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Close), "window-close");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Insert), "list-add");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Remove), "list-remove");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Reset), "view-refresh");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Run), "media-playback-start");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Backward), "media-skip-backward");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Forward), "media-skip-forward");
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (Preferences), "document-properties");
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (Run), TOOL_RUN);

	// Set tool bar properties
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (New), "clicked", G_CALLBACK (NewModel), NULL);
	g_signal_connect (G_OBJECT (Open), "clicked", G_CALLBACK (OpenModel), NULL);
	g_signal_connect (G_OBJECT (Save), "clicked", G_CALLBACK (SaveModel), NULL);
	g_signal_connect (G_OBJECT (Close), "clicked", G_CALLBACK (CloseDocument), NULL);
	g_signal_connect (G_OBJECT (Insert), "clicked", G_CALLBACK (InsertOscillator), NULL);
	g_signal_connect (G_OBJECT (Remove), "clicked", G_CALLBACK (RemoveOscillators), NULL);
	g_signal_connect (G_OBJECT (Reset), "clicked", G_CALLBACK (ResetSimulation), NULL);
	g_signal_connect (G_OBJECT (Run), "toggled", G_CALLBACK (RunSimulation), NULL);
	g_signal_connect (G_OBJECT (Backward), "clicked", G_CALLBACK (BackwardStep), NULL);
	g_signal_connect (G_OBJECT (Forward), "clicked", G_CALLBACK (ForwardStep), NULL);
	g_signal_connect (G_OBJECT (Preferences), "clicked", G_CALLBACK (SetPreferences), NULL);

	// Return tool bar
	return toolbar;
}

//****************************************************************************//
//      Create scale                                                          //
//****************************************************************************//
static GtkWidget* CreateScale (void)
{
	// Create scale
	scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, SPEED_MIN, SPEED_MAX, SPEED_STEP);

	// Set scale properties
	gtk_scale_set_digits (GTK_SCALE (scale), SPEED_DIGITS);
	gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
	gtk_scale_set_has_origin (GTK_SCALE (scale), TRUE);
	gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_RIGHT);

	// Return scale
	return scale;
}

//****************************************************************************//
//      Create tool layout                                                    //
//****************************************************************************//
GtkWidget* CreateToolLayout (void)
{
	// Create box
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	// Add items to box
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateToolBar ()), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (CreateScale ()), TRUE, TRUE, 0);

	// Return box object
	return box;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
