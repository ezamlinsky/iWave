/*                                                                      Common.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                               COMMON FUNCTIONS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Global constants                                                      //
//****************************************************************************//
# define	PROGRAM_TITLE	"iWave 1D"
# define	PROGRAM_NAME	"Ivanov's Wave Visualizator 1D"
# define	LOGO_FILE		"/usr/share/icons/hicolor/scalable/apps/stock-filter.svg"

//****************************************************************************//
//      Label list structure                                                  //
//****************************************************************************//
struct LabelArray
{
	GArray		*array;						// Labels array
	gboolean	state;						// New button state
};

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void ChangeDocumentState (gchar *fname, gboolean state);
void ChangeSavedState (gboolean state);
void ChangeSelectionState (gboolean state);
void ChangeRunState (gboolean state);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
