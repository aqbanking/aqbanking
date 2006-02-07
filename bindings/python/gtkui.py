# -*- encoding: latin1 -*-
# Copyright (C) 2005 by Andreas Degert
# see LICENSE.txt in the top level distribution directory
#
import aqbanking, gtk, ctypes

def gtk_main():
    while gtk.events_pending():
        gtk.main_iteration()

class BankingGtk(aqbanking.BankingBase):

    def __init__(self, name, configdir=None, progwin=None):
        aqbanking.BankingBase.__init__(self, name, configdir)
        self.cancel_flag = False
        self.progwin = progwin
        self.textbuffer = self.logwin = None

    def getPin(self,ab,flags,token,title,text,buffer,minLen,maxLen):
        d = gtk.Dialog(title=title, parent = self.progwin,
                       flags=gtk.DIALOG_MODAL,
                       buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT,
                                gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
        d.set_default_response(gtk.RESPONSE_ACCEPT)
        d.set_response_sensitive(gtk.RESPONSE_ACCEPT, False)
        w = gtk.Label(aqbanking.stripText(text))
        w.set_line_wrap(True)
        w.set_use_markup(True)
        h = gtk.HBox()
        h.pack_start(w, False, False)
        d.vbox.add(h)
        e = gtk.Entry()
        e.set_visibility(False)
        e.set_max_length(maxLen)
        e.set_activates_default(True)
        e.connect("changed", lambda w: d.set_response_sensitive(
            gtk.RESPONSE_ACCEPT, len(w.get_text()) >= minLen))
        d.vbox.add(e)
        d.show_all()
        ret = d.run()
        p = e.get_text()
        d.destroy()
        gtk_main()
        if ret != gtk.RESPONSE_ACCEPT:
            return 1
        ctypes.memmove(buffer, p, len(p)+1)
        return 0

    msgtypes = { aqbanking.MSG_FLAGS_TYPE_INFO: gtk.MESSAGE_INFO,
                 aqbanking.MSG_FLAGS_TYPE_WARN: gtk.MESSAGE_WARNING,
                 aqbanking.MSG_FLAGS_TYPE_ERROR: gtk.MESSAGE_ERROR,
                 }
    def messageBox(self, ab, flags, title, text, *buttons):
        gtktype = self.msgtypes[aqbanking.msg_flags_type(flags)]
        d = gtk.MessageDialog(
            parent=self.progwin, flags=gtk.DIALOG_MODAL, type=gtktype)
        d.set_markup(aqbanking.stripText(text))
        for i, b in enumerate(buttons):
            if b is None:
                continue
            w = gtk.Button(b)
            w.show()
            d.add_action_widget(w,i)
        ret = d.run()
        d.destroy()
        gtk_main()
        return ret

    def inputBox(*args):
        print "input_box", args
        return 0

    def showBox(*args):
        print "show_box", args
        return 0

    def hideBox(*args):
        print "hide_box", args
        return 0

    def ProgressDestroyed(self, w):
        self.progwin = self.logwin = self.textbuffer = None
        gtk.main_quit()
        gtk_main()

    def fini(self):
        # hack falls progressLog ohne progressStart/End aufgerufen wurde
        aqbanking.BankingBase.fini(self)
        if self.progwin is not None:
            self.progressEnd()
        
    def createProgress(self):
        w = gtk.Dialog()
        w.connect("destroy", self.ProgressDestroyed)
        w.set_default_size(500,400)
        sw = gtk.ScrolledWindow()
        sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.textview = gtk.TextView()
        self.textview.set_cursor_visible(False)
        self.textview.set_editable(False)
        self.textbuffer = self.textview.get_buffer()
        self.tag = self.textbuffer.create_tag(background="red")
        sw.add(self.textview)
        w.vbox.add(sw)
        e = gtk.ProgressBar()
        w.vbox.pack_start(e, False, False)
        w.vbox.show_all()
        b = gtk.Button('Abbrechen')
        b.show()
        w.add_action_widget(b,gtk.RESPONSE_CANCEL)
        b = gtk.Button('Schliessen')
        b.show()
        w.add_action_widget(b,gtk.RESPONSE_CLOSE)
        w.connect('response', self.progress_response)
        self.progwin = w
        self.logwin = e
        self.total = 100.0

    def progress_response(self, w, r):
        if r == gtk.RESPONSE_CANCEL:
            self.cancel_flag = True
        elif r == gtk.RESPONSE_CLOSE:
            self.progwin.destroy()

    def progressStart(self, ab, title, text, total):
        self.cancel_flag = False
        self.createProgress()
        self.progwin.set_title(title)
        insertmark = self.textbuffer.get_insert()
        insertiter = self.textbuffer.get_iter_at_mark(insertmark)
        self.textbuffer.insert(insertiter, text+'\n')
        self.progwin.set_response_sensitive(gtk.RESPONSE_CLOSE, False)
        self.progwin.show()
        gtk_main()
        self.total = float(total)
        return 0

    def progressAdvance(self, ab, l_id, progress):
        if progress >= 0:
            self.logwin.set_fraction(progress/self.total)
        gtk_main()
        if self.cancel_flag:
            self.cancel_flag = False
            return 1
        return 0

    def progressLog(self, ab, l_id, level, text):
        if self.textbuffer is None:
            #return 0
            # hack weil progressLog ohne progressStart aufgerufen wird
            if level > aqbanking.LogLevel.notice:
                return 0
            self.createProgress()
            self.progwin.show()
            gtk_main()
            #print "progressLog!", l_id, level, text
            #return 0
        level = aqbanking.LogLevel(level)
        insertmark = self.textbuffer.get_insert()
        insertiter = self.textbuffer.get_iter_at_mark(insertmark)
        if level <= aqbanking.LogLevel.error:
            self.textbuffer.insert_with_tags(insertiter, text+'\n', self.tag)
        elif level <= aqbanking.LogLevel.notice:
            self.textbuffer.insert(insertiter, text+'\n')
        else:
            pass
        self.textview.scroll_mark_onscreen(insertmark)
        gtk_main()
        return 0

    def progressEnd(self, *args):
        self.progwin.set_response_sensitive(gtk.RESPONSE_CLOSE, True)
        self.progwin.set_response_sensitive(gtk.RESPONSE_CANCEL, False)
        gtk.main()
        return 0

    def printout(*args):
        print "printout", args
        return 0

    def setPinStatus(*args):
        #print "set_pin_status", args
        return 0

    def getTan(*args):
        flag = 0
        return self.getPin(ab,flags,token,title,text,buffer,minLen,maxLen)

    def setTanStatus(*args):
        print "set_tan_status", args
        return 0
