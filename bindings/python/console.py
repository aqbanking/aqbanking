# -*- encoding: latin1 -*-
# Copyright (C) 2005 by Andreas Degert
# see LICENSE.txt in the top level distribution directory
#
import aqbanking, getpass, ctypes, sys

class BankingConsole(aqbanking.BankingBase):

    def getPin(self,ab,flags,token,title,text,buffer,minLen,maxLen):
        text = aqbanking.stripText(
            text, markup=False, encoding=sys.stdin.encoding)
        stdin = sys.stdin
        try:
            sys.stdin = file('/dev/tty')
        except:
            pass
        try:
            p = getpass.getpass(text+': ')
        finally:
            sys.stdin = stdin
        ctypes.memmove(buffer, p, len(p)+1)
        return 0

    def messageBox(self, ab, flags, title, text, b1, b2, b3):
        print "message_box", text
        return 0

    def inputBox(*args):
        print "input_box", args
        return 0

    def showBox(*args):
        print "show_box", args
        return 0

    def hideBox(*args):
        print "hide_box", args
        return 0

    def progressStart(self, ab, title, text, total):
        print text.decode("utf8")
        return 0

    def progressAdvance(self, ab, l_id, progress):
        #sys.stdout.write('.')
        return 0

    def progressLog(self, ab, l_id, level, text):
        print text.decode("utf8")
        return 0

    def progressEnd(*args):
        return 0

    def printout(*args):
        print "printout", args
        return 0

    def setPinStatus(*args):
        #print "set_pin_status", args
        return 0

    def getTan(self,ab,token,title,text,buffer,minLen,maxLen):
        return self.getPin(ab,0,token,title,text,buffer,minLen,maxLen)

    def setTanStatus(*args):
        print "set_tan_status", args
        return 0
