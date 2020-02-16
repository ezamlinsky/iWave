/*                                                               DrawingArea.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 DRAWING AREA                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<cmath>
# include	<TreeView.h>
# include	<DrawingArea.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	ARRAY_SIZE		8094			// Array size for interference wave
# define	BUFFER_SIZE		512				// Static buffer size
# define	TEXT_SIZE		10				// Text size
# define	FONT_FAMILY		"Sans"			// Font family
# define	SPACEY			32				// Space for Y labels
# define	SPACEX			24				// Space for X labels

//****************************************************************************//
//      External objects                                                      //
//****************************************************************************//
extern GtkWidget 		*scale;				// Scale
extern GtkWidget		*drawing;			// Drawing area
extern GtkWidget		*treeview;			// Tree view
extern GtkToolItem 		*Run;				// Run button

//****************************************************************************//
//      External variables                                                    //
//****************************************************************************//
extern gboolean			vOscillators;		// View oscillators flag
extern gboolean			vRwave;				// View running waves flag
extern gboolean			vIwave;				// View interference wave flag
extern gboolean			vEnvelope;			// View envelope flag

//============================================================================//
//      Animation settings                                                    //
//============================================================================//
extern gfloat 			timestamp;			// Current timestamp

//============================================================================//
//      Current viewpoint                                                     //
//============================================================================//
extern gfloat			startX;				// Begining X coordinate
extern gfloat			endX;				// Ending X coordinate
extern gfloat			startY;				// Begining Y coordinate
extern gfloat			endY;				// Ending Y coordinate

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
//      Local variables                                                       //
//****************************************************************************//
gfloat interference_wave [ARRAY_SIZE];		// Interference wave array
gfloat min_value [ARRAY_SIZE];				// Min value array
gfloat max_value [ARRAY_SIZE];				// Max value array

//****************************************************************************//
//      Radians to degrees function                                           //
//****************************************************************************//
gfloat RadToDeg (gfloat angle)
{
	if (angle < 0)
		return (2 * M_PI + fmod (angle, 2 * M_PI)) * 180 / M_PI;
	else
		return fmod (angle, 2 * M_PI) * 180 / M_PI;
}

//****************************************************************************//
//      Init function                                                         //
//****************************************************************************//
void InitArray (gfloat array[], gsize size)
{
	for (gsize i = 0; i < size; i++)
		array[i] = 0;
}

//****************************************************************************//
//      Init envelope arrays                                                  //
//****************************************************************************//
void InitEnvelopeArrays (void)
{
	// Init min value array
	InitArray (min_value, ARRAY_SIZE);

	// Init max value array
	InitArray (max_value, ARRAY_SIZE);
}

//****************************************************************************//
//      Simulation function                                                   //
//****************************************************************************//
gboolean Simulate (GtkWidget *widget)
{
	// Check if simulation is running
	if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (Run)))
	{
		// Go to next timestamp
		timestamp += gtk_range_get_value (GTK_RANGE (scale));

		// Redraw drawing area
		gtk_widget_queue_draw (GTK_WIDGET (drawing));
	}

	// Return simmualtion status
	return TRUE;
}

//****************************************************************************//
//      Signal handlers                                                       //
//****************************************************************************//

//============================================================================//
//      Signal handler for "draw" event                                       //
//============================================================================//
# include <cstdio>
static gboolean DrawGraph (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	/////////////////////////////////////////
	// TODO: Удалить этот блок
	startX = -8.1;
	endX = 8.1;
	startY = -5.1;
	endY = 5.1;
	/////////////////////////////////////////

	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Create text extent structure
	cairo_text_extents_t extent;

	// Get widget parameters
	gint width = gtk_widget_get_allocated_width (GTK_WIDGET (widget));
	gint height = gtk_widget_get_allocated_height (GTK_WIDGET (widget));

	// Init envelope arrays on window resize
	static gint last_width;
	if (last_width != width)
		InitEnvelopeArrays ();
	last_width = width;

	// Compute scale values
	gfloat hscale = (width - SPACEY) / (endX - startX);
	gfloat vscale = (height - SPACEX) / (endY - startY);

	// Set background
	cairo_set_source_rgb (cr, background.red, background.green, background.blue);
	cairo_paint (cr);

	// Set lines style
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

	// Draw vertical and horizontal sub lines
	cairo_set_source_rgb (cr, line.red, line.green, line.blue);
	cairo_set_line_width (cr, 1.0);
	for (gfloat x = ceil (startX * 5) / 5; x <= floor (endX * 5) / 5; x += 0.2)
	{
		gfloat value = (x - startX) * hscale + SPACEY;
		cairo_move_to (cr, value, 0);
		cairo_line_to (cr, value, height - SPACEX);
	}
	for (gfloat y = ceil (startY * 5) / 5; y <= floor (endY * 5) / 5; y += 0.2)
	{
		gfloat value = height - SPACEX - (y - startY) * vscale;
		cairo_move_to (cr, SPACEY, value);
		cairo_line_to (cr, width, value);
	}
	cairo_stroke (cr);

	// Draw vertical and horizontal main lines
	cairo_set_line_width (cr, 2.0);
	for (gfloat x = ceil (startX); x <= floor (endX); x += 1.0)
	{
		gfloat value = (x - startX) * hscale + SPACEY;
		cairo_move_to (cr, value, 0);
		cairo_line_to (cr, value, height - SPACEX + 3);
	}
	for (gfloat y = ceil (startY); y <= floor (endY); y += 1.0)
	{
		gfloat value = height - SPACEX - (y - startY) * vscale;
		cairo_move_to (cr, SPACEY - 3, value);
		cairo_line_to (cr, width, value);
	}
	cairo_stroke (cr);

	// Draw vertical and horizontal line labels
	cairo_set_source_rgb (cr, text.red, text.green, text.blue);
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, TEXT_SIZE);
	for (gfloat x = ceil (startX); x <= floor (endX); x += 1.0)
	{
		g_snprintf (buffer, BUFFER_SIZE, "%+.1f", x);
		cairo_text_extents (cr, buffer, &extent);
		cairo_move_to (cr, (x - startX) * hscale + SPACEY - extent.width + 8, height - extent.height);
		cairo_show_text (cr, buffer);
	}
	for (gfloat y = ceil (startY); y <= floor (endY); y += 1.0)
	{
		g_snprintf (buffer, BUFFER_SIZE, "%+.1f", y);
		cairo_text_extents (cr, buffer, &extent);
		cairo_move_to (cr, SPACEX - extent.width, height - SPACEX - (y - startY) * vscale + extent.height / 2);
		cairo_show_text (cr, buffer);
	}

	// Draw border lines and zero line
	cairo_set_source_rgb (cr, border.red, border.green, border.blue);
	cairo_set_line_width (cr, 2.0);
	cairo_move_to (cr, SPACEY, 0);
	cairo_line_to (cr, SPACEY, height - SPACEX);
	cairo_move_to (cr, SPACEY, height - SPACEX);
	cairo_line_to (cr, width, height - SPACEX);
	cairo_move_to (cr, SPACEY, height - SPACEX + startY * vscale);
	cairo_line_to (cr, width, height - SPACEX + startY * vscale);
	cairo_stroke (cr);

	// Get tree model object from tree view
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	if (model)
	{
		// Init interference wave array
		InitArray (interference_wave, ARRAY_SIZE);

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		{
			// Iterate through all elements
			do {
				// Get oscillator properties
				gboolean enable;
				gfloat position, magnitude, wavelength, phase;
				gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, OSCILLATOR_POSITION_ID, &position, OSCILLATOR_MAGNITUDE_ID, &magnitude, OSCILLATOR_WAVELENGTH_ID, &wavelength, OSCILLATOR_PHASE_ID, &phase, OSCILLATOR_ENABLE_ID, &enable, -1);
				if (enable)
				{
					// Convert phase to radians
					phase = timestamp / wavelength + phase * M_PI / 180;

					// Set running waves style
					cairo_set_antialias (cr, CAIRO_ANTIALIAS_SUBPIXEL);
					cairo_set_source_rgb (cr, rwave.red, rwave.green, rwave.blue);
					cairo_set_line_width (cr, 1.0);

					// Check if oscillators is visible
					gfloat curvalue = magnitude * sinf (phase);
					if (vOscillators)
					{
						cairo_arc (cr, (position - startX) * hscale + SPACEY, height - SPACEX - (curvalue - startY) * vscale, 0.1 * hscale, 0, 2 * M_PI);
						cairo_fill (cr);
					}

					// Compute running waves
					using namespace std;	// TODO: Удалить
					gint i = 0;
					gfloat arg = startX;
					gfloat y = magnitude * sinf (phase - 2.0 * M_PI / wavelength * abs (arg - position));
					interference_wave[0] += y;
					i++;
					if (vRwave)
					{
						gfloat value = height - SPACEX - (y - startY) * vscale;
						cairo_move_to (cr, SPACEY, value);
					}
					for (gfloat x = SPACEY + 1; x < width; x++)
					{
						arg = (x - SPACEY) / hscale + startX;
						y = magnitude * sinf (phase - 2.0 * M_PI / wavelength * abs (arg - position));
						interference_wave[i] += y;
						i++;
						if (vRwave)
						{
							gfloat value = height - SPACEX - (y - startY) * vscale;
							cairo_line_to (cr, x, value);
						}
					}
					if (vRwave)
						cairo_stroke (cr);

					// Update oscilator details
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, OSCILLATOR_CURVALUE_ID, curvalue, OSCILLATOR_CURPHASE_ID, RadToDeg (phase), -1);
				}

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

			// Check if envelope is visible
			if (vEnvelope)
			{
				// Set envelope style
				cairo_set_source_rgb (cr, envelope.red, envelope.green, envelope.blue);
				cairo_set_line_width (cr, 2.0);
				gfloat value;

				// Draw min envelope
				if (interference_wave[0] < min_value[0])
					min_value[0] = interference_wave[0];
				value = height - SPACEX - (min_value[0] - startY) * vscale;
				cairo_move_to (cr, SPACEY, value);
				for (guint x = SPACEY + 1; x < width; x++)
				{
					if (interference_wave[x - SPACEY] < min_value[x - SPACEY])
						min_value[x - SPACEY] = interference_wave[x - SPACEY];
					value = height - SPACEX - (min_value[x - SPACEY] - startY) * vscale;
					cairo_line_to (cr, x, value);
					// TODO: Придумать как обрезать нижний вывод по уровню
				}
				cairo_stroke (cr);

				// Draw max envelope
				if (interference_wave[0] > max_value[0])
					max_value[0] = interference_wave[0];
				value = height - SPACEX - (max_value[0] - startY) * vscale;
				cairo_move_to (cr, SPACEY, value);
				for (guint x = SPACEY + 1; x < width; x++)
				{
					if (interference_wave[x - SPACEY] > max_value[x - SPACEY])
						max_value[x - SPACEY] = interference_wave[x - SPACEY];
					value = height - SPACEX - (max_value[x - SPACEY] - startY) * vscale;
					cairo_line_to (cr, x, value);
				}
				cairo_stroke (cr);
			}

			// Check if interference wave is visible
			if (vIwave)
			{
				// Set interference waves style
				cairo_set_source_rgb (cr, iwave.red, iwave.green, iwave.blue);
				cairo_set_line_width (cr, 2.0);

				// Draw interference wave
				gfloat value = height - SPACEX - (interference_wave[0] - startY) * vscale;
				cairo_move_to (cr, SPACEY, value);
				for (guint x = SPACEY + 1; x < width; x++)
				{
					value = height - SPACEX - (interference_wave[x - SPACEY] - startY) * vscale;
					cairo_line_to (cr, x, value);
				}
				cairo_stroke (cr);
			}
		}
	}

	// Return process event status
	return FALSE;
}

//****************************************************************************//
//      Create drawing area                                                   //
//****************************************************************************//
GtkWidget* CreateDrawingArea (void)
{
	// Create drawing area
	drawing = gtk_drawing_area_new ();

	// Set drawing area properties
	gtk_widget_set_size_request (GTK_WIDGET (drawing), 1000, 500);

	// Set drawing colors
	gdk_rgba_parse  (&background, COLOR_BACKGROUND);
	gdk_rgba_parse  (&text, COLOR_TEXT);
	gdk_rgba_parse  (&border, COLOR_BORDER);
	gdk_rgba_parse  (&line, COLOR_LINE);
	gdk_rgba_parse  (&rwave, COLOR_RWAVE);
	gdk_rgba_parse  (&iwave, COLOR_IWAVE);
	gdk_rgba_parse  (&envelope, COLOR_ENVELOPE);

	// Set draw function
	g_timeout_add (40, (GSourceFunc) Simulate, NULL);

	// Assign signal handlers
	g_signal_connect (G_OBJECT (drawing), "draw", G_CALLBACK (DrawGraph), NULL);

	// Return drawing area
	return drawing;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
