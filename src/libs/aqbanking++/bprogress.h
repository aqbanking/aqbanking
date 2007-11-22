/***************************************************************************
 * $RCSfile$
 -------------------
    cvs         : $Id: banking.h 1106 2007-01-09 21:14:59Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef BANKING_BPROGRESS_H
#define BANKING_BPROGRESS_H


#include <aqbanking++/banking.h>
#include <gwenhywfar/types.h>
#include <time.h>

#include <list>
#include <string>


class AB_Progress;

typedef std::list<AB_Progress*> AB_ProgressPtrList;
typedef std::list<AB_Progress*>::iterator AB_ProgressPtrListIterator;
typedef std::list<AB_Progress*>::reverse_iterator AB_ProgressPtrListReverseIterator;


class AB_Progress {

protected:
  uint32_t _id;
  std::string _title;
  uint32_t _flags;
  bool _finished;
  bool _isVisible;

  uint64_t _total;
  uint64_t _current;

  time_t _startTime;
  time_t _lastTime;


  AB_Progress(uint32_t id, const char *title,
	      uint32_t flags, uint64_t _total);

public:
  virtual ~AB_Progress();

  uint32_t getId() const { return _id; };
  uint32_t getFlags() const { return _flags; };

  uint64_t getTotal() const { return _total;};
  uint64_t getCurrent() const { return _current;};
  void setCurrent(uint64_t i) { _current=i;};

  const std::string &getTitle() const { return _title; };

  bool finished() const { return _finished;};
  bool isVisible() const { return _isVisible;};
  void setVisible(bool b) { _isVisible=b;};

  time_t getStartTime() const { return _startTime;};

};




#endif

