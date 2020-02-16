/*                                                               Oscillators.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                              OSCILLATORS CLASS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<cmath>
# include	<cstdlib>
# include	<TreeView.h>
# include	<Oscillators.h>

//****************************************************************************//
//      Oscillator constants                                                  //
//****************************************************************************//

//============================================================================//
//      Oscillator list elements                                              //
//============================================================================//
# define	OSCILLATOR_DATA_TAG			"data"
# define	OSCILLATOR_OSCILLATOR_TAG	"oscillator"

//============================================================================//
//      Oscillator list attributes                                            //
//============================================================================//
# define	OSCILLATOR_POSITION_ATTR	"position"
# define	OSCILLATOR_MAGNITUDE_ATTR	"magnitude"
# define	OSCILLATOR_WAVELENGTH_ATTR	"wavelength"
# define	OSCILLATOR_PHASE_ATTR		"phase"
# define	OSCILLATOR_BETA_ATTR		"beta"
# define	OSCILLATOR_ENABLE_ATTR		"enabled"

//****************************************************************************//
//      Parse oscillator list element                                         //
//****************************************************************************//
static void ParseElement (GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer data, GError **error)
{
	// Check element name and go to appropriate code branch
	if (g_utf8_collate (element_name, OSCILLATOR_OSCILLATOR_TAG) == 0)
	{
		// Local variables
		gchar *position, *magnitude, *wavelength, *phase, *beta;
		gboolean enable;

		// Collect oscillator attributes
		if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error, G_MARKUP_COLLECT_STRING, OSCILLATOR_POSITION_ATTR, &position, G_MARKUP_COLLECT_STRING, OSCILLATOR_MAGNITUDE_ATTR, &magnitude, G_MARKUP_COLLECT_STRING, OSCILLATOR_WAVELENGTH_ATTR, &wavelength, G_MARKUP_COLLECT_STRING, OSCILLATOR_PHASE_ATTR, &phase, G_MARKUP_COLLECT_STRING, OSCILLATOR_BETA_ATTR, &beta, G_MARKUP_COLLECT_BOOLEAN, OSCILLATOR_ENABLE_ATTR, &enable, G_MARKUP_COLLECT_INVALID))
		{
			// Extract numeric values from strings
			gdouble position_val = strtod (position, NULL);
			gdouble magnitude_val = strtod (magnitude, NULL);
			gdouble wavelength_val = strtod (wavelength, NULL);
			gdouble phase_val = strtod (phase, NULL);
			gdouble beta_val = strtod (beta, NULL);

			// Check if oscillator position is correct
			if (!CheckValue (OSCILLATOR_POSITION_ATTR, POSITION_MIN, POSITION_MAX, position_val, error))
				return;

			// Check if oscillator magnitude is correct
			if (!CheckValue (OSCILLATOR_MAGNITUDE_ATTR, MAGNITUDE_MIN, MAGNITUDE_MAX, magnitude_val, error))
				return;

			// Check if oscillator wavelength is correct
			if (!CheckValue (OSCILLATOR_WAVELENGTH_ATTR, WAVELENGTH_MIN, WAVELENGTH_MAX, wavelength_val, error))
				return;

			// Check if oscillator phase is correct
			if (!CheckValue (OSCILLATOR_PHASE_ATTR, PHASE_MIN, PHASE_MAX, phase_val, error))
				return;

			// Check if oscillator beta is correct
			if (!CheckValue (OSCILLATOR_PHASE_ATTR, BETA_MIN, BETA_MAX, beta_val, error))
				return;

			// Compute initial oscillation state
			gfloat current_val = magnitude_val * sinf(phase_val * M_PI / 180);

			// Convert data pointer
			GtkListStore *list = reinterpret_cast <GtkListStore*> (data);

			// Add new element to list store object
			GtkTreeIter iter;
			gtk_list_store_append (GTK_LIST_STORE (list), &iter);
			gtk_list_store_set (GTK_LIST_STORE (list), &iter, OSCILLATOR_POSITION_ID, position_val, OSCILLATOR_CURPOSITION_ID, position_val, OSCILLATOR_MAGNITUDE_ID, magnitude_val, OSCILLATOR_CURVALUE_ID, current_val, OSCILLATOR_WAVELENGTH_ID, wavelength_val, OSCILLATOR_PHASE_ID, phase_val, OSCILLATOR_CURPHASE_ID, phase_val, OSCILLATOR_BETA_ID, beta_val, OSCILLATOR_ENABLE_ID, enable, -1);
		}
	}
	else if (g_utf8_collate (element_name, OSCILLATOR_DATA_TAG) == 0)
	{
		// Skip element processing
		return;
	}
	else
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Unknown element '%s'", element_name);
	}
}

//****************************************************************************//
//      Constructor                                                           //
//****************************************************************************//
Oscillators::Oscillators (void)
{
	// Set oscillator list to default value
	list = NULL;
}

//****************************************************************************//
//      Destructor                                                            //
//****************************************************************************//
Oscillators::~Oscillators (void)
{
	// Clear oscillator list
	this -> ClearList ();
}

//****************************************************************************//
//      Create new oscillator list                                            //
//****************************************************************************//
void Oscillators::NewList (void)
{
	// Free oscillator elements
	if (list)
	{
		// Clear oscillator list
		gtk_list_store_clear (list);

		// Decrement reference count to oscillator list
		g_object_unref (list);
	}

	// Set new oscillator elements
	list = gtk_list_store_new (OSCILLATOR_COLUMNS, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_BOOLEAN);

	// Sort oscillators by position
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list), OSCILLATOR_POSITION_ID, GTK_SORT_ASCENDING);
}

//****************************************************************************//
//      Close oscillator list                                                 //
//****************************************************************************//
void Oscillators::ClearList (void)
{
	// Free oscillator elements
	if (list)
	{
		// Clear oscillator list
		gtk_list_store_clear (list);

		// Decrement reference count to oscillator list
		g_object_unref (list);
	}

	// Set oscillator list to default value
	list = NULL;
}

//****************************************************************************//
//      Open oscillator list from the file                                    //
//****************************************************************************//
gboolean Oscillators::OpenList (const gchar *fname, GError **error)
{
	// Try to load file content into string buffer
	gchar *content;
	gsize bytes;
	if (g_file_get_contents (fname, &content, &bytes, error))
	{
		// Create new oscillator list
		GtkListStore *newlist = gtk_list_store_new (OSCILLATOR_COLUMNS, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_BOOLEAN);

		// Create XML parser
		GMarkupParser parser = {ParseElement, NULL, NULL, NULL, NULL};

		// Create new XML parser context
		GMarkupParseContext *context = g_markup_parse_context_new (&parser, G_MARKUP_PREFIX_ERROR_POSITION, newlist, NULL);

		// Try to parse XML document
		if (!g_markup_parse_context_parse (context, content, bytes, error) || !g_markup_parse_context_end_parse (context, error))
		{
			// Release XML parser context
			g_markup_parse_context_free (context);

			// Free temporary string buffers
			g_free (content);

			// Clear new oscillator list
			gtk_list_store_clear (newlist);

			// Decrement reference count to new oscillator list
			g_object_unref (newlist);

			// Return fail status
			return FALSE;
		}
		else
		{
			// Release XML parser context
			g_markup_parse_context_free (context);

			// Free temporary string buffer
			g_free (content);

			// Free oscillator elements
			if (list)
			{
				// Clear oscillator list
				gtk_list_store_clear (list);

				// Decrement reference count to oscillator list
				g_object_unref (list);
			}

			// Set new oscillator elements
			list = newlist;

			// Return success state
			return TRUE;
		}
	}

	// Return fail status
	return FALSE;
}

//****************************************************************************//
//      Save oscillator list into file                                        //
//****************************************************************************//
gboolean Oscillators::SaveList (const gchar *fname, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Check if oscillator list is set
	if (list)
	{
		// Start data section of XML file
		g_string_append_printf (string, "<" OSCILLATOR_DATA_TAG ">\n");

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
		{
			// Iterate through all elements
			do {
				// Get oscillator details
				gfloat position, magnitude, wavelength, phase, beta;
				gboolean enable;
				gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, OSCILLATOR_POSITION_ID, &position, OSCILLATOR_MAGNITUDE_ID, &magnitude, OSCILLATOR_WAVELENGTH_ID, &wavelength, OSCILLATOR_PHASE_ID, &phase, OSCILLATOR_BETA_ID, &beta, OSCILLATOR_ENABLE_ID, &enable, -1);

				// Escapes text according to XML rules
				gchar *text = g_markup_printf_escaped ("<" OSCILLATOR_OSCILLATOR_TAG " " OSCILLATOR_POSITION_ATTR "=\"%f\" " OSCILLATOR_MAGNITUDE_ATTR "=\"%f\" " OSCILLATOR_WAVELENGTH_ATTR "=\"%f\" " OSCILLATOR_PHASE_ATTR "=\"%f\" " OSCILLATOR_BETA_ATTR "=\"%f\" " OSCILLATOR_ENABLE_ATTR "=\"%i\"/>\n", position, magnitude, wavelength, phase, beta, enable);

				// Append oscillator information into string buffer
				string = g_string_append (string, text);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));
		}

		// End data section of XML file
		g_string_append_printf (string, "</" OSCILLATOR_DATA_TAG ">\n");
	}

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Get oscillator list                                                   //
//****************************************************************************//
GtkListStore* Oscillators::GetOscillatorList (void) const
{
	return list;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
