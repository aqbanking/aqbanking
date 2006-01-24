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
                             const QStringList& paths,
                             QWidget* parent,
                             const char *name)
:QBHelpBrowserUi(parent, name, WDestructiveClose){
  textBrowser->mimeSourceFactory()->setFilePath(paths);
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

  _backwardButton=new QToolButton( icon_back, tr("Backward"), "",
                                  textBrowser, SLOT(backward()),
                                  _toolBar );
  _backwardButton->setEnabled(FALSE);

  _forwardButton=new QToolButton(icon_forward, tr("Forward"), "",
                                 textBrowser, SLOT(forward()),
                                 _toolBar );

  _forwardButton->setEnabled( FALSE );

  QToolButton *button;

  button=new QToolButton( icon_home, tr("Home"), "",
                         textBrowser, SLOT(home()),
                         _toolBar );

  connect(textBrowser, SIGNAL(backwardAvailable(bool)),
          this, SLOT(slotBackwardAvailable(bool)));
  connect(textBrowser, SIGNAL(forwardAvailable(bool) ),
          this, SLOT(slotForwardAvailable(bool)));

  _toolBar->addSeparator();
  textBrowser->setFocus();
}



QBHelpBrowser::~QBHelpBrowser(){
}



void QBHelpBrowser::setFilePaths(const QStringList& pathList) {
  textBrowser->mimeSourceFactory()->setFilePath(pathList);
}



void QBHelpBrowser::slotBackwardAvailable(bool b){
  _menuBar->setItemEnabled(_backwardId, b);
  _backwardButton->setEnabled(b);
}



void QBHelpBrowser::slotForwardAvailable(bool b){
  _menuBar->setItemEnabled(_forwardId, b);
  _forwardButton->setEnabled(b);
}



void QBHelpBrowser::slotSourceChanged(const QString& url ){
  QString fName;
  int i;

  i=url.find('#');
  if (i)
    fName=url.left(i);
  else
    fName=url;

  if (textBrowser->mimeSourceFactory()->data(fName)==0) {
    setCaption( "QBanking Helpviewer - Not found" );
    textBrowser->setText(tr("<qt>"
                            "<font color=\"red\"><h2>Not found</h2></font>"
                            "Sorry - the help chapter <i>%1</i>"
                            " is not available."
                            "</qt>").arg(url));
  }
  else {
    if (textBrowser->documentTitle().isNull() )
      setCaption( "QBanking Helpviewer - " + url );
    else
      setCaption( "QBanking Helpviewer - " + textBrowser->documentTitle() ) ;
  }
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
  ( new QBHelpBrowser(textBrowser->source(), QStringList(QString("qbrowser")), parentWidget()) )->show();
}



void QBHelpBrowser::slotPrint(){
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
}



void QBHelpBrowser::slotAddBookmark() {
}



#include "qbhelpbrowser.moc"



