/*                                                                    TreeView.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  TREE VIEW                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Tree view constants                                                   //
//****************************************************************************//
# define	OSCILLATOR_COLUMNS			9	// Count of columns in oscilator list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	OSCILLATOR_POSITION_ID		0	// Oscillator position field id
# define	OSCILLATOR_CURPOSITION_ID	1	// Oscillator current position field id
# define	OSCILLATOR_MAGNITUDE_ID		2	// Oscillator magnitude field id
# define	OSCILLATOR_CURVALUE_ID		3	// Oscillator current value field id
# define	OSCILLATOR_PHASE_ID			4	// Oscillator initial phase field id
# define	OSCILLATOR_CURPHASE_ID		5	// Oscillator current phase field id
# define	OSCILLATOR_WAVELENGTH_ID	6	// Oscillator wavelength field id
# define	OSCILLATOR_BETA_ID			7	// Oscillator beta field id
# define	OSCILLATOR_ENABLE_ID		8	// Oscillator enable field id

//****************************************************************************//
//      Field value limits                                                    //
//****************************************************************************//

//============================================================================//
//      Position field limits                                                 //
//============================================================================//
# define	POSITION_DIGITS		2		// Amount of decimal digits position have
# define	POSITION_MIN		-40		// Min position value
# define	POSITION_MAX		+40		// Max position value
# define	POSITION_STEP		0.01	// Step increment for position changes
# define	POSITION_PAGE		1.00	// Page increment for position changes

//============================================================================//
//      Magnitude field limits                                                //
//============================================================================//
# define	MAGNITUDE_DIGITS	1		// Amount of decimal digits magnitude have
# define	MAGNITUDE_MIN		0		// Min magnitude value
# define	MAGNITUDE_MAX		10		// Max magnitude value
# define	MAGNITUDE_STEP		0.1		// Step increment for magnitude changes
# define	MAGNITUDE_PAGE		1.00	// Page increment for magnitude changes

//============================================================================//
//      Wavelength field limits                                               //
//============================================================================//
# define	WAVELENGTH_DIGITS	2		// Amount of decimal digits wavelength have
# define	WAVELENGTH_MIN		0.01	// Min wavelength value
# define	WAVELENGTH_MAX		100		// Max wavelength value
# define	WAVELENGTH_STEP		0.01	// Step increment for wavelength changes
# define	WAVELENGTH_PAGE		1.00	// Page increment for wavelength changes

//============================================================================//
//      Phase field limits                                                    //
//============================================================================//
# define	PHASE_DIGITS		2		// Amount of decimal digits phase have
# define	PHASE_MIN			0		// Min phase value
# define	PHASE_MAX			360		// Max phase value
# define	PHASE_STEP			0.01	// Step increment for phase changes
# define	PHASE_PAGE			1.00	// Page increment for phase changes

//============================================================================//
//      Beta field limits                                                     //
//============================================================================//
# define	BETA_DIGITS			2		// Amount of decimal digits beta have
# define	BETA_MIN			0		// Min beta value
# define	BETA_MAX			3		// Max beta value
# define	BETA_STEP			0.01	// Step increment for beta changes
# define	BETA_PAGE			1.00	// Page increment for beta changes

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
bool CheckValue (const gchar *field, gfloat min, gfloat max, gfloat value, GError **error);
void SetSortColumn (GtkTreeView *treeview, gint column, GtkSortType order);
GtkWidget* CreateTreeView (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
