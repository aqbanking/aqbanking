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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qbhelpbrowser.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <qtextbrowser.h>

#include <ctype.h>


static const char * const back_xpm[] = {
  "32 32 2 1",
  " 	c None",
  ".	c #00FF00",
  "                .               ",
  "               ..               ",
  "              ...               ",
  "             ....               ",
  "            .....               ",
  "           ......               ",
  "          .......               ",
  "         ........               ",
  "        .........               ",
  "       ..........               ",
  "      ...........               ",
  "     ............               ",
  "    ............................",
  "   .............................",
  "  ..............................",
  " ...............................",
  " ...............................",
  "  ..............................",
  "   .............................",
  "    ............................",
  "     ............               ",
  "      ...........               ",
  "       ..........               ",
  "        .........               ",
  "         ........               ",
  "          .......               ",
  "           ......               ",
  "            .....               ",
  "             ....               ",
  "              ...               ",
  "               ..               ",
  "                .               "};

static const char * const forward_xpm[] = {
  "32 32 2 1",
  " 	c None",
  ".	c #00FF00",
  "               .                ",
  "               ..               ",
  "               ...              ",
  "               ....             ",
  "               .....            ",
  "               ......           ",
  "               .......          ",
  "               ........         ",
  "               .........        ",
  "               ..........       ",
  "               ...........      ",
  "               ............     ",
  "............................    ",
  ".............................   ",
  "..............................  ",
  "............................... ",
  "............................... ",
  "..............................  ",
  ".............................   ",
  "............................    ",
  "               ............     ",
  "               ...........      ",
  "               ..........       ",
  "               .........        ",
  "               ........         ",
  "               .......          ",
  "               ......           ",
  "               .....            ",
  "               ....             ",
  "               ...              ",
  "               ..               ",
  "               .                "};





QBHelpBrowser::QBHelpBrowser(const QString& home,
                             const QString& path,
                             QWidget* parent,
                             const char *name)
:QBHelpBrowserUi(parent, name, WDestructiveClose){
  textBrowser->mimeSourceFactory()->setFilePath(path);
  textBrowser->setFrameStyle( QFrame::Panel | QFrame::Sunken);
  connect(textBrowser, SIGNAL(sourceChanged(const QString&)),
          this, SLOT(slotSourceChanged(const QString&)));

  if (!home.isEmpty() )
    textBrowser->setSource(home);

  QPopupMenu* file = new QPopupMenu( this );
  file->insertItem( tr("&New Window"), this, SLOT( slotNewWindow() ), Qt::CTRL+Qt::Key_N );
  file->insertItem( tr("&Print"), this, SLOT( slotPrint() ), Qt::CTRL+Qt::Key_P );
  file->insertSeparator();
  file->insertItem( tr("&Close"), this, SLOT( close() ), Qt::CTRL+Qt::Key_Q );

  // The same three icons are used twice each.
  QIconSet icon_back(QPixmap((const char**)back_xpm));
  QIconSet icon_forward(QPixmap((const char**)forward_xpm));
  QIconSet icon_home( QPixmap("home.xpm"));

  QPopupMenu* go = new QPopupMenu( this );
  _backwardId=go->insertItem(icon_back,
                             tr("&Backward"),
                             textBrowser,
                             SLOT(backward()),
                             Qt::CTRL+Qt::Key_Left);
  _forwardId = go->insertItem(icon_forward,
                              tr("&Forward"),
                              textBrowser,
                              SLOT(forward()),
                              Qt::CTRL+Qt::Key_Right );
  go->insertItem( icon_home, tr("&Home"), textBrowser, SLOT( home() ) );

  QPopupMenu* help = new QPopupMenu( this );
  help->insertItem( tr("&About"), this, SLOT( slotAbout() ) );
  help->insertItem( tr("About &Qt"), this, SLOT( slotAboutQt() ) );

  _hist = new QPopupMenu( this );

  _bookm = new QPopupMenu( this );
  _bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( slotAddBookmark() ) );
  _bookm->insertSeparator();

  _menuBar->insertItem(tr("&File"), file );
  _menuBar->insertItem(tr("&Go"), go );
  _menuBar->insertItem(tr( "History" ), _hist );
  _menuBar->insertItem(tr( "Bookmarks" ), _bookm );
  _menuBar->insertSeparator();
  _menuBar->insertItem(tr("&Help"), help );

  _menuBar->setItemEnabled( _forwardId, FALSE);
  _menuBar->setItemEnabled( _backwardId, FALSE);
  connect(textBrowser, SIGNAL(backwardAvailable(bool)),
          this, SLOT(setBackwardAvailable(bool)) );
  connect( textBrowser, SIGNAL( forwardAvailable( bool ) ),
          this, SLOT( setForwardAvailable( bool ) ) );

  QToolButton* button;

  button = new QToolButton( icon_back, tr("Backward"), "", textBrowser, SLOT(backward()), _toolBar );
  connect( textBrowser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
  button->setEnabled( FALSE );
  button = new QToolButton( icon_forward, tr("Forward"), "", textBrowser, SLOT(forward()), _toolBar );
  connect( textBrowser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
  button->setEnabled( FALSE );
  button = new QToolButton( icon_home, tr("Home"), "", textBrowser, SLOT(home()), _toolBar );

  _toolBar->addSeparator();
  textBrowser->setFocus();
}



void QBHelpBrowser::setBackwardAvailable(bool b){
  _menuBar->setItemEnabled(_backwardId, b);
}



void QBHelpBrowser::setForwardAvailable(bool b){
  _menuBar->setItemEnabled(_forwardId, b);
}



void QBHelpBrowser::slotSourceChanged(const QString& url ){
  if (textBrowser->documentTitle().isNull() )
    setCaption( "Qt Example - Helpviewer - " + url );
  else
    setCaption( "Qt Example - Helpviewer - " + textBrowser->documentTitle() ) ;

}



QBHelpBrowser::~QBHelpBrowser(){
}



void QBHelpBrowser::slotAbout(){
  QMessageBox::about( this, "Help Browser",
                     "<p>Simple Help Browser</p>"
                     "<p>(c) 2006 Martin Preuss</p>");
}



void QBHelpBrowser::slotAboutQt(){
  QMessageBox::aboutQt( this, "QBrowser" );
}



void QBHelpBrowser::slotNewWindow(){
  ( new QBHelpBrowser(textBrowser->source(), "qbrowser") )->show();
}



void QBHelpBrowser::slotPrint(){
#ifndef QT_NO_PRINTER
  QPrinter printer( QPrinter::HighResolution );
  printer.setFullPage(TRUE);
  if ( printer.setup( this ) ) {
    QPainter p( &printer );
    if( !p.isActive() ) // starting printing failed
      return;
    QPaintDeviceMetrics metrics(p.device());
    int dpiy = metrics.logicalDpiY();
    int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
    QRect body( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
    QSimpleRichText richText( textBrowser->text(),
                             QFont(),
                             textBrowser->context(),
                             textBrowser->styleSheet(),
                             textBrowser->mimeSourceFactory(),
                             body.height() );
    richText.setWidth( &p, body.width() );
    QRect view( body );
    int page = 1;
    do {
      richText.draw( &p, body.left(), body.top(), view, colorGroup() );
      view.moveBy( 0, body.height() );
      p.translate( 0 , -body.height() );
      p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
                 view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
      if ( view.top()  >= richText.height() )
        break;
      printer.newPage();
      page++;
    } while (TRUE);
  }
#endif
}



void QBHelpBrowser::slotAddBookmark() {
}



#include "qbhelpbrowser.moc"



