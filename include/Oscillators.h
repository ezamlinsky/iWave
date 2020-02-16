/*                                                                 Oscillators.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                              OSCILLATORS CLASS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Oscillators class                                                     //
//****************************************************************************//
class Oscillators
{
private:
	GtkListStore	*list;			// Oscillator list

public:

	// Constructor and destructor
	Oscillators (void);
	~Oscillators (void);

	// Create new oscillator list
	void NewList (void);

	// Clear oscillator list
	void ClearList (void);

	// Oscillator list opening and saving
	gboolean OpenList (const gchar *fname, GError **error);		// TODO: написать
	gboolean SaveList (const gchar *fname, GError **error);		// TODO: написать

	// Oscillator list
	GtkListStore* GetOscillatorList (void) const;
};
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
