/*                                                                     ToolBar.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                   TOOL BAR                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<cmath>
# include	<gtk/gtk.h>

//****************************************************************************//
//      Tool bar labels                                                       //
//****************************************************************************//
# define	TOOL_NEW			"New"			// "New" tool bar button
# define	TOOL_OPEN			"Open"			// "Open" tool bar button
# define	TOOL_SAVE			"Save"			// "Save" tool bar button
# define	TOOL_CLOSE			"Close"			// "Close" tool bar button
# define	TOOL_INSERT			"Insert"		// "Insert" tool bar button
# define	TOOL_REMOVE			"Remove"		// "Remove" tool bar button
# define	TOOL_RESET			"Reset"			// "Reset" tool bar button
# define	TOOL_RUN			"Run"			// "Run" tool bar button
# define	TOOL_PAUSE			"Pause"			// "Pause" tool bar button
# define	TOOL_BACKWARD		"Backward"		// "Backward" tool bar button
# define	TOOL_FORWARD		"Forward"		// "Forward" tool bar button
# define	TOOL_PREFERENCES	"Preferences"	// "Preferences" tool bar button

//****************************************************************************//
//      Simulation speed                                                      //
//****************************************************************************//
# define	SPEED_DIGITS		2			// Amount of decimal digits speed have
# define	SPEED_MIN			M_PI / 40	// Min speed value
# define	SPEED_MAX			M_PI / 4	// Max speed value
# define	SPEED_STEP			M_PI / 40	// Step increment for speed changes

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void ChangeToolButtonsSensitivity (GtkWidget *widget, gpointer data);
GtkWidget* CreateToolLayout (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
