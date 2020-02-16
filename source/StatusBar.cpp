/*                                                                 StatusBar.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  STATUS BAR                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# include	<StatusBar.h>

//****************************************************************************//
//      External objects                                                      //
//****************************************************************************//
extern GtkWidget		*statusbar;			// Status bar

//****************************************************************************//
//      External variables                                                    //
//****************************************************************************//
extern guint			menuid;				// Status bar context identifier for menu messages
extern guint			fileid;				// Status bar context identifier for file messages

//****************************************************************************//
//      Show status message                                                   //
//****************************************************************************//
void ShowStatusMessage (const gchar *message, guint context)
{
	// Remove all messages from status bar
	gtk_statusbar_remove_all (GTK_STATUSBAR (statusbar), context);

	// Push new message into status bar stack
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context, message);
}

//****************************************************************************//
//      Create status bar                                                     //
//****************************************************************************//
GtkWidget* CreateStatusBar (void)
{
	// Create status bar
	statusbar = gtk_statusbar_new ();

	// Get context identifier for menu messages
	menuid = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Menu");
	fileid = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "File");

	// Return status bar object
	return statusbar;
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
