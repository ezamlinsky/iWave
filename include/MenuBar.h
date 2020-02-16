/*                                                                     MenuBar.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                   MENU BAR                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Menu bar labels                                                       //
//****************************************************************************//

//============================================================================//
//      "File" menu labels                                                    //
//============================================================================//
# define	MENU_FILE					"_File"				// "File" menu button
# define	MENU_FILE_NEW				"_New"				// "New" menu button
# define	MENU_FILE_OPEN				"_Open"				// "Open" menu button
# define	MENU_FILE_SAVE				"_Save"				// "Save" menu button
# define	MENU_FILE_SAVE_AS			"Save _As..."		// "Save As" menu button
# define	MENU_FILE_CLOSE				"_Close"			// "Close" menu button
# define	MENU_FILE_QUIT				"_Quit"				// "Quit" menu button

//============================================================================//
//      "Edit" menu labels                                                    //
//============================================================================//
# define	MENU_EDIT					"_Edit"				// "Edit" menu button
# define	MENU_EDIT_INSERT			"_Insert"			// "Insert" menu button
# define	MENU_EDIT_REMOVE			"_Remove"			// "Remove" menu button
# define	MENU_EDIT_ENABLE_ALL		"_Enable All"		// "Enable All" menu button
# define	MENU_EDIT_INVERT			"I_nvert selection"	// "Invert selection" menu button
# define	MENU_EDIT_DISABLE_ALL		"_Disable All"		// "Disable All" menu button
# define	MENU_EDIT_PREFERENCES		"_Preferences"		// "Preferences" menu button

//============================================================================//
//      "View" menu labels                                                    //
//============================================================================//
# define	MENU_VIEW					"_View"				// "View" menu button
# define	MENU_VIEW_OSCILLATORS		"_Oscillators"		// "Oscillators" menu button
# define	MENU_VIEW_RWAVE				"_Running waves"	// "Running waves" menu button
# define	MENU_VIEW_IWAVE				"_Interference wave"// "Interference wave" menu button
# define	MENU_VIEW_ENVELOPE			"_Envelope"			// "Envelope" menu button

//============================================================================//
//      "Simulation" menu labels                                              //
//============================================================================//
# define	MENU_SIMULATION				"_Simulation"		// "Simulation" menu button
# define	MENU_SIMULATION_RESET		"Rese_t"			// "Reset" menu button
# define	MENU_SIMULATION_BACKWARD	"Step _backward"	// "Step backward" menu button
# define	MENU_SIMULATION_FORWARD		"Step _forward"		// "Step forward" menu button

//============================================================================//
//      "Help" menu labels                                                    //
//============================================================================//
# define	MENU_HELP					"_Help"				// "Help" menu button
# define	MENU_HELP_ABOUT				"_About"			// "About" menu button

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void ShowErrorMessage (GtkWindow *parent, const gchar *message, GError *error);
void ChangeMenuButtonsSensitivity (GtkWidget *widget, gpointer data);
void NewModel (void);
void OpenModel (void);
gboolean SaveModel (void);
gboolean CloseDocument (void);
void InsertOscillator (void);
void RemoveOscillators (void);
void SetPreferences (void);
void ResetSimulation (void);
void BackwardStep (void);
void ForwardStep (void);
void QuitProgram (void);
GtkWidget* CreateMenuBar (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
