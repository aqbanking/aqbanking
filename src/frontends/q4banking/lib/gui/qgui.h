

#ifndef QGUI_H
#define QGUI_H

class QGui;
class QGuiProgress;
class QGuiSimpleBox;
class QWidget;

#include <q4banking/cppgui.h>

#include <Qt/qstring.h>
#include <string>

class QBanking;


class Q4BANKING_API QGui: public CppGui {

private:
  uint32_t _lastProgressId;
  uint32_t _lastBoxId;

  QWidget *_parentWidget;
  std::list<QWidget*> _pushedParents;

  std::list<QGuiProgress*> _progressPtrList;
  std::list<QGuiSimpleBox*> _simpleBoxWidgets;

  void _addProgress(QGuiProgress *pro);
  void _delProgress(QGuiProgress *pro);
  QGuiProgress *_findProgress(uint32_t id);

  void _checkVisibilities();

public:
  QGui();
  virtual ~QGui();


  QWidget *getParentWidget() const { return _parentWidget;};

  void pushParentWidget(QWidget *w);
  void popParentWidget();

  std::string qstringToUtf8String(const QString &qs);
  QString extractHtml(const char *text);

protected:
  /** @name User Interaction
   *
   */
  /*@{*/
  /**
   * See @ref AB_Gui_MessageBox
   */
  virtual int messageBox(uint32_t flags,
			 const char *title,
			 const char *text,
			 const char *b1,
			 const char *b2,
			 const char *b3,
			 uint32_t guiid);

  /**
   * See @ref AB_Gui_InputBox
   */
  virtual int inputBox(uint32_t flags,
		       const char *title,
		       const char *text,
		       char *buffer,
		       int minLen,
		       int maxLen,
		       uint32_t guiid);

  /**
   * See @ref AB_Gui_ShowBox
   */
  virtual uint32_t showBox(uint32_t flags,
			   const char *title,
			   const char *text,
			   uint32_t guiid);

  /**
   * See @ref AB_Gui_HideBox
   */
  virtual void hideBox(uint32_t id);

  /**
   * See @ref AB_Gui_ProgressStart
   */
  virtual uint32_t progressStart(uint32_t flags,
				 const char *title,
				 const char *text,
				 uint64_t total,
				 uint32_t guiid);

  /**
   * See @ref AB_Gui_ProgressAdvance
   */
  virtual int progressAdvance(uint32_t id,
                              uint64_t progress);

  /**
   * See @ref AB_Gui_ProgressLog
   */
  virtual int progressLog(uint32_t id,
			  GWEN_LOGGER_LEVEL level,
			  const char *text);

  /**
   * See @ref AB_Gui_ProgressEnd
   */
  virtual int progressEnd(uint32_t id);

};



#endif
