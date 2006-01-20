/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QBHELPWINDOW_H
#define QBHELPWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>
#include <qstringlist.h>

#include <qbanking/qbhelpbrowser.ui.h>
#include <qbanking/qbanking.h>


class QComboBox;
class QPopupMenu;

class QBANKING_API QBHelpBrowser : public QBHelpBrowserUi {
  Q_OBJECT
public:
  QBHelpBrowser(const QString& home,
                const QStringList& paths,
                QWidget* parent = 0,
                const char *name=0);
  ~QBHelpBrowser();

  void setFilePaths(const QStringList& pathList);

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
