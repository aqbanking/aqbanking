/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>

#include <qbanking/qbhelpbrowser.ui.h>


class QComboBox;
class QPopupMenu;

class QBHelpBrowser : public QBHelpBrowserUi {
  Q_OBJECT
public:
  QBHelpBrowser(const QString& home,
                const QString& path,
                QWidget* parent = 0,
                const char *name=0);
  ~QBHelpBrowser();

private slots:
  void setBackwardAvailable(bool );
  void setForwardAvailable(bool );

  void slotSourceChanged(const QString& );
  void slotAbout();
  void slotAboutQt();
  void slotNewWindow();
  void slotPrint();
  void slotAddBookmark();

private:
  int _backwardId, _forwardId;
  QStringList _history, _bookmarks;
  QMap<int, QString> _mHistory, _mBookmarks;
  QPopupMenu *_hist, *_bookm;

};





#endif
