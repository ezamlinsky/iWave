/*                                                                   StatusBar.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  STATUS BAR                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2017, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
void ShowStatusMessage (const gchar *message, guint context);
GtkWidget* CreateStatusBar (void);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
