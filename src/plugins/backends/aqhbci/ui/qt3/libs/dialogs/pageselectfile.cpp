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


#include "wizard.h"
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>


#include <gwenhywfar/debug.h>





bool Wizard::initSelectFilePage() {
  setNextEnabled(selectFilePage, false);
  QObject::connect((QObject*)(fileTypeCombo),
                   SIGNAL(activated(int)),
                   this,
                   SLOT(slotFiletypeChanged(int)));
  return true;
}



void Wizard::slotFiletypeChanged(int i){
  if (i==0) {
    setNextEnabled(selectFilePage, false);
    fileTypeBrowser->setText(tr("Please select a file type"));
  }
  else {
    if (_plugins) {
      GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

      it=GWEN_PluginDescription_List2_First(_plugins);
      if (it) {
        GWEN_PLUGIN_DESCRIPTION *pd;

        pd=GWEN_PluginDescription_List2Iterator_Data(it);
        while(pd && --i) pd=GWEN_PluginDescription_List2Iterator_Next(it);
        GWEN_PluginDescription_List2Iterator_free(it);
        if (pd) {
          const char *p;
          GWEN_BUFFER *tbuf;

          setNextEnabled(selectFilePage, true);
          p=GWEN_PluginDescription_GetName(pd);
          if (p)
            _mediumTypeName=p;
          else
            _mediumTypeName.erase();
          tbuf=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_Buffer_AppendString(tbuf, "<qt>");
          if (GWEN_PluginDescription_GetLongDescrByFormat(pd, "html", tbuf)){
            p=GWEN_PluginDescription_GetLongDescr(pd);
            if (p) {
              fileTypeBrowser->setText(p);
            }
            else
              fileTypeBrowser->setText(tr("no description available"));
          }
          else {
            GWEN_Buffer_AppendString(tbuf, "</qt>");
            fileTypeBrowser->setText(GWEN_Buffer_GetStart(tbuf));
          }
          GWEN_Buffer_free(tbuf);
        }
        else
          fileTypeBrowser->setText(tr("no description available"));
      } /* if it */
    }
  }
}



bool Wizard::doSelectFilePage(QWidget *p){
  DBG_INFO(0, "Creating medium");
  _medium=AH_HBCI_FindMedium(_hbci,
                             _mediumTypeName.c_str(),
                             _mediumName.c_str());
  if (_medium) {
    /* medium exists while it shouldn't */
    DBG_ERROR(0, "Medium already exists");
    QMessageBox::critical(0,
                          tr("Medium Error"),
                          tr("The medium already exists.\n"
                             "Please select another name."),
                          tr("Dismiss"),0,0,0);
    _medium=0;
    return false;
  }

  _mediumCreated=false;

  /* create the medium */
  _medium=AH_HBCI_MediumFactory(_hbci,
                                _mediumTypeName.c_str(),
                                _mediumSubTypeName.c_str(),
                                _mediumName.c_str());
  if (!_medium) {
    QMessageBox::critical(0,
                          tr("Medium Error"),
                          tr("Could not create the medium.\n"
                             "Please check the console logs."),
                          tr("Dismiss"),0,0,0);
    return false;
  }
  if (AH_Medium_Create(_medium)) {
    QMessageBox::critical(0,
                          tr("Medium Error"),
                          tr("Could not create the medium.\n"
                             "Please check the console logs."),
                          tr("Dismiss"),0,0,0);
    AH_Medium_free(_medium);
    if (_createFile)
      QFile::remove(fileNameEdit->text());
    _medium=0;
    return false;
  }

  AH_HBCI_AddMedium(_hbci, _medium);
  DBG_INFO(0, "New medium created");
  _mediumCreated=true;

  return doSelectCheckFileCardPage(p);
}



bool Wizard::undoSelectFilePage(QWidget *p){
  return undoSelectCheckFileCardPage(p);
}




