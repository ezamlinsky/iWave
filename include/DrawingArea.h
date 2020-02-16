/*                                                                 DrawingArea.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 DRAWING AREA                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Default colors                                                        //
//****************************************************************************//
# define	COLOR_BACKGROUND	"#438FC3"		// Background color
# define	COLOR_TEXT			"#FFFFFF"		// Text color
# define	COLOR_BORDER		"#94C1EA"		// Border color
# define	COLOR_LINE			"#7CB0DF"		// Lines color
# define	COLOR_RWAVE			"#FFFFFF"		// Running waves color
# define	COLOR_IWAVE			"#FF0000"		// Interference wave color
# define	COLOR_ENVELOPE		"#FFFFFF"		// Envelope color

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void InitEnvelopeArrays (void);
GtkWidget* CreateDrawingArea (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
